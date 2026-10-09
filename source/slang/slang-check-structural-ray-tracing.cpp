#include "slang-check-impl.h"
#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-lookup.h"
#include "slang-mangle.h"
#include "slang-module.h"
#include "slang-session.h"
#include "slang-syntax.h"

namespace Slang
{

// Accepts ordinary-module magic annotations only for executable stage interfaces.
// For example, `__magic_type(MissShaderType) interface IMissShader` has the same declaration
// reference representation as an ordinary interface; arbitrary core magic classes may require
// different operands and cannot safely be applied to an ordinary declaration.
bool isRayTracingStageInterfaceModifier(MagicTypeModifier* modifier, Decl* decl)
{
    return as<InterfaceDecl>(decl) && modifier && modifier->magicNodeType &&
           modifier->magicNodeType.getInfo()->createFunc &&
           modifier->magicNodeType.isSubClassOf<RayTracingStageInterfaceType>();
}

// This file owns the structural ray-tracing rules and their declaration identities.
// Ordinary interfaces, associated-type constraints, overload resolution, and capability inference
// remain in the normal checker. Their call sites delegate here only for the additional rules:
// stage receivers and inputs, stage selection, schema entry uniqueness, API mixing, and the
// boundary between logical shaders and native entry points. Each rule below states its example
// and the representation or API contract it preserves.
//
// Schemas, lists, and hit groups are ordinary values. Empty payloads are ordinary payloads.
// Their ordinary generic uses are allowed; stage/input generic uses retain the restriction below.

bool isCoreLegacyRayTracingPipelineMethod(FunctionDeclBase* functionDecl)
{
    if (!functionDecl || !functionDecl->getName())
        return false;

    auto name = functionDecl->getName()->text.getUnownedSlice();
    if (name != "TraceRay" && name != "TraceMotionRay" && name != "CallShader")
        return false;

    for (auto parent = functionDecl->parentDecl; parent; parent = parent->parentDecl)
    {
        if (as<AggTypeDecl>(parent))
            return false;
        if (auto moduleDecl = as<ModuleDecl>(parent))
            return moduleDecl->hasModifier<FromCoreModuleModifier>();
    }
    return false;
}

static String _getStructuralRayTracingSourceDeclName(Decl* decl)
{
    if (!decl)
        return String();

    auto leafName = decl->getName();
    if (!leafName || leafName->text.getLength() == 0)
        return String();

    auto parentDecl = decl->parentDecl;
    if (auto genericParentDecl = as<GenericDecl>(parentDecl))
        parentDecl = genericParentDecl->parentDecl;
    if (auto fileParentDecl = as<FileDecl>(parentDecl))
        parentDecl = fileParentDecl->parentDecl;
    if (auto moduleParentDecl = as<ModuleDecl>(parentDecl))
        parentDecl = moduleParentDecl->parentDecl;

    auto parentName = _getStructuralRayTracingSourceDeclName(parentDecl);
    if (parentName.getLength() == 0)
        return leafName->text;

    StringBuilder result;
    result << parentName << "." << leafName->text;
    return result.produceString();
}

static bool _hasStructuralRayTracingGenericSubstitution(DeclRefBase* declRef)
{
    // Consider `GenericMiss<uint>` and an ordinary `Miss`. The first decl-ref contains a
    // `GenericAppDeclRef` carrying `uint`, while the second has no generic substitution at all.
    // Check that semantic representation directly instead of trying to recognize generic syntax
    // in a printed or mangled name.
    bool result = false;
    SubstitutionSet(declRef).forEachGenericSubstitution([&](GenericDecl*, Val::OperandView<Val>)
                                                        { result = true; });
    return result;
}

String getStructuralRayTracingSourceTypeName(ASTBuilder* astBuilder, Type* type)
{
    auto declRefType = as<DeclRefType>(type ? type->resolve() : nullptr);
    if (!declRefType)
        return String();

    auto sourceName = _getStructuralRayTracingSourceDeclName(declRefType->getDeclRef().getDecl());
    if (sourceName.getLength() == 0 ||
        !_hasStructuralRayTracingGenericSubstitution(declRefType->getDeclRef().declRefBase))
    {
        return sourceName;
    }

    // `GenericMiss<uint>` and `GenericMiss<float>` share one declaration path, but their
    // canonical semantic types have distinct mangled identities. Hash that existing identity only
    // to keep the public target symbol compact; the compiler never parses a mangled spelling to
    // rediscover either the declaration or its substitutions.
    auto canonicalType = type->getCanonicalType();
    auto mangledTypeName = getMangledTypeName(astBuilder, canonicalType);
    StringBuilder result;
    result << sourceName << getHashedName(mangledTypeName.getUnownedSlice());
    return result.produceString();
}

static String _encodeStructuralRayTracingSymbolName(UnownedStringSlice logicalName)
{
    static const UnownedStringSlice kEncodedPrefix = toSlice("__slang_structural_rt_");
    StringBuilder result;
    result << kEncodedPrefix;
    for (auto c : logicalName)
    {
        auto byte = uint8_t(c);
        result.appendChar("0123456789abcdef"[byte >> 4]);
        result.appendChar("0123456789abcdef"[byte & 0xf]);
    }
    return result.produceString();
}

String getStructuralRayTracingEntryPointName(UnownedStringSlice sourceTypeName)
{
    // Consider `Miss` and `Stages.Miss`. Keeping `Miss` unchanged preserves the public names used
    // by existing structural programs. `Stages.Miss` cannot be emitted as a CUDA or C-like symbol,
    // while C-like targets reserve the entry-point name `main`, so encode every UTF-8 byte of those
    // names after a compiler-reserved prefix. We also encode source names that start with the
    // prefix; consequently a user-written identifier cannot collide with an encoded qualified
    // name.
    static const UnownedStringSlice kEncodedPrefix = toSlice("__slang_structural_rt_");
    bool isSimpleIdentifier =
        sourceTypeName.getLength() != 0 && !sourceTypeName.startsWith(kEncodedPrefix) &&
        sourceTypeName != toSlice("main") &&
        ((sourceTypeName[0] >= 'A' && sourceTypeName[0] <= 'Z') ||
         (sourceTypeName[0] >= 'a' && sourceTypeName[0] <= 'z') || sourceTypeName[0] == '_');
    for (Index i = 1; isSimpleIdentifier && i < sourceTypeName.getLength(); ++i)
    {
        auto c = sourceTypeName[i];
        isSimpleIdentifier =
            (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_';
    }
    if (isSimpleIdentifier)
        return String(sourceTypeName);

    return _encodeStructuralRayTracingSymbolName(sourceTypeName);
}

// The annotation already identifies the AST type that DeclRefType::create constructs. Reading
// its class here avoids constructing a type merely to classify a declaration during checking.
static ASTNodeType _getMagicTypeClass(Decl* declaration)
{
    auto modifier = declaration ? declaration->findModifier<MagicTypeModifier>() : nullptr;
    return modifier && modifier->magicNodeType ? modifier->magicNodeType.getTag()
                                               : ASTNodeType::CountOf;
}

// Reads a semantic role from an ordinary declaration's checked attribute. For example,
// MissInput<C> remains an ordinary generic struct while its annotation identifies the stage view.
// The attribute is serialized with the declaration, so no module-import registration is needed.
static KnownBuiltinDeclName _getRayTracingBuiltin(Decl* declaration)
{
    auto attribute = declaration ? declaration->findModifier<KnownBuiltinAttribute>() : nullptr;
    auto value = attribute ? as<ConstantIntVal>(attribute->name) : nullptr;
    return value ? KnownBuiltinDeclName(value->getValue()) : KnownBuiltinDeclName::COUNT;
}

StructuralRayTracingStageKind getStructuralRayTracingStageKind(InterfaceDecl* interfaceDecl)
{
    switch (_getMagicTypeClass(interfaceDecl))
    {
    case ASTNodeType::ClosestHitShaderType:
        return StructuralRayTracingStageKind::ClosestHit;
    case ASTNodeType::AnyHitShaderType:
        return StructuralRayTracingStageKind::AnyHit;
    case ASTNodeType::IntersectionShaderType:
        return StructuralRayTracingStageKind::Intersection;
    case ASTNodeType::MissShaderType:
        return StructuralRayTracingStageKind::Miss;
    case ASTNodeType::CallableShaderType:
        return StructuralRayTracingStageKind::Callable;
    default:
        return StructuralRayTracingStageKind::Count;
    }
}

bool isExecutableStructuralRayTracingStageInterface(InterfaceDecl* interfaceDecl)
{
    auto modifier = interfaceDecl ? interfaceDecl->findModifier<MagicTypeModifier>() : nullptr;
    return modifier && modifier->magicNodeType &&
           modifier->magicNodeType.isSubClassOf<RayTracingStageInterfaceType>();
}

StructuralRayTracingStageKind getStructuralRayTracingStageInputKind(AggTypeDecl* typeDecl)
{
    switch (_getRayTracingBuiltin(typeDecl))
    {
    case KnownBuiltinDeclName::RayTracingClosestHitInput:
        return StructuralRayTracingStageKind::ClosestHit;
    case KnownBuiltinDeclName::RayTracingAnyHitInput:
        return StructuralRayTracingStageKind::AnyHit;
    case KnownBuiltinDeclName::RayTracingIntersectionInput:
        return StructuralRayTracingStageKind::Intersection;
    case KnownBuiltinDeclName::RayTracingMissInput:
        return StructuralRayTracingStageKind::Miss;
    case KnownBuiltinDeclName::RayTracingCallableInput:
        return StructuralRayTracingStageKind::Callable;
    default:
        return StructuralRayTracingStageKind::Count;
    }
}

StructuralRayTracingAssociatedTypeKind getStructuralRayTracingAssociatedTypeKind(
    AssocTypeDecl* requirement)
{
    switch (_getRayTracingBuiltin(requirement))
    {
    case KnownBuiltinDeclName::RayTracingStageContext:
        return StructuralRayTracingAssociatedTypeKind::StageContext;
    case KnownBuiltinDeclName::RayTracingStageRecord:
        return StructuralRayTracingAssociatedTypeKind::StageRecord;
    case KnownBuiltinDeclName::RayTracingPayloadContextPayload:
        return StructuralRayTracingAssociatedTypeKind::PayloadContextPayload;
    case KnownBuiltinDeclName::RayTracingHitPrimitive:
        return StructuralRayTracingAssociatedTypeKind::HitPrimitive;
    case KnownBuiltinDeclName::RayTracingPrimitiveAttributes:
        return StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes;
    case KnownBuiltinDeclName::RayTracingCallableData:
        return StructuralRayTracingAssociatedTypeKind::CallableData;
    case KnownBuiltinDeclName::RayTracingProgramHitGroups:
        return StructuralRayTracingAssociatedTypeKind::ProgramHitGroups;
    case KnownBuiltinDeclName::RayTracingProgramMissShaders:
        return StructuralRayTracingAssociatedTypeKind::ProgramMissShaders;
    case KnownBuiltinDeclName::RayTracingProgramCallableShaders:
        return StructuralRayTracingAssociatedTypeKind::ProgramCallableShaders;
    default:
        return StructuralRayTracingAssociatedTypeKind::Count;
    }
}

// Finds a requirement in the interface supplied by the caller's witness and projects that same
// witness to its owner. Consider `HitContext : IHitContext`, with `IHitContext : IPayloadContext`
// and `IPayloadContext : IStageContext`. Resolving `Record` needs the inherited IStageContext
// requirement. The checked interface facets identify that declaration; projecting the original
// witness preserves this path even when HitContext has another IStageContext conformance.
static AssocTypeDecl* _findStructuralAssociatedRequirement(
    SubtypeWitness*& witness,
    StructuralRayTracingAssociatedTypeKind kind)
{
    witness = witness ? as<SubtypeWitness>(witness->resolve()) : nullptr;
    if (!witness)
        return nullptr;
    auto interfaceType = as<DeclRefType>(witness->getSup()->resolve());
    auto interfaceDecl =
        interfaceType ? interfaceType->getDeclRef().as<InterfaceDecl>() : DeclRef<InterfaceDecl>();
    if (!interfaceDecl)
        return nullptr;
    auto module = getModule(interfaceDecl.getDecl());
    SLANG_RELEASE_ASSERT(module);
    auto sharedSemantics = module->getLinkage()->getSemanticsForReflection();
    for (auto facet : sharedSemantics->getInheritanceInfo(interfaceType).facets)
    {
        auto owner = facet->origin.declRef.as<InterfaceDecl>();
        if (!owner)
            continue;
        for (auto requirement : owner.getDecl()->getDirectMemberDeclsOfType<AssocTypeDecl>())
        {
            if (getStructuralRayTracingAssociatedTypeKind(requirement) != kind)
                continue;
            witness = sharedSemantics->tryProjectInterfaceSubtypeWitness(witness, facet->getType());
            return witness ? requirement : nullptr;
        }
    }
    return nullptr;
}

Type* resolveStructuralRayTracingAssociatedType(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind)
{
    auto requirement = _findStructuralAssociatedRequirement(witness, kind);
    if (!requirement)
        return nullptr;
    auto requirementWitness = tryLookUpRequirementWitness(astBuilder, witness, requirement);
    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::val)
        return as<Type>(requirementWitness.getVal()->resolve());
    if (requirementWitness.getFlavor() == RequirementWitness::Flavor::declRef)
    {
        auto type = DeclRefType::create(astBuilder, requirementWitness.getDeclRef());
        return type ? as<Type>(type->resolve()) : nullptr;
    }
    return nullptr;
}

// Selects the associated-type constraint that supplies the next ABI requirement.
// For example, a miss contract can constrain Context by both IExtra and IPayloadContext.
// Only the latter supplies the annotated Payload requirement. Selecting that checked constraint
// by its requirement role keeps additional bounds and refined context interfaces ordinary Slang.
SubtypeWitness* resolveStructuralRayTracingAssociatedTypeConstraint(
    ASTBuilder* astBuilder,
    SubtypeWitness* witness,
    StructuralRayTracingAssociatedTypeKind kind,
    StructuralRayTracingAssociatedTypeKind requiredMember)
{
    SLANG_RELEASE_ASSERT(requiredMember != StructuralRayTracingAssociatedTypeKind::Count);
    auto associatedType = _findStructuralAssociatedRequirement(witness, kind);
    if (!associatedType)
        return nullptr;
    auto owner = as<InterfaceDecl>(associatedType->parentDecl);
    SLANG_RELEASE_ASSERT(owner);
    for (auto constraint : owner->getDirectMemberDeclsOfType<GenericTypeConstraintDecl>())
    {
        if (constraint->isEqualityConstraint)
            continue;
        auto subType = isDeclRefTypeOf<AssocTypeDecl>(constraint->sub.type);
        if (!subType || subType.getDecl() != associatedType)
            continue;
        auto requirementWitness = tryLookUpRequirementWitness(astBuilder, witness, constraint);
        if (requirementWitness.getFlavor() != RequirementWitness::Flavor::val)
            continue;
        auto constraintWitness = as<SubtypeWitness>(requirementWitness.getVal()->resolve());
        if (!constraintWitness)
            continue;

        // A refined context may inherit the requirement rather than declare it directly.
        // Reuse normal requirement projection to recognize that bound, but return its original
        // witness so resolving Record later starts from the same context conformance.
        auto projectedWitness = constraintWitness;
        if (_findStructuralAssociatedRequirement(projectedWitness, requiredMember))
            return constraintWitness;
    }
    return nullptr;
}

StructuralRayTracingHitAttributesKind getStructuralRayTracingHitAttributesKind(Type* primitiveType)
{
    primitiveType = primitiveType ? as<Type>(primitiveType->resolve()) : nullptr;
    auto primitiveDecl = isDeclRefTypeOf<AggTypeDecl>(primitiveType);
    if (!primitiveDecl)
        return StructuralRayTracingHitAttributesKind::None;
    switch (_getRayTracingBuiltin(primitiveDecl.getDecl()))
    {
    case KnownBuiltinDeclName::RayTracingTrianglePrimitive:
        return StructuralRayTracingHitAttributesKind::Triangle;
    case KnownBuiltinDeclName::RayTracingCurvePrimitive:
        return StructuralRayTracingHitAttributesKind::Curve;
    default:
        return StructuralRayTracingHitAttributesKind::Custom;
    }
}

// Distinguishes source dispatch overloads for later adapter lowering. For example,
// trace(desc, scene, program, payload) has an annotated parameter even when payload is empty;
// the overload without that parameter selects the implicit-payload operation instead.
StructuralRayTracingTraceMethodKind getStructuralRayTracingTraceMethodKind(
    FunctionDeclBase* functionDecl)
{
    if (!functionDecl || !functionDecl->hasModifier<RayTracingTraceAttribute>())
        return StructuralRayTracingTraceMethodKind::None;
    for (auto parameter : functionDecl->getParameters())
    {
        if (parameter->hasModifier<RayTracingPayloadAttribute>())
            return StructuralRayTracingTraceMethodKind::ExplicitPayload;
    }
    return StructuralRayTracingTraceMethodKind::ImplicitEmptyPayload;
}

bool isStructuralRayTracingTraceMethod(FunctionDeclBase* functionDecl)
{
    return getStructuralRayTracingTraceMethodKind(functionDecl) !=
           StructuralRayTracingTraceMethodKind::None;
}

bool isStructuralRayTracingCallShaderMethod(FunctionDeclBase* functionDecl)
{
    return functionDecl && functionDecl->hasModifier<RayTracingCallShaderAttribute>();
}

FunctionDeclBase* getStructuralRayTracingStageInvokeRequirement(InterfaceDecl* interfaceDecl)
{
    if (!isExecutableStructuralRayTracingStageInterface(interfaceDecl))
        return nullptr;
    for (auto member : interfaceDecl->getDirectMemberDeclsOfType<FunctionDeclBase>())
    {
        if (member->getName() && member->getName()->text == "invoke")
            return member;
    }
    return nullptr;
}

StructuralRayTracingEntryPack getStructuralRayTracingEntryPack(
    ASTBuilder* astBuilder,
    Type* entryListType)
{
    StructuralRayTracingEntryPack result;
    if (auto declRefType = as<DeclRefType>(entryListType))
    {
        if (auto genericApp = SubstitutionSet(declRefType->getDeclRef()).findGenericAppDeclRef())
        {
            // A concrete entry-list specialization has a type pack and a matching conformance-
            // witness pack, both empty for an empty list. Select them by semantic role instead of
            // relying on their positions among the generic arguments. Both IR lowering and
            // reflection consume this exact checked representation.
            for (auto argument : genericApp->getArgs())
            {
                auto resolvedArgument = argument->resolve();
                if (auto typePack = as<ConcreteTypePack>(resolvedArgument))
                    result.types = typePack;
                else if (auto witnessPack = as<TypePackSubtypeWitness>(resolvedArgument))
                    result.witnesses = witnessPack;
            }
        }
    }
    // Dependent list types have no concrete entries until specialization.
    if (!result.types)
        result.types = astBuilder->getTypePack(ArrayView<Type*>());
    SLANG_RELEASE_ASSERT(
        !result.witnesses || result.witnesses->getCount() == result.types->getTypeCount());
    return result;
}

static Stage _getNativeStage(StructuralRayTracingStageKind kind);
static StructuralRayTracingStageKind _getStructuralStage(Stage stage);
static StructuralRayTracingStageKind _getDirectStageInputKind(Type* type);

// Rejects storage when a structural stage conformance finishes checking.
//
// Consider this example:
//
//     struct StatefulClosestHit : rt::IClosestHitShader
//     {
//         uint state;
//         ...
//     }
//
// A standalone `-entry StatefulClosestHit` request finds the struct by name, but a ray-generation
// entry point reaches the same stage only through an `ITraceProgramSchema` witness. Both paths
// later synthesize a receiver without a source value. Checking the concrete conformance keeps that
// compiler-created receiver valid for schema entries and standalone stages.
// The semantic checker suppresses repeated diagnostics using its ordinary diagnoseOnce helper:
// several conformances may reach the same invalid stage or stateful base struct.
static void _checkStructuralRayTracingStageStorage(
    SemanticsVisitor* visitor,
    Type* stageType,
    SourceLoc conformanceLoc)
{
    auto astBuilder = visitor->getASTBuilder();
    SLANG_RELEASE_ASSERT(stageType);
    auto witnessedStageType = stageType;
    auto resolvedStageType = as<Type>(stageType->resolve());
    SLANG_RELEASE_ASSERT(resolvedStageType);

    const bool hasNominalWitnessType = as<DeclRefType>(witnessedStageType) != nullptr;
    auto stageDeclRef = isDeclRefTypeOf<AggTypeDecl>(resolvedStageType);
    const bool isCompilerBuiltinType =
        stageDeclRef && (stageDeclRef.getDecl()->findModifier<BuiltinTypeModifier>() ||
                         stageDeclRef.getDecl()->findModifier<MagicTypeModifier>());

    // A user-defined interface may refine a stage contract with additional requirements. It is
    // not itself an executable implementation and therefore has no receiver to validate here.
    if (stageDeclRef.as<InterfaceDecl>())
        return;

    // Structural dispatch creates a stage value without a source object. Only a nominal struct has
    // the representation that contract requires. A scalar such as `float` can resolve through its
    // compiler-provided builtin struct declaration, but that declaration does not make the scalar
    // a source struct. Extension conformances can also witness generic parameters that have no
    // aggregate declaration. Treating either representation as an ordinary source struct would
    // bypass this contract.
    if (!hasNominalWitnessType || !stageDeclRef.as<StructDecl>() || isCompilerBuiltinType)
    {
        const bool hasSourceAggregateDeclaration =
            hasNominalWitnessType && stageDeclRef && !isCompilerBuiltinType;
        visitor->diagnoseOnce(Diagnostics::StructuralRayTracingStageImplementationMustBeStruct{
            .stageType = witnessedStageType,
            .location =
                hasSourceAggregateDeclaration ? stageDeclRef.getDecl()->loc : conformanceLoc});
        return;
    }

    auto structType = stageDeclRef.as<StructDecl>();
    // A base struct contributes storage to the compiler-created stage value even though its
    // fields are not direct members of the concrete implementation. Follow the checked base
    // declaration references so generic base specializations use the same semantic inheritance
    // path as ordinary struct layout.
    for (auto currentType = structType; currentType;)
    {
        for (auto field : currentType.getDecl()->getFields())
        {
            if (!isEffectivelyStatic(field))
                visitor->diagnoseOnce(
                    Diagnostics::StructuralRayTracingStageInstanceField{.field = field});
        }
        currentType = findBaseStructDeclRef(astBuilder, currentType);
    }
}

// Rejects a module that uses both pipeline APIs under the current interoperability contract.
// For example, an rt::IMissShader implementation and a separate [shader("miss")] function in
// the same module conflict. RayQuery/HitObject operations do not select that pipeline API.
// This is an explicit compatibility policy, not a consequence of ordinary interface conformance.
static void _checkRayTracingAPIUse(
    Module* module,
    RayTracingAPIFamily family,
    Decl* decl,
    DiagnosticSink* sink)
{
    if (!module || !decl)
        return;
    auto moduleDecl = module->getModuleDecl();
    auto& currentDecl = family == RayTracingAPIFamily::Structural
                            ? moduleDecl->structuralRayTracingUse
                            : moduleDecl->legacyRayTracingUse;
    // Only the first use of the second family diagnoses the conflict. Keeping these checked
    // declarations on the module also covers a later, explicitly selected native entry point.
    if (currentDecl)
        return;
    currentDecl = decl;
    auto otherDecl = family == RayTracingAPIFamily::Structural
                         ? moduleDecl->legacyRayTracingUse
                         : moduleDecl->structuralRayTracingUse;
    if (!otherDecl)
        return;

    auto currentAPI = family == RayTracingAPIFamily::Structural ? "structural" : "legacy";
    auto otherAPI = family == RayTracingAPIFamily::Structural ? "legacy" : "structural";
    sink->diagnose(Diagnostics::MixedRayTracingApis{
        .currentAPI = currentAPI,
        .otherAPI = otherAPI,
        .currentDecl = decl,
        .otherDecl = otherDecl});
}

// Accounts for pipeline calls when checking the module's API family.
// For example, helper() { TraceRay(...); } selects the legacy family even if helper is not an
// entry point. A structural trace wrapper may use TraceRay internally without selecting both.
void checkRayTracingAPICall(
    FunctionDeclBase* caller,
    FunctionDeclBase* callee,
    DiagnosticSink* sink)
{
    if (!caller || !callee)
        return;

    auto callerModule = getModule(caller);
    if (!callerModule || isStructuralRayTracingTraceMethod(caller) ||
        isStructuralRayTracingCallShaderMethod(caller))
        return;

    if (isStructuralRayTracingTraceMethod(callee) || isStructuralRayTracingCallShaderMethod(callee))
    {
        _checkRayTracingAPIUse(callerModule, RayTracingAPIFamily::Structural, caller, sink);
    }
    else if (isCoreLegacyRayTracingPipelineMethod(callee))
    {
        _checkRayTracingAPIUse(callerModule, RayTracingAPIFamily::Legacy, caller, sink);
    }
}

// Validates the receiver of each completed executable-stage conformance.
// For example, `struct Miss : rt::IMissShader` selects the structural API and must have no
// instance fields. `struct Schema : rt::ITraceProgramSchema` is an ordinary type and has no such
// receiver restriction. Ordinary conformance checking has already validated required members.
void SemanticsVisitor::checkStructuralRayTracingStageConformance(
    DeclRef<InterfaceDecl> superInterfaceDeclRef,
    WitnessTable* witnessTable,
    SourceLoc conformanceLoc)
{
    auto stageKind = getStructuralRayTracingStageKind(superInterfaceDeclRef.getDecl());
    if (stageKind == StructuralRayTracingStageKind::Count || !witnessTable)
        return;

    auto witnessedType = witnessTable->witnessedType;
    auto witnessedDeclRef =
        isDeclRefTypeOf<AggTypeDecl>(witnessedType ? as<Type>(witnessedType->resolve()) : nullptr);
    auto witnessedDecl = witnessedDeclRef ? witnessedDeclRef.getDecl() : nullptr;
    if (witnessedDecl)
    {
        _checkRayTracingAPIUse(
            getModule(witnessedDecl),
            RayTracingAPIFamily::Structural,
            witnessedDecl,
            getSink());
    }

    _checkStructuralRayTracingStageStorage(this, witnessedType, conformanceLoc);
}

// Returns the section name for a structural schema requirement used in duplicate-entry diagnostics,
// or null for other associated-type requirements.
static const char* _getStructuralRayTracingSchemaSectionName(AssocTypeDecl* requirement)
{
    switch (getStructuralRayTracingAssociatedTypeKind(requirement))
    {
    case StructuralRayTracingAssociatedTypeKind::ProgramHitGroups:
        return "hit-group";
    case StructuralRayTracingAssociatedTypeKind::ProgramMissShaders:
        return "miss-shader";
    case StructuralRayTracingAssociatedTypeKind::ProgramCallableShaders:
        return "callable-shader";
    default:
        break;
    }
    return nullptr;
}

void SemanticsVisitor::diagnoseDuplicateStructuralRayTracingSchemaEntries(
    Type* entryListType,
    AssocTypeDecl* associatedTypeRequirement,
    Type* schemaType,
    Decl* satisfyingDecl)
{

    auto sectionName = _getStructuralRayTracingSchemaSectionName(associatedTypeRequirement);
    if (!sectionName)
        return;

    SLANG_RELEASE_ASSERT(entryListType && schemaType && satisfyingDecl);

    // Consider this example:
    //
    //     typealias HitGroups = HitGroupList<OpaqueHit, OpaqueHit>;
    //
    // The type pack in this checked associated-type witness is the source declaration's exact
    // list of implementations. Each implementation receives one dense function index; it
    // does not stand for a physical SBT record. A host can therefore reuse `OpaqueHit` in any
    // number of records, but listing it twice here would assign one implementation two indices and
    // make reflection ambiguous. Reject that source contract before lowering, because the IR and
    // reflection representations intentionally rely on unique entries.
    //
    // Canonical AST identity makes aliases name the same entry without reconstructing or
    // structurally comparing types here.
    auto entries =
        getStructuralRayTracingEntryPack(getASTBuilder(), entryListType->getCanonicalType());
    HashSet<Type*> seenEntries;
    for (Index i = 0; i < entries.types->getTypeCount(); ++i)
    {
        auto entryType = entries.types->getElementType(i)->getCanonicalType();
        if (seenEntries.add(entryType))
            continue;

        getSink()->diagnose(Diagnostics::DuplicateStructuralRayTracingEntry{
            .section = sectionName,
            .entry = entryType,
            .schema = schemaType->getCanonicalType(),
            .location = satisfyingDecl->loc});
    }
}

static bool _isLegacyRayTracingStage(Stage stage)
{
    switch (stage)
    {
    case Stage::ClosestHit:
    case Stage::AnyHit:
    case Stage::Intersection:
    case Stage::Miss:
    case Stage::Callable:
        return true;
    default:
        return false;
    }
}

// Includes command-line-selected native stages in the module's API-family check.
// For example, `-entry ordinaryMiss -stage miss` selects a legacy entry even without [shader].
// Retaining the first source use on the module also preserves this rule after serialization.
void diagnoseMixedRayTracingAPIUse(EntryPoint* entryPoint, DiagnosticSink* sink)
{
    if (!_isLegacyRayTracingStage(entryPoint->getStage()))
        return;

    auto entryPointDecl = entryPoint->getFuncDecl();
    auto family = entryPoint->isStructuralRayTracingEntryPoint() ? RayTracingAPIFamily::Structural
                                                                 : RayTracingAPIFamily::Legacy;
    _checkRayTracingAPIUse(getModule(entryPointDecl), family, entryPointDecl, sink);
}

static void _checkAttributedLegacyEntryPoints(
    Module* module,
    ContainerDecl* containerDecl,
    DiagnosticSink* sink)
{
    // A legacy ray-tracing entry point can be present without being named on the command line.
    // Walk the checked declarations before module diagnostics finish so a structural conformance
    // elsewhere in the same module cannot evade the mixed-API rule merely because the legacy
    // function was not selected for the current compile request.
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto innerMember = member;
        if (auto genericDecl = as<GenericDecl>(innerMember))
            innerMember = genericDecl->inner;

        if (auto functionDecl = as<FuncDecl>(innerMember))
        {
            if (auto entryPointAttr = functionDecl->findModifier<EntryPointAttribute>())
            {
                auto stage =
                    getStageFromAtom(CapabilitySet{entryPointAttr->capabilitySet}.getTargetStage());
                if (_isLegacyRayTracingStage(stage))
                {
                    _checkRayTracingAPIUse(module, RayTracingAPIFamily::Legacy, functionDecl, sink);
                }
            }
        }

        if (auto childContainer = as<ContainerDecl>(innerMember))
            _checkAttributedLegacyEntryPoints(module, childContainer, sink);
    }
}

// Returns the native stage that constrains a function accepting a structural stage input.
//
// Consider this example:
//
//     [shader("anyhit")]
//     void nativeAnyHit(rt::ClosestHitInput<C> input) { ... }
//
// `nativeAnyHit` is not selected by a structural stage conformance. Its checked
// `EntryPointAttribute` is nevertheless an explicit any-hit contract and must win over the fallback
// that infers a stage from an otherwise-unannotated helper's first stage-input parameter. Preserve
// the native `Stage` here: mapping a compute or miss entry point to
// `StructuralRayTracingStageKind::Count` would make a known mismatched stage look the same as an
// unconstrained helper.
static Stage _getRequiredStageForStructuralInput(FunctionDeclBase* functionDecl)
{
    if (auto entryPointAttribute = functionDecl->findModifier<EntryPointAttribute>())
    {
        auto stageAtom = CapabilitySet{entryPointAttribute->capabilitySet}.getTargetStage();
        if (stageAtom != CapabilityAtom::Invalid)
            return getStageFromAtom(stageAtom);
    }

    CapabilitySet declaredCapabilities;
    for (auto decl = static_cast<Decl*>(functionDecl); decl; decl = decl->parentDecl)
    {
        for (auto requirement : decl->getModifiersOfType<RequireCapabilityAttribute>())
            declaredCapabilities.unionWith(requirement->capabilitySet);
        if (as<ModuleDecl>(decl))
            break;
    }

    auto stageAtom = declaredCapabilities.getUniquelyImpliedStageAtom();
    if (stageAtom == CapabilityAtom::Invalid)
        return Stage::Unknown;
    return getStageFromAtom(stageAtom);
}

// Checks the stage constraint on each input parameter. For example,
// `helper(ClosestHitInput<C> hit, MissInput<C> miss)` mixes incompatible stages and is rejected.
// A selected conformance supplies the stage for an implementation; ordinary helper functions use
// their declared stage or first input.
static void _diagnoseStructuralStageInputParameters(
    SemanticsVisitor* visitor,
    DeclRef<FunctionDeclBase> function,
    Stage functionStage)
{
    auto functionDecl = function.getDecl();
    for (auto parameter : functionDecl->getParameters())
    {
        auto parameterType = function.substitute(visitor->getASTBuilder(), parameter->type.type);
        auto inputStage = _getDirectStageInputKind(parameterType);
        if (inputStage == StructuralRayTracingStageKind::Count)
            continue;

        _checkRayTracingAPIUse(
            getModule(functionDecl),
            RayTracingAPIFamily::Structural,
            functionDecl,
            visitor->getSink());
        auto requiredInputStage = _getNativeStage(inputStage);
        if (functionStage == Stage::Unknown)
        {
            functionStage = requiredInputStage;
            continue;
        }
        if (requiredInputStage == functionStage)
            continue;

        auto location = parameter->type.exp ? parameter->type.exp->loc : parameter->loc;
        visitor->diagnoseOnce(Diagnostics::StructuralRayTracingInputStageMismatch{
            .type = parameterType,
            .stage = getStageName(requiredInputStage),
            .function = functionDecl,
            .location = location});
    }
}

static DeclRef<FuncDecl> _getStageImplementationFromSubtypeWitness(
    ASTBuilder* astBuilder,
    InterfaceDecl* stageInterface,
    SubtypeWitness* witness)
{
    witness = witness ? as<SubtypeWitness>(witness->resolve()) : nullptr;
    auto invokeRequirement = getStructuralRayTracingStageInvokeRequirement(stageInterface);
    if (!invokeRequirement || !witness)
        return DeclRef<FuncDecl>();
    auto invokeWitness = tryLookUpRequirementWitness(astBuilder, witness, invokeRequirement);
    if (invokeWitness.getFlavor() != RequirementWitness::Flavor::declRef)
        return DeclRef<FuncDecl>();

    // The requirement witness is also the source of truth for the implementation's outer generic
    // substitutions. Returning only its `FuncDecl*` would turn `GenericMiss<MyPayload>.invoke`
    // back into the unspecialized declaration and make AST-to-IR lowering encounter a free generic
    // type. Preserve the checked declaration reference exactly as the conformance produced it.
    return invokeWitness.getDeclRef().as<FuncDecl>();
}

// Checks the functions selected by one completed conformance. Consider a generic extension that
// supplies invoke for both an ordinary type and a miss-stage type: the method declaration alone
// has no single stage. The stage belongs to the conformance whose witness selects that method.
static void _diagnoseStructuralStageConformance(
    SemanticsVisitor* visitor,
    InheritanceDecl* inheritanceDecl)
{
    auto witnessTable = inheritanceDecl->witnessTable;
    auto interfaceType = as<DeclRefType>(inheritanceDecl->base.type);
    if (!witnessTable || !interfaceType || !interfaceType->getDeclRef().as<InterfaceDecl>())
        return;
    auto astBuilder = visitor->getASTBuilder();
    auto inheritanceDeclRef =
        createDefaultSubstitutionsIfNeeded(astBuilder, visitor, makeDeclRef(inheritanceDecl));
    auto witness = astBuilder->getDeclaredSubtypeWitness(
        witnessTable->witnessedType,
        interfaceType,
        inheritanceDeclRef);
    for (auto facet : visitor->getShared()->getInheritanceInfo(interfaceType).facets)
    {
        auto stageInterface = facet->origin.declRef.as<InterfaceDecl>();
        if (!isExecutableStructuralRayTracingStageInterface(stageInterface.getDecl()))
            continue;
        auto stageWitness =
            visitor->getShared()->tryProjectInterfaceSubtypeWitness(witness, facet->getType());
        auto implementation = _getStageImplementationFromSubtypeWitness(
            astBuilder,
            stageInterface.getDecl(),
            stageWitness);
        if (!implementation)
            continue;
        auto stage = _getNativeStage(getStructuralRayTracingStageKind(stageInterface.getDecl()));
        auto functionDecl = implementation.getDecl();
        auto capabilities = functionDecl->inferredCapabilityRequirements;
        if (capabilities && capabilities->isIncompatibleWith(getAtomFromStage(stage)))
        {
            visitor->diagnoseOnce(Diagnostics::DeclHasDependenciesNotCompatibleOnStage{
                .stage = getStageName(stage),
                .decl = functionDecl});
        }
        _diagnoseStructuralStageInputParameters(visitor, implementation, stage);
    }
}

// Runs after ordinary declaration, body, and capability checking has completed. At that point
// conformance witnesses contain the selected methods, including inherited/default implementations.
static void _diagnoseStructuralStageDeclarations(
    SemanticsVisitor* visitor,
    ContainerDecl* containerDecl)
{
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto innerMember = member;
        if (auto genericDecl = as<GenericDecl>(innerMember))
            innerMember = genericDecl->inner;
        if (auto inheritanceDecl = as<InheritanceDecl>(innerMember))
            _diagnoseStructuralStageConformance(visitor, inheritanceDecl);
        if (auto functionDecl = as<FunctionDeclBase>(innerMember))
        {
            auto function = createDefaultSubstitutionsIfNeeded(
                visitor->getASTBuilder(),
                visitor,
                makeDeclRef(functionDecl));
            _diagnoseStructuralStageInputParameters(
                visitor,
                function.as<FunctionDeclBase>(),
                _getRequiredStageForStructuralInput(functionDecl));
        }
        if (auto childContainer = as<ContainerDecl>(innerMember))
            _diagnoseStructuralStageDeclarations(visitor, childContainer);
    }
}

// Defers storage checks until ordinary module checking has completed type inheritance.
// For example, inspecting Box<T> while T's generic constraints are still being checked must not
// recursively ask the inheritance checker to finish that same unfinished declaration.
void SemanticsVisitor::beginStructuralRayTracingModule()
{
    SLANG_RELEASE_ASSERT(!getShared()->m_deferStructuralRayTracingTypeUses);
    getShared()->m_deferStructuralRayTracingTypeUses = true;
}

// Finishes stage and module-wide rules after checked conformance witnesses are available.
// For example, a generic extension can supply Miss.invoke; checking its selected stage before
// ordinary conformance and capability inference finish would inspect an incomplete signature.
void SemanticsVisitor::checkStructuralRayTracingModule(ModuleDecl* moduleDecl)
{
    _checkAttributedLegacyEntryPoints(getModule(moduleDecl), moduleDecl, getSink());
    _diagnoseStructuralStageDeclarations(this, moduleDecl);
}

static Stage _getNativeStage(StructuralRayTracingStageKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return Stage::ClosestHit;
    case StructuralRayTracingStageKind::AnyHit:
        return Stage::AnyHit;
    case StructuralRayTracingStageKind::Intersection:
        return Stage::Intersection;
    case StructuralRayTracingStageKind::Miss:
        return Stage::Miss;
    case StructuralRayTracingStageKind::Callable:
        return Stage::Callable;
    default:
        return Stage::Unknown;
    }
}

static StructuralRayTracingStageKind _getStructuralStage(Stage stage)
{
    switch (stage)
    {
    case Stage::ClosestHit:
        return StructuralRayTracingStageKind::ClosestHit;
    case Stage::AnyHit:
        return StructuralRayTracingStageKind::AnyHit;
    case Stage::Intersection:
        return StructuralRayTracingStageKind::Intersection;
    case Stage::Miss:
        return StructuralRayTracingStageKind::Miss;
    case Stage::Callable:
        return StructuralRayTracingStageKind::Callable;
    default:
        return StructuralRayTracingStageKind::Count;
    }
}

// Reads the selected stage's ABI types through its checked requirement witnesses.
// For example, Miss.Context.Payload = Color means the generated miss adapter receives Color.
// Preserve the conformance path: a context with several interface conformances may give Record
// different meanings on those paths, and metadata must agree with the stage input's accessor.
static bool _populateStructuralEntryPointInfo(
    SemanticsVisitor* visitor,
    StructuralRayTracingStageKind stageKind,
    SubtypeWitness* stageWitness,
    StructuralRayTracingEntryPointInfo* outInfo)
{
    // Resolve the stage ABI contract through the selected conformance witness. This preserves
    // generic substitutions and makes the checked associated types the single source of truth for
    // the non-operational IR metadata retained for later adapter synthesis.
    outInfo->stageKind = stageKind;
    auto astBuilder = visitor->getASTBuilder();
    auto contextRequirement = StructuralRayTracingAssociatedTypeKind::StageContext;
    auto contextMember = StructuralRayTracingAssociatedTypeKind::HitPrimitive;
    if (stageKind == StructuralRayTracingStageKind::Miss)
        contextMember = StructuralRayTracingAssociatedTypeKind::PayloadContextPayload;
    else if (stageKind == StructuralRayTracingStageKind::Callable)
        contextMember = StructuralRayTracingAssociatedTypeKind::CallableData;
    outInfo->contextType =
        resolveStructuralRayTracingAssociatedType(astBuilder, stageWitness, contextRequirement);
    auto contextWitness = resolveStructuralRayTracingAssociatedTypeConstraint(
        astBuilder,
        stageWitness,
        contextRequirement,
        contextMember);
    if (!outInfo->contextType || !contextWitness)
        return false;

    switch (stageKind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
    case StructuralRayTracingStageKind::AnyHit:
    case StructuralRayTracingStageKind::Intersection:
        {
            outInfo->recordType = resolveStructuralRayTracingAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::StageRecord);
            if (stageKind != StructuralRayTracingStageKind::Intersection)
            {
                outInfo->payloadType = resolveStructuralRayTracingAssociatedType(
                    astBuilder,
                    contextWitness,
                    StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
            }

            auto primitiveWitness = resolveStructuralRayTracingAssociatedTypeConstraint(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::HitPrimitive,
                StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes);
            outInfo->hitAttributesType = resolveStructuralRayTracingAssociatedType(
                astBuilder,
                primitiveWitness,
                StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes);
            outInfo->primitiveType = resolveStructuralRayTracingAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::HitPrimitive);
            outInfo->hitAttributesKind =
                getStructuralRayTracingHitAttributesKind(outInfo->primitiveType);
            return (stageKind == StructuralRayTracingStageKind::Intersection ||
                    outInfo->payloadType) &&
                   outInfo->recordType && outInfo->primitiveType && outInfo->hitAttributesType &&
                   outInfo->hitAttributesKind != StructuralRayTracingHitAttributesKind::None;
        }
    case StructuralRayTracingStageKind::Miss:
        {
            outInfo->payloadType = resolveStructuralRayTracingAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
            outInfo->recordType = resolveStructuralRayTracingAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::StageRecord);
            return outInfo->payloadType && outInfo->recordType;
        }
    case StructuralRayTracingStageKind::Callable:
        outInfo->callableDataType = resolveStructuralRayTracingAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::CallableData);
        outInfo->recordType = resolveStructuralRayTracingAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::StageRecord);
        return outInfo->callableDataType && outInfo->recordType;
    default:
        return false;
    }
}

// Selects one executable conformance for a source struct named by -entry.
// For example, a struct implementing both IMissShader and ICallableShader needs `-stage miss`
// or `-stage callable`; a struct implementing only IMissShader can omit the explicit stage.
// The selected witness supplies the exact generic invoke implementation and all ABI types.
DeclRef<FuncDecl> findStructuralRayTracingEntryPointByName(
    Linkage* linkage,
    Module* module,
    Name* name,
    Profile& ioProfile,
    DiagnosticSink* sink,
    bool* outFoundStruct,
    StructuralRayTracingEntryPointInfo* outInfo)
{
    // A structural entry request names a stage struct rather than its `invoke` method. Resolve the
    // struct in the requested module, select exactly one structural stage conformance (or the
    // `-stage`), and return the specialized witness method that ordinary entry-point lowering can
    // compile. The public entry name remains the struct name and is stored separately on
    // `EntryPoint`.
    *outFoundStruct = false;
    *outInfo = {};
    auto expr = module->findDeclFromString(getText(name), sink);
    auto declRefExpr = as<DeclRefExpr>(expr);
    auto stageTypeDeclRef =
        declRefExpr ? declRefExpr->declRef.as<AggTypeDecl>() : DeclRef<AggTypeDecl>();
    if (!stageTypeDeclRef || getModule(stageTypeDeclRef.getDecl()) != module)
        return DeclRef<FuncDecl>();

    *outFoundStruct = true;
    SharedSemanticsContext sharedContext(linkage, module, sink);
    for (auto dependency : module->getModuleDependencies())
    {
        auto moduleDecl = dependency->getModuleDecl();
        if (sharedContext.importedModulesSet.add(moduleDecl))
            sharedContext.importedModulesList.add(moduleDecl);
    }
    SemanticsVisitor visitor(&sharedContext);
    visitor.ensureDecl(stageTypeDeclRef, DeclCheckState::ReadyForConformances);

    DeclRef<FuncDecl> stageImplementations[int(StructuralRayTracingStageKind::Count)] = {};
    SubtypeWitness* stageWitnesses[int(StructuralRayTracingStageKind::Count)] = {};
    InterfaceDecl* stageInterfaces[int(StructuralRayTracingStageKind::Count)] = {};
    bool ambiguousImplementations[int(StructuralRayTracingStageKind::Count)] = {};
    auto stageType = DeclRefType::create(linkage->getASTBuilder(), stageTypeDeclRef);
    outInfo->stageType = stageType;
    for (auto facet : visitor.getShared()->getInheritanceInfo(stageType).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = getStructuralRayTracingStageKind(interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingStageKind::Count)
        {
            if (auto implementation = _getStageImplementationFromSubtypeWitness(
                    visitor.getASTBuilder(),
                    interfaceDeclRef.getDecl(),
                    facet->subtypeWitness))
            {
                // Two module-local interfaces can identify the same logical stage. Repeated
                // facets selecting the same checked method are harmless, but distinct methods
                // cannot be selected by a stage name alone. Keep the first witness and diagnose
                // only if this is the stage requested by the entry point.
                if (stageImplementations[int(kind)])
                {
                    if (stageImplementations[int(kind)] != implementation)
                        ambiguousImplementations[int(kind)] = true;
                    continue;
                }
                stageImplementations[int(kind)] = implementation;
                stageWitnesses[int(kind)] = facet->subtypeWitness;
                stageInterfaces[int(kind)] = interfaceDeclRef.getDecl();
            }
        }
    }

    Count implementedStageCount = 0;
    StructuralRayTracingStageKind onlyImplementedStage = StructuralRayTracingStageKind::Count;
    for (int i = 0; i < int(StructuralRayTracingStageKind::Count); ++i)
    {
        if (stageImplementations[i])
        {
            ++implementedStageCount;
            onlyImplementedStage = StructuralRayTracingStageKind(i);
        }
    }

    if (implementedStageCount == 0)
    {
        sink->diagnose(Diagnostics::StructuralRayTracingEntryPointNotStage{
            .stageType = stageTypeDeclRef.getDecl()});
        return DeclRef<FuncDecl>();
    }

    auto requestedStage = ioProfile.getStage();
    auto selectedStage = _getStructuralStage(requestedStage);
    if (requestedStage != Stage::Unknown)
    {
        if (selectedStage == StructuralRayTracingStageKind::Count ||
            !stageImplementations[int(selectedStage)])
        {
            sink->diagnose(Diagnostics::StructuralRayTracingEntryPointStageMismatch{
                .stage = getStageName(requestedStage),
                .stageType = stageTypeDeclRef.getDecl()});
            return DeclRef<FuncDecl>();
        }
    }
    else if (implementedStageCount == 1)
    {
        selectedStage = onlyImplementedStage;
        ioProfile = Profile(_getNativeStage(selectedStage));
    }
    else
    {
        sink->diagnose(Diagnostics::StructuralRayTracingEntryPointAmbiguousStage{
            .stageType = stageTypeDeclRef.getDecl()});
        return DeclRef<FuncDecl>();
    }

    if (ambiguousImplementations[int(selectedStage)])
    {
        sink->diagnose(Diagnostics::StructuralRayTracingEntryPointAmbiguousImplementation{
            .stage = getStageName(_getNativeStage(selectedStage)),
            .stageType = stageTypeDeclRef.getDecl()});
        return DeclRef<FuncDecl>();
    }

    auto invokeMethod = stageImplementations[int(selectedStage)];
    if (!invokeMethod)
    {
        sink->diagnose(Diagnostics::InternalCompilerError{.location = stageTypeDeclRef.getLoc()});
        return DeclRef<FuncDecl>();
    }

    outInfo->stageInterface = stageInterfaces[int(selectedStage)];
    if (!_populateStructuralEntryPointInfo(
            &visitor,
            selectedStage,
            stageWitnesses[int(selectedStage)],
            outInfo))
    {
        sink->diagnose(Diagnostics::InternalCompilerError{.location = stageTypeDeclRef.getLoc()});
        return DeclRef<FuncDecl>();
    }
    return invokeMethod;
}

enum class StructuralRayTracingRuntimeTypeKind
{
    None,
    Stage,
    StageInput,
};

static StructuralRayTracingStageKind _getDirectStageInputKind(Type* type)
{
    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return getStructuralRayTracingStageInputKind(typeDecl);
}

static StructuralRayTracingRuntimeTypeKind _getInterfaceRuntimeTypeKind(
    InterfaceDecl* interfaceDecl)
{
    if (getStructuralRayTracingStageKind(interfaceDecl) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::Stage;
    return StructuralRayTracingRuntimeTypeKind::None;
}

// Completed inheritance facets include direct bases, refinements, extension conformances, and
// generic constraints. Type-use checks are deferred during module checking so this query never
// forces an unfinished signature merely to classify its eventual runtime use.
static StructuralRayTracingRuntimeTypeKind _getDirectStructuralRuntimeTypeKind(
    SemanticsVisitor* visitor,
    Type* type)
{
    for (auto facet : visitor->getShared()->getInheritanceInfo(type).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = _getInterfaceRuntimeTypeKind(interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingRuntimeTypeKind::None)
            return kind;
    }
    return StructuralRayTracingRuntimeTypeKind::None;
}

// Finds actual stage or stage-input storage, including specialized fields and containers.
// For example, Box<MissInput<C>> stores an input when Box<T> declares `T value`.
// Walk the normal substituted field types; generic-argument restrictions are checked separately.
static StructuralRayTracingRuntimeTypeKind _findStructuralRuntimeType(
    SemanticsVisitor* visitor,
    Type* type,
    HashSet<Type*>& seenTypes,
    SourceLoc location,
    UInt depth = 0)
{
    if (!type || as<ErrorType>(type))
        return StructuralRayTracingRuntimeTypeKind::None;

    // LoopField<T> { LoopField<T, int> next; } creates a different canonical type at every
    // field step, so the cycle set cannot stop it. Apply the same nesting budget as constructor
    // and layout checking; this is invalid source nesting, not an alternative type representation.
    if (depth >= kMaxTypeNestingDepth)
    {
        visitor->getSink()->diagnose(
            Diagnostics::MaximumTypeNestingLevelExceeded{.location = location});
        return StructuralRayTracingRuntimeTypeKind::None;
    }

    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();

    if (_getDirectStageInputKind(type) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::StageInput;
    auto directKind = _getDirectStructuralRuntimeTypeKind(visitor, type);
    if (directKind != StructuralRayTracingRuntimeTypeKind::None)
        return directKind;

    if (auto typePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < typePack->getTypeCount(); ++i)
        {
            auto kind = _findStructuralRuntimeType(
                visitor,
                typePack->getElementType(i),
                seenTypes,
                location,
                depth + 1);
            if (kind != StructuralRayTracingRuntimeTypeKind::None)
                return kind;
        }
    }

    if (auto structType = as<DeclRefType>(type))
    {
        if (auto structDeclRef = structType->getDeclRef().as<StructDecl>())
        {
            // Consider this example:
            //
            //     struct Box<T> { T value; }
            //     Box<Box<ClosestHitInput<C>>> value;
            //
            // The outer and inner `Box` refer to the same `StructDecl`, but they are different
            // specialized types. Treating the declaration as the recursion key mistakes the
            // inner `Box` for a cycle and never reaches `ClosestHitInput<C>`. A canonical `Type`
            // preserves the specialization arguments while still giving equivalent spellings one
            // identity. Keep it only for this active recursion path so sibling fields can reuse the
            // same specialization without being skipped.
            auto canonicalStructType = type->getCanonicalType();
            if (!seenTypes.add(canonicalStructType))
                return StructuralRayTracingRuntimeTypeKind::None;

            auto result = StructuralRayTracingRuntimeTypeKind::None;
            for (auto fieldDeclRef :
                 getFields(visitor->getASTBuilder(), structDeclRef, MemberFilterStyle::Instance))
            {
                auto field = fieldDeclRef.getDecl();
                visitor->ensureDecl(field, DeclCheckState::CanUseTypeOfValueDecl);
                auto fieldType = getType(visitor->getASTBuilder(), fieldDeclRef);
                result =
                    _findStructuralRuntimeType(visitor, fieldType, seenTypes, location, depth + 1);
                if (result != StructuralRayTracingRuntimeTypeKind::None)
                    break;
            }
            seenTypes.remove(canonicalStructType);
            if (result != StructuralRayTracingRuntimeTypeKind::None)
                return result;
        }
    }

    if (auto arrayType = as<ArrayExpressionType>(type))
        return _findStructuralRuntimeType(
            visitor,
            arrayType->getElementType(),
            seenTypes,
            location,
            depth + 1);
    if (auto optionalType = as<OptionalType>(type))
        return _findStructuralRuntimeType(
            visitor,
            optionalType->getValueType(),
            seenTypes,
            location,
            depth + 1);
    if (auto pointerType = as<PtrTypeBase>(type))
        return _findStructuralRuntimeType(
            visitor,
            pointerType->getValueType(),
            seenTypes,
            location,
            depth + 1);
    if (auto tupleType = as<TupleType>(type))
    {
        for (Index i = 0; i < tupleType->getMemberCount(); ++i)
        {
            auto kind = _findStructuralRuntimeType(
                visitor,
                tupleType->getMember(i),
                seenTypes,
                location,
                depth + 1);
            if (kind != StructuralRayTracingRuntimeTypeKind::None)
                return kind;
        }
    }
    return StructuralRayTracingRuntimeTypeKind::None;
}

static StructuralRayTracingRuntimeTypeKind _findStructuralRuntimeType(
    SemanticsVisitor* visitor,
    Type* type,
    SourceLoc location)
{
    if (!type || as<ErrorType>(type))
        return StructuralRayTracingRuntimeTypeKind::None;
    HashSet<Type*> seenTypes;
    return _findStructuralRuntimeType(visitor, type, seenTypes, location);
}

static void _diagnoseInvalidStructuralRayTracingRuntimeType(
    SemanticsVisitor* visitor,
    StructuralRayTracingRuntimeTypeKind kind,
    Type* type,
    SourceLoc location)
{
    if (kind == StructuralRayTracingRuntimeTypeKind::Stage)
    {
        visitor->getSink()->diagnose(
            Diagnostics::StructuralRayTracingStageRuntimeValue{.type = type, .location = location});
    }
    else if (kind == StructuralRayTracingRuntimeTypeKind::StageInput)
    {
        visitor->getSink()->diagnose(
            Diagnostics::StructuralRayTracingInputStorage{.type = type, .location = location});
    }
}

// Returns a checked generic argument when it directly denotes a structural stage, stage-input
// type. Type aliases are resolved first because they are alternate names for the same
// semantic type. Callers exempt compiler-provided structural types whose own generic arguments
// form their intended compile-time representation.
static Type* _getDirectStructuralRuntimeGenericArgument(
    SemanticsVisitor* visitor,
    Type* argument,
    StructuralRayTracingRuntimeTypeKind& outKind)
{
    // A variadic function receives its direct `each T` substitution as one concrete pack. Inspect
    // those immediate elements just as non-variadic arguments are inspected, without following
    // arbitrary substitution graphs or looking through ordinary user-defined containers.
    if (auto typePack = as<ConcreteTypePack>(argument ? argument->resolve() : nullptr))
    {
        for (Index i = 0; i < typePack->getTypeCount(); ++i)
        {
            if (auto invalidType = _getDirectStructuralRuntimeGenericArgument(
                    visitor,
                    typePack->getElementType(i),
                    outKind))
            {
                return invalidType;
            }
        }
        return nullptr;
    }

    auto type = argument ? as<Type>(argument->resolve()) : nullptr;
    if (!type || as<ErrorType>(type))
        return nullptr;

    if (_getDirectStageInputKind(type) != StructuralRayTracingStageKind::Count)
    {
        outKind = StructuralRayTracingRuntimeTypeKind::StageInput;
        return type;
    }

    auto kind = _getDirectStructuralRuntimeTypeKind(visitor, type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return nullptr;
    outKind = kind;
    return type;
}

// Recognizes the library's shader lists, whose packs name implementations without storing them.
// For example, MissShaderList<MyMiss> is allowed even though MyMiss cannot be an ordinary generic
// value. The annotation identifies this checked library representation; it grants no module-wide
// exemption to unrelated helpers or extensions.
static bool _isStructuralRayTracingTypeList(Decl* decl)
{
    return _getRayTracingBuiltin(decl) == KnownBuiltinDeclName::RayTracingShaderList;
}

static bool _isStructuralRayTracingTypeApplication(Type* type)
{
    auto declRef = isDeclRefTypeOf<AggTypeDecl>(type);
    return declRef && _isStructuralRayTracingTypeList(declRef.getDecl());
}

// Restricts stage/input values and generic substitutions under the current adapter contract.
// For example, Box<MissInput<C>> cannot store an input, and Factory<MissInput<C>> cannot defer
// manufacturing it to a generic method. A direct read-only parameter is supplied by the adapter.
// Schema/group/list values and their use as ordinary generic arguments are unrestricted.
static bool _checkStructuralRayTracingTypeUse(
    SemanticsVisitor* visitor,
    const StructuralRayTracingTypeUse& use)
{
    if (use.kind == StructuralRayTracingTypeUse::Kind::GenericArguments)
    {
        auto applicationType = use.genericApplicationType;
        if (applicationType &&
            (_getDirectStageInputKind(applicationType) != StructuralRayTracingStageKind::Count ||
             _getDirectStructuralRuntimeTypeKind(visitor, applicationType) !=
                 StructuralRayTracingRuntimeTypeKind::None ||
             _isStructuralRayTracingTypeApplication(applicationType)))
        {
            return false;
        }

        for (auto argument : use.genericArguments)
        {
            auto invalidKind = StructuralRayTracingRuntimeTypeKind::None;
            auto invalidType =
                _getDirectStructuralRuntimeGenericArgument(visitor, argument.type, invalidKind);
            if (!invalidType)
                continue;
            _diagnoseInvalidStructuralRayTracingRuntimeType(
                visitor,
                invalidKind,
                invalidType,
                argument.location);
            return true;
        }
        return false;
    }

    auto kind = _findStructuralRuntimeType(visitor, use.type, use.location);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return false;
    if (use.kind == StructuralRayTracingTypeUse::Kind::ReadOnlyInputParameter &&
        kind == StructuralRayTracingRuntimeTypeKind::StageInput &&
        _getDirectStageInputKind(use.type) != StructuralRayTracingStageKind::Count)
    {
        return false;
    }
    if (use.kind == StructuralRayTracingTypeUse::Kind::Construction)
    {
        visitor->getSink()->diagnose(Diagnostics::StructuralRayTracingTypeConstruction{
            .type = use.type,
            .location = use.location});
    }
    else
    {
        _diagnoseInvalidStructuralRayTracingRuntimeType(visitor, kind, use.type, use.location);
    }
    return true;
}

// Retains the exact checked type until module checking has completed the constraints that its
// inheritance depends on. Ad hoc reflection/specialization contexts have no module phase to drain
// a queue, and operate on already checked declarations, so they evaluate the use immediately.
static bool _diagnoseOrDeferStructuralRayTracingTypeUse(
    SemanticsVisitor* visitor,
    const StructuralRayTracingTypeUse& use)
{
    if (use.kind == StructuralRayTracingTypeUse::Kind::GenericArguments)
    {
        if (use.genericArguments.getCount() == 0)
            return false;
    }
    else if (!use.type || as<ErrorType>(use.type))
    {
        return false;
    }

    auto shared = visitor->getShared();
    if (shared->m_deferStructuralRayTracingTypeUses)
    {
        shared->m_pendingStructuralRayTracingTypeUses.add(use);
        return false;
    }
    return _checkStructuralRayTracingTypeUse(visitor, use);
}

void SemanticsVisitor::diagnosePendingStructuralRayTracingTypeUses()
{
    auto shared = getShared();
    SLANG_RELEASE_ASSERT(shared->m_deferStructuralRayTracingTypeUses);
    // Inheritance queries can finish a lazily synthesized declaration. Its own uses join this
    // queue, so copy the current record before checking and drain newly added records as well.
    for (Index i = 0; i < shared->m_pendingStructuralRayTracingTypeUses.getCount(); ++i)
    {
        auto use = shared->m_pendingStructuralRayTracingTypeUses[i];
        _checkStructuralRayTracingTypeUse(this, use);
    }
    shared->m_pendingStructuralRayTracingTypeUses.clear();
    shared->m_deferStructuralRayTracingTypeUses = false;
}

// Allows stage inputs only as direct read-only parameters, not as stored variables or references.
// For example, `void helper(in rt::MissInput<C> input)` can borrow the current stage view;
// `rt::MissInput<C> saved;` and `void helper(out rt::MissInput<C> input)` cannot create that state.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingVariableType(VarDeclBase* varDecl)
{
    auto paramDecl = as<ParamDecl>(varDecl);
    auto isReadOnlyValueParameter = paramDecl && !paramDecl->hasModifier<OutModifier>() &&
                                    !paramDecl->hasModifier<RefModifier>() &&
                                    !paramDecl->hasModifier<BorrowModifier>() &&
                                    !paramDecl->hasModifier<HLSLPayloadModifier>();
    StructuralRayTracingTypeUse use;
    use.kind = isReadOnlyValueParameter ? StructuralRayTracingTypeUse::Kind::ReadOnlyInputParameter
                                        : StructuralRayTracingTypeUse::Kind::Value;
    use.type = varDecl->type.type;
    use.location = varDecl->loc;
    _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

// Prevents ordinary functions from returning a compiler-created stage or input view.
// For example, `rt::MissInput<C> makeInput()` cannot produce the state supplied by a miss adapter.
// Constructor results are handled at their actual construction use instead of their declaration.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingCallableResult(CallableDecl* callableDecl)
{
    if (as<ConstructorDecl>(callableDecl))
        return;
    StructuralRayTracingTypeUse use;
    use.type = callableDecl->returnType.type;
    use.location =
        callableDecl->returnType.exp ? callableDecl->returnType.exp->loc : callableDecl->loc;
    _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

// Prevents properties from manufacturing stage/input views outside the adapter parameter.
// For example, `property rt::MissInput<C> input { get; }` promises a value with no source storage.
// Payload and record properties have ordinary application-defined result types and remain legal.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingPropertyType(PropertyDecl* propertyDecl)
{
    StructuralRayTracingTypeUse use;
    use.type = propertyDecl->type.type;
    use.location = propertyDecl->type.exp ? propertyDecl->type.exp->loc : propertyDecl->loc;
    _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

// Rejects direct source construction of values whose state must come from the stage adapter.
// For example, `rt::MissInput<C>()` cannot create a current ray or a reference to its payload.
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingConstruction(InvokeExpr* invoke)
{
    auto typeType = as<TypeType>(invoke->functionExpr->type);
    if (!typeType)
        return false;
    StructuralRayTracingTypeUse use;
    use.kind = StructuralRayTracingTypeUse::Kind::Construction;
    // Constructor resolution can replace the type expression with a function reference. Preserve
    // its checked Type here, before that ordinary expression rewrite loses the construction form.
    use.type = typeType->getType();
    use.location = invoke->functionExpr->loc;
    return _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

// Checks the concrete result of a call, including results specialized from ordinary generics.
// For example, `makeDefault<rt::MissInput<C>>()` must not manufacture an input just because the
// generic function's declared return type was T.
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingInvokeResult(InvokeExpr* invoke)
{
    StructuralRayTracingTypeUse use;
    use.type = invoke->type;
    use.location = invoke->loc;
    return _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

// Prevents a generic body from manufacturing stage/input values after specialization.
// Consider `void make<T>() { T local = T(); }` called as make<MissInput<C>>(). Ordinary generic
// checking sees only T, and the call returns void; the concrete storage would escape the other
// frontend checks. Keep the conservative stage/input argument rule until such uses are checked
// after specialization. Schema/list/group types have ordinary value semantics and remain legal.
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericArguments(InvokeExpr* invoke)
{
    auto functionDeclRef = as<DeclRefExpr>(invoke->functionExpr);
    if (!functionDeclRef)
        return false;

    auto functionDecl = as<FunctionDeclBase>(functionDeclRef->declRef.getDecl());
    if (isStructuralRayTracingTraceMethod(functionDecl) ||
        isStructuralRayTracingCallShaderMethod(functionDecl))
        return false;

    StructuralRayTracingTypeUse use;
    use.kind = StructuralRayTracingTypeUse::Kind::GenericArguments;
    SubstitutionSet(functionDeclRef->declRef)
        .forEachGenericSubstitution(
            [&](GenericDecl* genericDecl, Val::OperandView<Val> arguments)
            {
                if (_isStructuralRayTracingTypeList(genericDecl->inner))
                    return;
                for (auto argument : arguments)
                {
                    // Witnesses and value arguments do not represent runtime types. In particular,
                    // do not resolve a witness while its generic signature is still being formed.
                    if (auto argumentType = as<Type>(argument))
                        use.genericArguments.add({argumentType, invoke->functionExpr->loc});
                }
            });
    return _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

// Rejects stage/input arguments in arbitrary generic types for the same specialization reason.
// For example, even an empty Factory<T> can define a method that constructs T internally.
// The standard shader lists explicitly represent type packs and do not manufacture their entries.
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericTypeApplication(
    GenericAppExpr* genericApplication,
    Expr* checkedResult)
{
    StructuralRayTracingTypeUse use;
    use.kind = StructuralRayTracingTypeUse::Kind::GenericArguments;
    auto applicationTypeType = checkedResult ? as<TypeType>(checkedResult->type) : nullptr;
    use.genericApplicationType = applicationTypeType ? applicationTypeType->getType() : nullptr;

    // Consider `Phantom<ClosestHitInput<C>>`: even an empty Phantom has a forbidden direct type
    // argument. Retain checked arguments before the GenericAppExpr is replaced by its resolved
    // declaration reference. The result type identifies schema containers that intentionally own
    // their structural arguments; the exemption is evaluated after their inheritance is complete.
    for (auto argument : genericApplication->arguments)
    {
        if (auto argumentType = as<TypeType>(argument->type))
            use.genericArguments.add({argumentType->getType(), argument->loc});
    }
    return _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

// Finds whether a checked call names the receiver's selected stage implementation. Consider:
//
//     interface IProvider { associatedtype Context : rt::IPayloadContext; }
//     extension<T : IProvider> T
//     { void invoke(in rt::MissInput<Context> input) {} }
//     struct Miss : IProvider, rt::IMissShader { typealias Context = MyContext; }
//     struct Ordinary : IProvider { typealias Context = MyContext; }
//
// Both types use the same function declaration, but only Miss selects it for a stage contract.
// Member lookup has already retained the receiver type and the specialized callee DeclRef. Match
// that callee against the receiver's checked conformance, preserving its generic substitutions.
// Unqualified calls in a method are MemberExprs with an implicit ThisExpr base as well.
static bool _isDirectStructuralRayTracingStageInvoke(SemanticsVisitor* visitor, DeclRefExpr* callee)
{
    auto member = as<MemberExpr>(callee);
    if (!member)
        return false;
    auto receiverType = unwrapModifiedType(member->baseExpression->type.type);
    for (auto facet : visitor->getShared()->getInheritanceInfo(receiverType).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        if (!isExecutableStructuralRayTracingStageInterface(interfaceDeclRef.getDecl()))
            continue;
        auto implementation = _getStageImplementationFromSubtypeWitness(
            visitor->getASTBuilder(),
            interfaceDeclRef.getDecl(),
            facet->subtypeWitness);
        if (implementation && implementation.declRefBase->equals(callee->declRef.declRefBase))
            return true;
    }
    return false;
}

// Rejects ordinary calls to the method selected as an executable stage implementation.
// For example, `stage.invoke(input)` must enter through pipeline dispatch, which supplies stage
// state and handles any-hit termination. An unrelated ordinary type's method named invoke is legal.
bool SemanticsVisitor::diagnoseDirectStructuralRayTracingStageInvoke(
    InvokeExpr* invoke,
    FunctionDeclBase* functionDecl)
{
    // A generic receiver such as T : IMissShader can refer directly to the interface requirement
    // before a concrete implementation is known. Its annotated owner already identifies the role.
    auto interfaceDecl = as<InterfaceDecl>(functionDecl->parentDecl);
    bool isStageRequirement =
        interfaceDecl &&
        functionDecl == getStructuralRayTracingStageInvokeRequirement(interfaceDecl);
    if (!isStageRequirement &&
        !_isDirectStructuralRayTracingStageInvoke(this, as<DeclRefExpr>(invoke->functionExpr)))
        return false;

    getSink()->diagnose(
        Diagnostics::DirectStructuralRayTracingStageInvoke{.location = invoke->functionExpr->loc});
    return true;
}

// Uses the source stage's capabilities when its native entry point will be synthesized.
//
// Consider `struct MyMiss : rt::IMissShader { ... }` compiled for Metal. The source role is
// `miss`, but Metal has no native miss entry point. The selected interface's declared
// capabilities describe the supported source role, including Metal's synthesized path. Native
// function entry points continue to use the capabilities of their ordinary stage profile.
CapabilitySet getEntryPointStageCapabilities(EntryPoint* entryPoint)
{
    if (!entryPoint->isStructuralRayTracingEntryPoint())
        return entryPoint->getProfile().getCapabilityName();

    auto stageInterface = entryPoint->getStructuralRayTracingInfo().stageInterface;
    SLANG_RELEASE_ASSERT(stageInterface && stageInterface->inferredCapabilityRequirements);
    return CapabilitySet{stageInterface->inferredCapabilityRequirements};
}

// Anchors capability diagnostics on the public entry-point declaration.
//
// Consider `-entry MyMiss` selecting `struct MyMiss : rt::IMissShader { ... }`. A capability
// error should name `MyMiss`, even though the selected function is its `invoke` witness. The
// capability provenance walk still starts at that function; this helper only chooses the
// declaration shown by the primary diagnostic. Native entry points already use their function.
Decl* getEntryPointCapabilityDiagnosticDecl(EntryPoint* entryPoint)
{
    if (!entryPoint->isStructuralRayTracingEntryPoint())
        return entryPoint->getFuncDecl();

    auto stageType =
        as<DeclRefType>(entryPoint->getStructuralRayTracingInfo().stageType->resolve());
    SLANG_RELEASE_ASSERT(stageType);
    return stageType->getDeclRef().getDecl();
}

// Creates the logical entry point selected by a stage-struct name and collects its requirements.
//
// Consider `-entry Stages.Miss` selecting a nested struct that implements `rt::IMissShader`.
// Conformance lookup chooses the exact `invoke` witness, but reflection keeps `Stages.Miss` as
// the source identity and derives a legal physical symbol for targets that cannot spell a dot.
// The logical input signature is not a native shader ABI, so the caller skips native varying
// checks and uses the returned capabilities with the shared target/profile validator instead.
// A name that does not select a struct leaves `outFoundStructuralStage` false, allowing ordinary
// function entry-point lookup to continue.
RefPtr<EntryPoint> tryCreateStructuralRayTracingEntryPoint(
    FrontEndEntryPointRequest* entryPointReq,
    bool* outFoundStructuralStage,
    CapabilitySet* outCapabilities)
{
    auto compileRequest = entryPointReq->getCompileRequest();
    auto linkage = compileRequest->getLinkage();
    auto entryPointName = entryPointReq->getName();
    auto entryPointProfile = entryPointReq->getProfile();
    StructuralRayTracingEntryPointInfo structuralInfo;
    auto implementation = findStructuralRayTracingEntryPointByName(
        linkage,
        entryPointReq->getTranslationUnit()->getModule(),
        entryPointName,
        entryPointProfile,
        compileRequest->getSink(),
        outFoundStructuralStage,
        &structuralInfo);
    if (!implementation)
        return nullptr;

    auto entryPoint = EntryPoint::create(linkage, implementation, entryPointProfile);
    entryPoint->setNameOverride(entryPointName);
    auto sourceTypeName =
        getStructuralRayTracingSourceTypeName(linkage->getASTBuilder(), structuralInfo.stageType);
    entryPoint->setEntryPointNameOverride(
        getStructuralRayTracingEntryPointName(sourceTypeName.getUnownedSlice()));
    entryPoint->setStructuralRayTracingInfo(structuralInfo);

    *outCapabilities = CapabilitySet{entryPoint->getInferredCapabilityRequirements()};
    if (structuralInfo.primitiveType)
    {
        // Consider a closest-hit context with `typealias Primitive = rt::CurvePrimitive`.
        // The primitive selects a Metal-only ABI even if `invoke` never reads its attributes.
        // Read the exact resolved associated type's declared requirement so the target validator
        // sees that constraint without rebuilding a primitive-to-capability table.
        auto primitiveType = as<DeclRefType>(structuralInfo.primitiveType->resolve());
        SLANG_RELEASE_ASSERT(primitiveType);
        auto primitiveDecl = primitiveType->getDeclRef().getDecl();
        SLANG_RELEASE_ASSERT(primitiveDecl->inferredCapabilityRequirements);
        outCapabilities->nonDestructiveJoin(primitiveDecl->inferredCapabilityRequirements);
    }
    return entryPoint;
}

// Classifies the descriptor as a host-bound opaque handle in ordinary type checking.
//
// Consider `rt::TraceProgramDescriptor<MySchema> program`. Its source declaration has no fields,
// but that does not make it an empty value that can be put in a constant buffer. Its intrinsic
// representation supplies resources at the shader boundary, so both type tags and handle
// placement checks must use the same classification. The intrinsic modifier survives ordinary
// source and serialized-module loading without a module-registration hook.
bool isStructuralRayTracingOpaqueHandleType(Type* type)
{
    return isIntrinsicTypeWithOp(type, kIROp_TraceProgramDescriptorType);
}

// Rejects source attempts to manufacture compiler-owned structural stage and operation metadata.
//
// Consider `__intrinsic_op(miss_stage_interface)` on an ordinary user declaration. That opcode does
// not implement a callable intrinsic; it records a stage contract already validated by the
// compiler. Accepting it directly would bypass those checks. Apply the same rule to named and
// numeric opcodes, while retaining the core module's existing permission to define intrinsics.
bool diagnoseInvalidStructuralRayTracingIntrinsicOp(
    IROp op,
    bool isCoreModule,
    UnownedStringSlice operationName,
    SourceLoc loc,
    DiagnosticSink* sink)
{
    if (isCoreModule)
        return false;

    const bool isStageInterface =
        op >= kIROp_FirstRaytracingStageInterface && op <= kIROp_LastRaytracingStageInterface;
    if (!isStageInterface && op != kIROp_StructuralRayTracingEntryPointInfoDecoration &&
        op != kIROp_StructuralRayTracingSourceOperationDecoration)
        return false;

    sink->diagnose(
        Diagnostics::CompilerOwnedIntrinsicOp{.operation = operationName, .location = loc});
    return true;
}

// Prevents a logical structural stage signature from reaching native entry-point ABI passes.
//
// Consider `-entry MyMiss` selecting a struct whose `invoke` takes `rt::MissInput<C>`.
// Linking preserves that logical signature and its structural metadata. Adapter synthesis must
// replace it with a native entry point before normal ABI legalization can interpret its inputs.
// This check reports any unconsumed marker at that boundary, including while adapter synthesis
// remains unimplemented in this frontend slice.
static bool _diagnoseUnloweredStructuralRayTracingEntryPoints(
    const List<IRFunc*>& entryPoints,
    DiagnosticSink* sink)
{
    bool found = false;
    for (auto entryPoint : entryPoints)
    {
        if (!entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
            continue;
        sink->diagnose(
            Diagnostics::UnloweredStructuralRayTracingEntryPoint{.entryPoint = entryPoint});
        found = true;
    }
    return found;
}

// Prevents reachable structural dispatch placeholders from silently becoming empty calls.
//
// Consider a ray-generation entry point calling `helper()`, which calls `tracer.trace(...)`.
// The library overload carries a source-operation marker until dispatch lowering consumes it.
// Follow the ordinary entry-point reference graph, including generic specializations, to reject
// the reachable placeholder. An imported overload that no entry point reaches is harmless.
static bool _diagnoseUnloweredStructuralRayTracingOperations(IRModule* module, DiagnosticSink* sink)
{
    bool found = false;
    Dictionary<IRInst*, HashSet<IRFunc*>> referencingEntryPoints;
    buildEntryPointReferenceGraph(referencingEntryPoints, module);
    for (const auto& [inst, entryPoints] : referencingEntryPoints)
    {
        auto func = as<IRFunc>(inst);
        if (!func || entryPoints.getCount() == 0)
            continue;
        auto marker = func->findDecoration<IRStructuralRayTracingSourceOperationDecoration>();
        if (!marker)
            continue;

        auto operationKindInst = as<IRIntLit>(marker->getOperationKind());
        SLANG_RELEASE_ASSERT(operationKindInst);
        auto operationKind = StructuralRayTracingSourceOperationKind(operationKindInst->getValue());
        const char* operationName = nullptr;
        switch (operationKind)
        {
        case StructuralRayTracingSourceOperationKind::TraceExplicitPayload:
            operationName = "trace with explicit payload";
            break;
        case StructuralRayTracingSourceOperationKind::TraceImplicitEmptyPayload:
            operationName = "trace with implicit empty payload";
            break;
        case StructuralRayTracingSourceOperationKind::CallShader:
            operationName = "callShader";
            break;
        default:
            SLANG_UNEXPECTED("invalid structural ray-tracing source operation marker");
        }

        // Diagnose the user's entry point rather than the marker's installed-library location.
        for (auto entryPoint : entryPoints)
        {
            sink->diagnose(Diagnostics::UnloweredStructuralRayTracingSourceOperation{
                .operationName = String(operationName),
                .entryPoint = entryPoint});
        }
        found = true;
    }
    return found;
}

// Requires descriptors to become target resources before ordinary parameter layout runs.
//
// Consider `uniform rt::TraceProgramDescriptor<MySchema> program` on an entry point. Its
// resources remain part of that entry point's interface even if the body does not read `program`.
// Concrete descriptor types are hoisted to module scope, so inspect those types rather than only
// their executable uses. Adapter lowering must consume their opaque source representation first.
static bool _diagnoseUnloweredTraceProgramDescriptors(IRModule* module, DiagnosticSink* sink)
{
    bool found = false;
    for (auto inst : module->getGlobalInsts())
    {
        if (!as<IRTraceProgramDescriptorType>(inst))
            continue;
        sink->diagnose(
            Diagnostics::UnloweredTraceProgramDescriptor{.location = findFirstUseLoc(inst)});
        found = true;
    }
    return found;
}

// Checks the structural representations that must be consumed before native ABI legalization.
//
// For example, compiling `-entry MyMiss` must not send its logical `MissInput<C>` parameter to
// the native shader-input legalizer. Keep the entry, operation, and descriptor boundary checks
// together so adding adapter synthesis gives all three a single, explicit insertion point.
SlangResult diagnoseUnloweredStructuralRayTracing(
    IRModule* module,
    const List<IRFunc*>& entryPoints,
    DiagnosticSink* sink)
{
    bool foundEntryPoint = _diagnoseUnloweredStructuralRayTracingEntryPoints(entryPoints, sink);
    bool foundOperation = _diagnoseUnloweredStructuralRayTracingOperations(module, sink);
    bool foundDescriptor = _diagnoseUnloweredTraceProgramDescriptors(module, sink);
    return foundEntryPoint || foundOperation || foundDescriptor ? SLANG_FAIL : SLANG_OK;
}

} // namespace Slang
