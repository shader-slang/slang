#include "slang-check-portable-ray-tracing.h"

#include "core/slang-string-util.h"
#include "slang-check-impl.h"
#include "slang-lookup.h"
#include "slang-mangle.h"
#include "slang-module.h"
#include "slang-session.h"
#include "slang-syntax.h"

namespace Slang
{
namespace PortableRT
{

// This file owns the portable ray-tracing rules and their declaration identities.
// Ordinary interfaces, associated-type constraints, overload resolution, and capability inference
// remain in the normal checker. Their call sites delegate here only for the additional rules:
// stage types and inputs, stage selection, schema entry uniqueness, and native entry signatures.
// Each rule below states its example and the representation or API contract it preserves.
//
// Schemas, lists, and hit groups are ordinary values. Empty payloads are ordinary payloads.
// Concrete stage structs are ordinary values too; input generic uses retain the restriction below.

// =============================================================================
// File-local helpers
// =============================================================================

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
static KnownBuiltinDeclName _getBuiltin(Decl* declaration)
{
    auto attribute = declaration ? declaration->findModifier<KnownBuiltinAttribute>() : nullptr;
    auto value = attribute ? as<ConstantIntVal>(attribute->name) : nullptr;
    return value ? KnownBuiltinDeclName(value->getValue()) : KnownBuiltinDeclName::COUNT;
}

static Stage _getDirectStageInputKind(Type* type)
{
    type = unwrapModifiedType(type);
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return getStageInputKind(typeDecl);
}

// Requires a concrete stage implementation to be a user-defined struct. Its invoke method is
// static, so ordinary fields and instances do not participate in stage dispatch. For example,
// `struct Miss : rt::IMissShader { uint state; ... }` is allowed: selecting Miss as a stage calls
// its static invoke function without constructing Miss or supplying a value for state.
static void _checkStageImplementationType(
    SemanticsVisitor* visitor,
    Type* stageType,
    SourceLoc conformanceLoc)
{
    SLANG_RELEASE_ASSERT(stageType);
    auto witnessedStageType = stageType;
    auto resolvedStageType = as<Type>(stageType->resolve());
    SLANG_RELEASE_ASSERT(resolvedStageType);

    auto stageDeclRef = isDeclRefTypeOf<AggTypeDecl>(resolvedStageType);
    const bool isCompilerBuiltinType =
        stageDeclRef && (stageDeclRef.getDecl()->findModifier<BuiltinTypeModifier>() ||
                         stageDeclRef.getDecl()->findModifier<MagicTypeModifier>());

    // A user-defined interface may refine a stage contract with additional requirements. It is
    // not itself a concrete stage implementation.
    if (stageDeclRef.as<InterfaceDecl>())
        return;

    // The RT API requires an ordinary struct even if another type satisfies the stage interface.
    // The StructDecl check rejects a class or an extension on a generic parameter T. It does not
    // reject `extension float : rt::IClosestHitShader`: float's built-in declaration is a struct.
    // The separate isCompilerBuiltinType check rejects float and other built-in types.
    // TODO: Allow a generic extension's target T and check the concrete types it applies to.
    // Simply skipping T here would leave those types unchecked by this rule.
    // Tracked in https://github.com/shader-slang/slang/issues/13618.
    if (!stageDeclRef.as<StructDecl>() || isCompilerBuiltinType)
    {
        const bool hasSourceAggregateDeclaration = stageDeclRef && !isCompilerBuiltinType;
        visitor->diagnoseOnce(Diagnostics::StructuralRayTracingStageImplementationMustBeStruct{
            .stageType = witnessedStageType,
            .location =
                hasSourceAggregateDeclaration ? stageDeclRef.getDecl()->loc : conformanceLoc});
    }
}

// Returns the section name for a structural schema requirement used in duplicate-entry diagnostics,
// or null for other associated-type requirements.
static const char* _getSchemaSectionName(AssocTypeDecl* requirement)
{
    switch (_getBuiltin(requirement))
    {
    case KnownBuiltinDeclName::RayTracingProgramHitGroups:
        return "hit-group";
    case KnownBuiltinDeclName::RayTracingProgramMissShaders:
        return "miss-shader";
    case KnownBuiltinDeclName::RayTracingProgramCallableShaders:
        return "callable-shader";
    default:
        break;
    }
    return nullptr;
}

static DeclRef<FuncDecl> _getStageImplementationFromSubtypeWitness(
    ASTBuilder* astBuilder,
    InterfaceDecl* stageInterface,
    SubtypeWitness* witness)
{
    witness = witness ? as<SubtypeWitness>(witness->resolve()) : nullptr;
    auto invokeRequirement = getStageInvokeRequirement(stageInterface);
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

// Only the library's stage interfaces and input views have restricted value uses. A user struct
// implementing a stage interface is an ordinary value; identifying it never requires inheritance.
enum class RestrictedTypeKind
{
    None,
    StageInterface,
    StageInput,
};

static RestrictedTypeKind _getDirectRestrictedTypeKind(Type* type)
{
    type = unwrapModifiedType(type);
    if (_getDirectStageInputKind(type) != Stage::Unknown)
        return RestrictedTypeKind::StageInput;
    if (as<RayTracingStageInterfaceType>(type))
        return RestrictedTypeKind::StageInterface;
    return RestrictedTypeKind::None;
}

// Checks the supplied type and built-in containers without inspecting ordinary struct fields.
// Consider `struct Box<T> { T value; }`: the field is checked when Box is checked, and writing
// Box<MissInput<C>> is rejected by the generic-argument check. Inspecting every ordinary struct
// here would needlessly force unfinished field declarations, e.g. Node's own `Ptr<Node> next`.
//
// Arrays and pointers can name an input without a generic application in source, so follow their
// already-checked element types. Generic fields such as T.Item need validation after
// specialization, just like T.Item() construction; that limitation is tracked in
// shader-slang/slang#13686.
static RestrictedTypeKind _getRestrictedTypeKind(Type* type)
{
    type = unwrapModifiedType(type);
    auto kind = _getDirectRestrictedTypeKind(type);
    if (kind != RestrictedTypeKind::None)
        return kind;

    unwrapTypeElements(
        type,
        [&](Type* elementType)
        {
            kind = _getRestrictedTypeKind(elementType);
            return kind != RestrictedTypeKind::None;
        });
    return kind;
}

// Reports an invalid use of an RT builtin type: a stage input or the stage interface itself.
// "RT builtin" means a compiler-recognized library type, not necessarily a C++ BuiltinType.
// Concrete structs implementing the interface remain ordinary types. Returns true when an error
// is reported, or false when the supplied type needs no diagnostic.
static bool _diagnoseInvalidBuiltinTypeUse(DiagnosticSink* sink, Type* type, SourceLoc location)
{
    switch (_getRestrictedTypeKind(type))
    {
    case RestrictedTypeKind::StageInterface:
        // The interface itself, e.g. IMissShader, cannot be a runtime value. A concrete struct
        // implementing IMissShader is an ordinary type and does not enter this branch.
        sink->diagnose(
            Diagnostics::StructuralRayTracingStageRuntimeValue{.type = type, .location = location});
        return true;
    case RestrictedTypeKind::StageInput:
        // Allowed input parameters and local copies return from the variable checker before
        // reaching this helper. Here the context forbids an input, even in an array or pointer.
        sink->diagnose(
            Diagnostics::StructuralRayTracingInputStorage{.type = type, .location = location});
        return true;
    default:
        // Ordinary types need no diagnostic from this check.
        return false;
    }
}

// Checks one explicit or inferred generic type argument. Consider `consume(input)` with
// consume<T>(T value): its checked substitution names MissInput<C> even without written <...>.
// A variadic call groups these same arguments into a ConcreteTypePack, so inspect its elements.
// No inheritance, fields, or conformance witnesses are needed to recognize these builtin types.
static bool _diagnoseRestrictedGenericArgument(DiagnosticSink* sink, Type* type, SourceLoc location)
{
    if (auto typePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < typePack->getTypeCount(); ++i)
        {
            if (_diagnoseRestrictedGenericArgument(sink, typePack->getElementType(i), location))
                return true;
        }
        return false;
    }
    switch (_getDirectRestrictedTypeKind(type))
    {
    case RestrictedTypeKind::StageInput:
        sink->diagnose(Diagnostics::StructuralRayTracingInputGenericArgument{
            .type = type,
            .location = location});
        return true;
    case RestrictedTypeKind::StageInterface:
        return _diagnoseInvalidBuiltinTypeUse(sink, type, location);
    default:
        return false;
    }
}

// =============================================================================
// Shared functions used by other compiler files
// =============================================================================

// Allows the ray-tracing library to select its stage-interface and descriptor AST types.
// For example, `__magic_type(MissShaderType) interface IMissShader` and
// `__magic_type(TraceProgramDescriptorType) struct TraceProgramDescriptor<Schema>` both store
// their checked declaration reference. Existing type construction can therefore preserve their
// generic arguments without a custom representation. Other core magic classes may require
// different operands and cannot safely be applied to an ordinary declaration.
bool isBuiltinTypeModifier(MagicTypeModifier* modifier, Decl* decl)
{
    if (!modifier || !modifier->magicNodeType || !modifier->magicNodeType.getInfo()->createFunc)
        return false;
    return (as<InterfaceDecl>(decl) &&
            modifier->magicNodeType.isSubClassOf<RayTracingStageInterfaceType>()) ||
           (as<StructDecl>(decl) &&
            modifier->magicNodeType.isSubClassOf<TraceProgramDescriptorType>());
}

String getSourceTypeName(ASTBuilder* astBuilder, Type* stageType)
{
    auto declRefType = as<DeclRefType>(stageType ? stageType->resolve() : nullptr);
    if (!declRefType)
        return String();

    if (SubstitutionSet(declRefType->getDeclRef()).findGenericAppDeclRef())
    {
        // `GenericMiss<uint>` and `GenericMiss<float>` share one declaration path. Ordinary type
        // mangling preserves the selected arguments and provides a distinct name for each type.
        return getMangledTypeName(astBuilder, stageType->getCanonicalType());
    }

    // Ordinary declaration-reference printing gives names such as `Stages.Miss`, omitting modules.
    return declRefType->toString();
}

String getEntryPointName(UnownedStringSlice sourceTypeName, Stage stage)
{
    SLANG_RELEASE_ASSERT(isRaytracingStage(stage));

    // Keep qualified names readable in generated code: `Stages.Miss` becomes `Stages_Miss_miss`.
    // Include the selected stage so a struct implementing closest-hit and any-hit gets distinct
    // names for those two entry points.
    StringBuilder result;
    result << StringUtil::calcCharReplaced(sourceTypeName, '.', '_');
    result << "_" << getStageName(stage);
    return result.produceString();
}

Stage getStageKind(InterfaceDecl* interfaceDecl)
{
    switch (_getMagicTypeClass(interfaceDecl))
    {
    case ASTNodeType::ClosestHitShaderType:
        return Stage::ClosestHit;
    case ASTNodeType::AnyHitShaderType:
        return Stage::AnyHit;
    case ASTNodeType::IntersectionShaderType:
        return Stage::Intersection;
    case ASTNodeType::MissShaderType:
        return Stage::Miss;
    case ASTNodeType::CallableShaderType:
        return Stage::Callable;
    default:
        return Stage::Unknown;
    }
}

bool isExecutableStageInterface(InterfaceDecl* interfaceDecl)
{
    auto modifier = interfaceDecl ? interfaceDecl->findModifier<MagicTypeModifier>() : nullptr;
    return modifier && modifier->magicNodeType &&
           modifier->magicNodeType.isSubClassOf<RayTracingStageInterfaceType>();
}

Stage getStageInputKind(AggTypeDecl* typeDecl)
{
    switch (_getBuiltin(typeDecl))
    {
    case KnownBuiltinDeclName::RayTracingClosestHitInput:
        return Stage::ClosestHit;
    case KnownBuiltinDeclName::RayTracingAnyHitInput:
        return Stage::AnyHit;
    case KnownBuiltinDeclName::RayTracingIntersectionInput:
        return Stage::Intersection;
    case KnownBuiltinDeclName::RayTracingMissInput:
        return Stage::Miss;
    case KnownBuiltinDeclName::RayTracingCallableInput:
        return Stage::Callable;
    default:
        return Stage::Unknown;
    }
}

FunctionDeclBase* getStageInvokeRequirement(InterfaceDecl* interfaceDecl)
{
    if (!isExecutableStageInterface(interfaceDecl))
        return nullptr;
    for (auto member : interfaceDecl->getDirectMemberDeclsOfType<FunctionDeclBase>())
    {
        if (member->getName() && member->getName()->text == "invoke")
            return member;
    }
    return nullptr;
}

// Returns the checked entry types used by the duplicate-entry diagnostic. The list's conformance
// witnesses are handled by ordinary conformance checking and are not needed to compare types.
static ConcreteTypePack* _getEntryTypes(ASTBuilder* astBuilder, Type* entryListType)
{
    if (auto declRefType = as<DeclRefType>(entryListType))
    {
        if (auto genericApp = SubstitutionSet(declRefType->getDeclRef()).findGenericAppDeclRef())
        {
            // Select the type pack by its semantic kind, independently of the positions of the
            // generic arguments. An empty list already has a zero-length type pack.
            for (auto argument : genericApp->getArgs())
            {
                if (auto typePack = as<ConcreteTypePack>(argument->resolve()))
                    return typePack;
            }
        }
    }
    // The list itself may still be a type parameter. Consider this example:
    //
    //     struct Schema<Misses : rt::IMissShaderList> : rt::ITraceProgramSchema
    //     {
    //         // Other requirements omitted.
    //         typealias MissShaders = Misses;
    //     }
    //
    // While checking Schema, we cannot enumerate Misses. Return an empty pack so the caller's
    // duplicate-entry loop has nothing to visit. This does not mean Misses is an empty list.
    // A list such as MissShaderList<T> already has an enumerable pack, even if T is generic.
    // TODO: Check duplicates after specialization too. This fallback does not schedule a later
    // check; Schema<MissShaderList<M, M>> currently passes this declaration-time restriction.
    return astBuilder->getTypePack(ArrayView<Type*>());
}

// Checks the selected native entry signature, including aliases and closed generic arguments.
// A helper may accept MissInput<C>, but selecting that helper with -entry requires this check.
// Source imports and saved modules use the same checked function reference. Types revealed only
// by later, separate specialization still need validation at that specialization boundary.
void diagnoseInputInNativeEntryPoint(EntryPoint* entryPoint, DiagnosticSink* sink)
{
    SLANG_RELEASE_ASSERT(!entryPoint->isStructuralRayTracingEntryPoint());
    auto diagnoseInputType = [sink](Type* type, SourceLoc location)
    {
        if (_getRestrictedTypeKind(type) == RestrictedTypeKind::StageInput)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingInputInNativeEntryPoint{
                .type = type,
                .location = location});
        }
    };

    auto astBuilder = entryPoint->getLinkage()->getASTBuilder();
    auto function = entryPoint->getFuncDeclRef();
    for (auto parameter : getParameters(astBuilder, function))
    {
        auto parameterDecl = parameter.getDecl();
        auto location = parameterDecl->type.exp ? parameterDecl->type.exp->loc : parameterDecl->loc;
        diagnoseInputType(getType(astBuilder, parameter), location);
    }
    auto resultTypeExpr = function.getDecl()->returnType.exp;
    diagnoseInputType(
        getResultType(astBuilder, function),
        resultTypeExpr ? resultTypeExpr->loc : function.getDecl()->loc);
}

// Selects one executable conformance for a source struct named by -entry Stage or Stage.invoke.
// For example, a struct implementing both IMissShader and ICallableShader needs `-stage miss`
// or `-stage callable`; a struct implementing only IMissShader can omit the explicit stage.
// The selected witness supplies the exact invoke implementation and preserves dependent types.
// Generic stages currently need arguments in the entry name, for example `-entry "M<P>"`.
// TODO(#13800): Support separate CLI/API specialization while keeping invoke, its stage contract,
// and target validation consistent with the concrete arguments.
DeclRef<FuncDecl> findEntryPointByName(
    Linkage* linkage,
    Module* module,
    Name* name,
    Profile& ioProfile,
    DiagnosticSink* sink,
    bool* outFoundStruct,
    Type** outStageType)
{
    // Both `Stage` and `Stage.invoke` select the stage's checked conformance. Read the receiver
    // type from member lookup so `GenericStage<Payload>.invoke` keeps its generic arguments.
    // The selected conformance, rather than ordinary overload resolution, chooses the method
    // when one struct implements several stages.
    *outFoundStruct = false;
    *outStageType = nullptr;
    SharedSemanticsContext sharedContext(linkage, module, sink);
    SemanticsVisitor visitor(&sharedContext);
    auto expr = module->findDeclFromString(getText(name), sink);
    auto declRefExpr = as<DeclRefExpr>(expr);
    auto stageTypeDeclRef =
        declRefExpr ? declRefExpr->declRef.as<AggTypeDecl>() : DeclRef<AggTypeDecl>();
    bool selectedInvoke = false;
    if (!stageTypeDeclRef)
    {
        auto overloadedExpr = as<OverloadedExpr>(expr);
        auto memberName = declRefExpr      ? declRefExpr->declRef.getName()
                          : overloadedExpr ? overloadedExpr->lookupResult2.getName()
                                           : nullptr;
        if (getText(memberName) != "invoke")
            return DeclRef<FuncDecl>();
        auto base = visitor.GetBaseExpr(expr);
        auto typeType = base ? as<TypeType>(base->type) : nullptr;
        if (!typeType)
            return DeclRef<FuncDecl>();
        auto stageType = typeType->getType();
        if (declRefExpr)
        {
            // With a default implementation, `Stage.invoke` can reference the interface's
            // requirement. Its This witness retains Stage even though the base is the interface.
            if (auto thisWitness = findThisTypeWitness(
                    SubstitutionSet(declRefExpr->declRef),
                    isDeclRefTypeOf<InterfaceDecl>(stageType).getDecl()))
                stageType = thisWitness->getSub();
        }
        stageTypeDeclRef = isDeclRefTypeOf<AggTypeDecl>(stageType);
        selectedInvoke = true;
    }
    if (!stageTypeDeclRef || getModule(stageTypeDeclRef.getDecl()) != module)
        return DeclRef<FuncDecl>();

    *outFoundStruct = !selectedInvoke;
    for (auto dependency : module->getModuleDependencies())
    {
        auto moduleDecl = dependency->getModuleDecl();
        if (sharedContext.importedModulesSet.add(moduleDecl))
            sharedContext.importedModulesList.add(moduleDecl);
    }
    visitor.ensureDecl(stageTypeDeclRef, DeclCheckState::ReadyForConformances);

    DeclRef<FuncDecl> stageImplementations[SLANG_STAGE_COUNT] = {};
    bool ambiguousImplementations[SLANG_STAGE_COUNT] = {};
    auto stageType = DeclRefType::create(linkage->getASTBuilder(), stageTypeDeclRef);
    for (auto facet : visitor.getShared()->getInheritanceInfo(stageType).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = getStageKind(interfaceDeclRef.getDecl());
        if (kind != Stage::Unknown)
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
            }
        }
    }

    Count implementedStageCount = 0;
    Stage onlyImplementedStage = Stage::Unknown;
    for (UInt i = 0; i < SLANG_STAGE_COUNT; ++i)
    {
        if (stageImplementations[i])
        {
            ++implementedStageCount;
            onlyImplementedStage = Stage(i);
        }
    }

    if (implementedStageCount == 0)
    {
        // An ordinary struct may also have a static method named invoke.
        if (selectedInvoke)
            return DeclRef<FuncDecl>();
        sink->diagnose(Diagnostics::StructuralRayTracingEntryPointNotStage{
            .stageType = stageTypeDeclRef.getDecl()});
        return DeclRef<FuncDecl>();
    }
    *outFoundStruct = true;

    auto requestedStage = ioProfile.getStage();
    auto selectedStage = requestedStage;
    if (requestedStage != Stage::Unknown)
    {
        if (UInt(selectedStage) >= SLANG_STAGE_COUNT || !stageImplementations[int(selectedStage)])
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
        ioProfile = Profile(selectedStage);
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
            .stage = getStageName(selectedStage),
            .stageType = stageTypeDeclRef.getDecl()});
        return DeclRef<FuncDecl>();
    }

    auto invokeMethod = stageImplementations[int(selectedStage)];
    if (!invokeMethod)
    {
        sink->diagnose(Diagnostics::InternalCompilerError{.location = stageTypeDeclRef.getLoc()});
        return DeclRef<FuncDecl>();
    }

    *outStageType = stageType;
    return invokeMethod;
}

// Sets the names of an entry point selected through a checked stage conformance.
// Preserve the requested public name, such as Stages.Miss or Stages.Miss.invoke. Both spellings
// derive the same target symbol from the stage type and select the same checked invoke method.
void initializeEntryPoint(EntryPoint* entryPoint, Name* entryPointName, Type* stageType)
{
    entryPoint->markAsStructuralRayTracingEntryPoint();
    entryPoint->setNameOverride(entryPointName);
    auto sourceTypeName = getSourceTypeName(entryPoint->getLinkage()->getASTBuilder(), stageType);
    entryPoint->setEntryPointNameOverride(
        getEntryPointName(sourceTypeName.getUnownedSlice(), entryPoint->getStage()));
}

// =============================================================================
// Checks called by ordinary semantic checking
// =============================================================================

// Checks the extra RT API restrictions after ordinary interface conformance checking succeeds.
// For example, `struct Miss : rt::IMissShader` must be a user-defined struct.
// Ordinary conformance checking has already verified its required members, such as Context and
// invoke. Applying these restrictions at that point also covers a Miss listed in a schema
// without being selected directly with `-entry Miss`.
// A conformance to an ordinary interface, such as ITraceProgramSchema, needs no stage restrictions.
void checkStageRestrictions(
    SemanticsVisitor* visitor,
    DeclRef<InterfaceDecl> superInterfaceDeclRef,
    WitnessTable* witnessTable,
    SourceLoc conformanceLoc)
{
    auto stageKind = getStageKind(superInterfaceDeclRef.getDecl());
    if (stageKind == Stage::Unknown || !witnessTable)
        return;

    _checkStageImplementationType(visitor, witnessTable->witnessedType, conformanceLoc);
}

void diagnoseDuplicateSchemaEntries(
    ASTBuilder* astBuilder,
    DiagnosticSink* sink,
    Type* entryListType,
    AssocTypeDecl* associatedTypeRequirement,
    Type* schemaType,
    SourceLoc satisfyingDeclLoc)
{

    auto sectionName = _getSchemaSectionName(associatedTypeRequirement);
    if (!sectionName)
        return;

    SLANG_RELEASE_ASSERT(entryListType && schemaType);

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
    auto entries = _getEntryTypes(astBuilder, entryListType->getCanonicalType());
    HashSet<Type*> seenEntries;
    for (Index i = 0; i < entries->getTypeCount(); ++i)
    {
        auto entryType = entries->getElementType(i)->getCanonicalType();
        if (seenEntries.add(entryType))
            continue;

        sink->diagnose(Diagnostics::DuplicateStructuralRayTracingEntry{
            .section = sectionName,
            .entry = entryType,
            .schema = schemaType->getCanonicalType(),
            .location = satisfyingDeclLoc});
    }
}

// Checks the declaration's own type, direction, scope, and initializer. Consider:
//
//     void helper(in rt::MissInput<C> input)
//     {
//         let copy = input;          // Refers to the existing invocation's input.
//         rt::MissInput<C> fresh;    // Has no existing input to refer to.
//     }
//
// A direct input is allowed as a non-uniform in parameter or an initialized non-static local.
// Globals, fields, and other parameter directions are rejected. An initializer must itself pass
// the ordinary expression/conversion checks below, which reject fresh input construction.
void diagnoseInvalidVariableType(SemanticsVisitor* visitor, VarDeclBase* varDecl)
{
    auto type = varDecl->getType();
    // Recognize the input type itself, such as MissInput<C>. Unknown means it is not a stage input.
    if (_getDirectStageInputKind(type) != Stage::Unknown)
    {
        if (auto param = as<ParamDecl>(varDecl))
        {
            // checkCallableDeclCommon also calls this helper for function parameters. An in
            // parameter receives an existing input from its caller; uniform would instead request
            // a host binding. Uniform parameters and other directions reach the rejection below.
            if (getExplicitlyDeclaredParamPassingMode(param) == ParamPassingMode::In &&
                !param->hasModifier<HLSLUniformModifier>())
                return;
        }
        else if (
            isLocalVar(varDecl) && !varDecl->hasModifier<HLSLStaticModifier>() && varDecl->initExpr)
        {
            // Only an initialized, non-static local can copy an input. Having an initializer is
            // not enough: `MissInput<C> copy = input` is allowed, but `MissInput<C> fresh = {}`
            // creates a new input and is rejected by the expression check.
            //
            // For `MissInput<C> fresh;`, variable checking may insert a default-constructor call.
            // Check that inserted expression too, even though no initializer was written.
            diagnoseInvalidExpression(visitor, varDecl->initExpr);
            // The local declaration is allowed; any initializer error was reported above.
            return;
        }
    }
    // Reject input declarations that were not accepted above: globals, fields, static locals,
    // disallowed parameters, and `MissInput<C> fresh;` when no initializer was inserted. This also
    // checks restricted types inside built-in containers and the stage interface itself.
    // Reaching this call does not imply an error: ordinary types, including concrete stage
    // structs, pass without a diagnostic.
    _diagnoseInvalidBuiltinTypeUse(visitor->getSink(), type, varDecl->loc);
}

// Rejects results such as `rt::MissInput<C> makeInput();`. Inputs may be received as in parameters
// and copied locally, but a function cannot promise a new input. Constructors are checked at
// their call expressions instead, so their declarations do not trigger this result check.
void diagnoseInvalidReturnType(DiagnosticSink* sink, CallableDecl* decl)
{
    if (as<ConstructorDecl>(decl))
        return;
    _diagnoseInvalidBuiltinTypeUse(
        sink,
        decl->returnType.type,
        decl->returnType.exp ? decl->returnType.exp->loc : decl->loc);
}

// Rejects a property such as `property rt::MissInput<C> input { get; }`. Payload and record
// properties have ordinary application-defined result types and remain legal.
void diagnoseInvalidPropertyType(DiagnosticSink* sink, PropertyDecl* propertyDecl)
{
    _diagnoseInvalidBuiltinTypeUse(
        sink,
        propertyDecl->type.type,
        propertyDecl->type.exp ? propertyDecl->type.exp->loc : propertyDecl->loc);
}

// Checks actual construction and call results after normal expression checking. Consider:
//
//     use(rt::MissInput<C>(input)); // Same-type coercion returns input itself: allowed.
//     use(rt::MissInput<C>());      // A constructor creates a fresh view: rejected.
//     use({});                     // Initializing the argument creates a fresh view: rejected.
//
// ResolveInvoke and coercion distinguish these cases. Inspect their checked result, rather than
// treating every Type(...) spelling as construction. Ordinary struct results need no RT check.
bool diagnoseInvalidExpression(SemanticsVisitor* visitor, Expr* expr)
{
    auto type = expr->type;
    if (_getRestrictedTypeKind(type) == RestrictedTypeKind::None)
        return false;

    bool isConstruction = false;
    auto location = expr->loc;
    if (auto invoke = as<InvokeExpr>(expr))
    {
        if (auto callee = as<DeclRefExpr>(invoke->functionExpr))
        {
            if (as<ConstructorDecl>(callee->declRef.getDecl()))
            {
                isConstruction = true;
                location = invoke->functionExpr->loc;
            }
        }
    }
    else if (as<InitializerListExpr>(expr) || as<DefaultConstructExpr>(expr))
    {
        isConstruction = true;
    }
    else
    {
        return false;
    }
    if (isConstruction)
    {
        // Coercion can check the same constructor after call checking. Use ordinary suppression
        // so that one forbidden construction produces one diagnostic.
        visitor->diagnoseOnce(
            Diagnostics::StructuralRayTracingTypeConstruction{.type = type, .location = location});
        return true;
    }
    return _diagnoseInvalidBuiltinTypeUse(visitor->getSink(), type, location);
}

// Checks the type arguments of a resolved function, method, or constructor call. Consider:
//
//     void consume<T>(T value) { T fresh = T(); }
//     void helper(in rt::MissInput<C> input)
//     {
//         consume<rt::MissInput<C>>(input);
//         consume(input);
//     }
//
// The chosen declaration's substitutions include both written and inferred arguments. Rejecting
// Input for T prevents the generic body from constructing it after specialization. This temporary
// restriction does not catch an associated type such as T.Item becoming Input; see issue #13686.
bool diagnoseInvalidGenericArguments(DiagnosticSink* sink, InvokeExpr* invoke)
{
    auto functionDeclRef = as<DeclRefExpr>(invoke->functionExpr);
    if (!functionDeclRef)
        return false;

    bool diagnosed = false;
    SubstitutionSet(functionDeclRef->declRef)
        .forEachGenericSubstitution(
            [&](GenericDecl*, Val::OperandView<Val> arguments)
            {
                if (diagnosed)
                    return;
                for (auto argument : arguments)
                {
                    // Integer arguments and conformance witnesses need no type-use check.
                    if (auto argumentType = as<Type>(argument))
                    {
                        if (_diagnoseRestrictedGenericArgument(
                                sink,
                                argumentType,
                                invoke->functionExpr->loc))
                        {
                            diagnosed = true;
                            return;
                        }
                    }
                }
            });
    return diagnosed;
}

// Checks a generic type application even when nothing is called. Consider:
//
//     struct Factory<T> { static void make() { T fresh = T(); } }
//     typealias InputFactory = Factory<rt::MissInput<C>>;
//
// This has the same unknown-T construction risk as a generic call. Check the written type
// arguments once normal type resolution succeeds. A concrete stage struct is an ordinary
// argument, so shader lists and generic helpers require no special exemption for those structs.
bool diagnoseInvalidGenericTypeApplication(DiagnosticSink* sink, GenericAppExpr* genericApplication)
{
    for (auto argument : genericApplication->arguments)
    {
        if (auto argumentType = as<TypeType>(argument->type))
        {
            if (_diagnoseRestrictedGenericArgument(sink, argumentType->getType(), argument->loc))
                return true;
        }
    }
    return false;
}

} // namespace PortableRT
} // namespace Slang
