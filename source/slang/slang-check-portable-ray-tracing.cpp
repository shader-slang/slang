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
// Concrete stage structs are ordinary values too. Generic type arguments follow ordinary rules.

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

// Defined below, after getStageInputKind, which the module scan reads.
static bool _compilationCanDeclareStageInputs(Linkage* linkage);

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

// Checks the supplied type and built-in containers, plus the substituted fields of a specialized
// generic struct. Consider `struct Box<T> { T value; }`: its field is checked while its type is
// still T. A later use that supplies the concrete argument, such as a `Box<Carrier>` parameter,
// reaches this classifier carrying the generic application, and each field's substituted type is
// available through its member declaration reference — so following those fields restores the
// storage rule without a separate pending-use list. Fields of non-generic structs were already
// rejected at their own declaration and are not walked again; fields still written in terms of T
// substitute to types the declaration-time checks already covered, and likewise produce nothing
// here.
//
// Arrays and pointers expose already-checked element types, so follow those types here too.
// A visited set stops recursive struct types such as `struct Node { Ptr<Node> next; }` from
// walking into themselves forever, and a field whose type is not checked yet — for example while
// the struct's own declaration is still being formed — is skipped rather than treated as
// ordinary; the declaration-time checks cover it once complete.
// Resolves a substituted type to the concrete type it names, so the classifier can read the
// declaration. Two indirections are ordinary for a field that was written in terms of a generic
// parameter:
// 1. An associated type such as `T.Item` with `T = Carrier` substitutes to a lookup through the
//    conformance witness. Resolving it reads the conformance's witness table, which exists once
//    the declaring type reaches ReadyForConformances; the use site can be checked before that
//    state, so advance it on demand here.
// 2. The concrete member may be a typealias (Carrier.Item is one), so follow `typealias`
//    declarations to the type they name, the same as an ordinary source type.
// Each step can expose another lookup (for example an alias to a nested associated type), so
// the loop applies the same treatment to each result.
static Type* _resolveSubstitutedFieldType(SemanticsVisitor* visitor, Type* type)
{
    for (Index step = 0; type && step < 4; ++step)
    {
        LookupDeclRef* lookup = nullptr;
        if (auto declRefType = as<DeclRefType>(type))
        {
            DeclRefBase* declRefBase = declRefType->getDeclRef();
            lookup = as<LookupDeclRef>(declRefBase);
        }
        if (lookup)
        {
            auto witness = as<SubtypeWitness>(lookup->getWitness());
            DeclRef<AggTypeDeclBase> subDeclRefType;
            if (witness)
                subDeclRefType = isDeclRefTypeOf<AggTypeDeclBase>(witness->getSub());
            if (!subDeclRefType)
                break;
            visitor->ensureDecl(subDeclRefType.getDecl(), DeclCheckState::ReadyForConformances);
        }
        type = as<Type>(type->resolve());
        if (!type)
            return nullptr;
        if (auto declRefType = as<DeclRefType>(type))
        {
            if (auto typeDefDeclRef = declRefType->getDeclRef().as<TypeDefDecl>())
            {
                type = getType(type->getASTBuilder(), typeDefDeclRef);
                continue;
            }
        }
        break;
    }
    return type;
}

static RestrictedTypeKind _getRestrictedTypeKindImpl(
    SemanticsVisitor* visitor,
    Type* type,
    HashSet<Type*>& visited)
{
    type = unwrapModifiedType(type);
    auto kind = _getDirectRestrictedTypeKind(type);
    if (kind != RestrictedTypeKind::None)
        return kind;

    if (!visited.add(type))
        return RestrictedTypeKind::None;

    unwrapTypeElements(
        type,
        [&](Type* elementType)
        {
            kind = _getRestrictedTypeKindImpl(visitor, elementType, visited);
            return kind != RestrictedTypeKind::None;
        });
    if (kind != RestrictedTypeKind::None)
        return kind;

    auto declRefType = as<DeclRefType>(type);
    auto structDeclRef =
        declRefType ? declRefType->getDeclRef().as<StructDecl>() : DeclRef<StructDecl>();
    if (!structDeclRef || !SubstitutionSet(structDeclRef).findGenericAppDeclRef())
        return RestrictedTypeKind::None;

    auto astBuilder = type->getASTBuilder();
    for (auto fieldDeclRef : getFields(astBuilder, structDeclRef, MemberFilterStyle::All))
    {
        auto fieldType = _resolveSubstitutedFieldType(visitor, getType(astBuilder, fieldDeclRef));
        if (!fieldType)
            continue;
        kind = _getRestrictedTypeKindImpl(visitor, fieldType, visited);
        if (kind != RestrictedTypeKind::None)
            return kind;
    }
    return RestrictedTypeKind::None;
}

static RestrictedTypeKind _getRestrictedTypeKind(SemanticsVisitor* visitor, Type* type)
{
    HashSet<Type*> visited;
    return _getRestrictedTypeKindImpl(visitor, type, visited);
}

// Reports an invalid use of an RT builtin type: a stage input or the stage interface itself.
// "RT builtin" means a compiler-recognized library type, not necessarily a C++ BuiltinType.
// Concrete structs implementing the interface remain ordinary types. Returns true when an error
// is reported, or false when the supplied type needs no diagnostic.
static bool _diagnoseInvalidBuiltinTypeUse(
    SemanticsVisitor* visitor,
    DiagnosticSink* sink,
    Type* type,
    SourceLoc location)
{
    switch (_getRestrictedTypeKind(visitor, type))
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
    SharedSemanticsContext sharedContext(entryPoint->getLinkage(), entryPoint->getModule(), sink);
    SemanticsVisitor visitor(&sharedContext);
    if (!_compilationCanDeclareStageInputs(entryPoint->getLinkage()))
        return;
    auto diagnoseInputType = [&visitor, sink](Type* type, SourceLoc location)
    {
        if (_getRestrictedTypeKind(&visitor, type) == RestrictedTypeKind::StageInput)
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
// Globals, fields, and other parameter directions are rejected. The input types have private
// constructors, so ordinary constructor access checking rejects direct construction.
void diagnoseInvalidVariableType(
    SemanticsVisitor* visitor,
    DiagnosticSink* sink,
    VarDeclBase* varDecl)
{
    auto linkage = visitor->getShared()->m_linkage;
    if (!linkage || !_compilationCanDeclareStageInputs(linkage))
        return;
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
            return;
        }
    }
    // Reject input declarations that were not accepted above: globals, fields, static locals,
    // disallowed parameters, and uninitialized locals such as `MissInput<C> fresh;`. This also
    // checks restricted types inside built-in containers and the stage interface itself.
    // Reaching this call does not imply an error: ordinary types, including concrete stage
    // structs, pass without a diagnostic.
    _diagnoseInvalidBuiltinTypeUse(visitor, sink, type, varDecl->loc);
}

// Rejects results such as `rt::MissInput<C> makeInput();`. Inputs may be received as in parameters
// and copied locally, but a function cannot promise a new input. Constructors do not have
// a separately declared return type, so they are not subject to this check.
void diagnoseInvalidReturnType(SemanticsVisitor* visitor, DiagnosticSink* sink, CallableDecl* decl)
{
    auto linkage = visitor->getShared()->m_linkage;
    if (!linkage || !_compilationCanDeclareStageInputs(linkage))
        return;
    if (as<ConstructorDecl>(decl))
        return;
    _diagnoseInvalidBuiltinTypeUse(
        visitor,
        sink,
        decl->returnType.type,
        decl->returnType.exp ? decl->returnType.exp->loc : decl->loc);
}

// Rejects a property such as `property rt::MissInput<C> input { get; }`. Payload and record
// properties have ordinary application-defined result types and remain legal.
void diagnoseInvalidPropertyType(
    SemanticsVisitor* visitor,
    DiagnosticSink* sink,
    PropertyDecl* propertyDecl)
{
    auto linkage = visitor->getShared()->m_linkage;
    if (!linkage || !_compilationCanDeclareStageInputs(linkage))
        return;
    _diagnoseInvalidBuiltinTypeUse(
        visitor,
        sink,
        propertyDecl->type.type,
        propertyDecl->type.exp ? propertyDecl->type.exp->loc : propertyDecl->loc);
}

// =============================================================================
// Checks after a generic's parameters become concrete
// =============================================================================

// A generic body is checked once while its parameters are still abstract, so a forbidden use the
// concrete instantiation would expose cannot be diagnosed at the declaration. Consider:
//
//     uint manufacture<T : ICarrier>()
//     {
//         T.Item local = T.Item();  // Checked with T abstract; MissInput<C> once T = Carrier.
//     }
//
// The walker below re-reads that already-checked body with one instantiation's substitutions
// applied, and reports the uses the substitution turned into stage-input uses. It reuses the
// checked expressions' types (their substitution is the same machinery getType applies to
// member declaration references), and does not rebuild any syntax. Declarations and expressions
// whose types the substitution leaves untouched are skipped: the declaration-time checks already
// covered them concretely, so each reported use is one the abstraction introduced.
class _SpecializedCallableBodyRTChecker
    : public SemanticsDeclReferenceVisitor<_SpecializedCallableBodyRTChecker>
{
    typedef SemanticsDeclReferenceVisitor<_SpecializedCallableBodyRTChecker> Base;

public:
    _SpecializedCallableBodyRTChecker(SemanticsContext const& outer)
        : Base(outer)
    {
    }

    SubstitutionSet subst;
    SourceLoc instantiationLoc;
    String calleeName;

    void processReferencedDecl(Decl*) override {}
    void processDeclModifiers(Decl*, SourceLoc) override {}

    void visitVarDeclBase(VarDeclBase* varDecl)
    {
        _checkVarDecl(varDecl);
        Base::visitVarDeclBase(varDecl);
    }

    void visitDeclGroup(DeclGroup* group)
    {
        for (auto decl : group->decls)
            dispatchIfNotNull(decl);
    }

    void visitInvokeExpr(InvokeExpr* expr)
    {
        // A construction resolves to a constructor invocation, including the synthesized
        // default constructor the abstract body saw for `T()` and `T.Item()`.
        auto declRefExpr = as<DeclRefExpr>(expr->functionExpr);
        if (declRefExpr && as<ConstructorDecl>(declRefExpr->declRef.getDecl()))
            _checkConstructionExpr(expr);
        Base::visitInvokeExpr(expr);
    }

    void visitDefaultConstructExpr(DefaultConstructExpr* expr) { _checkConstructionExpr(expr); }

    void visitInitializerListExpr(InitializerListExpr* expr)
    {
        // `T t = {}` constructs through the constructor, exactly as the direct spelling of
        // `MissInput<C> t = {}` does, so the braced form is a construction too.
        _checkConstructionExpr(expr);
        Base::visitInitializerListExpr(expr);
    }

    void visitGenericAppDeclRef(GenericAppDeclRef* genericAppDeclRef)
    {
        if (!visitedVals.add(genericAppDeclRef))
            return;
        _checkInnerSpecialization(genericAppDeclRef);
        // Operand zero holds the inner declaration itself, which is an AST-node operand rather
        // than a Val; follow the same kind check the base's visitVal applies.
        for (Index i = 0; i < genericAppDeclRef->getOperandCount(); i++)
        {
            if (genericAppDeclRef->m_operands[i].kind == ValNodeOperandKind::ValNode)
                dispatchIfNotNull(genericAppDeclRef->getOperand(i));
        }
    }

private:
    // Returns the substituted, resolved form of an abstract checked type, or null when the
    // substitution does not change it. Substituted associated types resolve through conformance
    // witness tables that may not be checked yet at the call site, so resolve through the same
    // helper the storage classifier uses.
    Type* _substituteCheckedType(Type* type)
    {
        int diff = 0;
        auto substitutedVal = type->substituteImpl(getASTBuilder(), subst, &diff);
        if (!diff || !substitutedVal)
            return nullptr;
        auto substitutedType = as<Type>(substitutedVal);
        if (!substitutedType)
            return nullptr;
        return _resolveSubstitutedFieldType(this, substitutedType);
    }

    void _reportUse(Diagnostics::StructuralRayTracingInputConstruction const& diagnostic)
    {
        diagnoseOnce(diagnostic);
        diagnoseOnce(Diagnostics::StructuralRayTracingInputGenericInstantiation{
            .callee = calleeName,
            .location = instantiationLoc});
    }

    void _reportUse(Diagnostics::StructuralRayTracingInputStorage const& diagnostic)
    {
        diagnoseOnce(diagnostic);
        diagnoseOnce(Diagnostics::StructuralRayTracingInputGenericInstantiation{
            .callee = calleeName,
            .location = instantiationLoc});
    }

    // Applies the same storage policy diagnoseInvalidVariableType applies to a concrete
    // declaration, to the type this instantiation revealed for an abstract `T` declaration:
    // an `in` parameter receives an existing input from its caller, and an initialized local
    // copies one. Everything else — an uninitialized local, a `static` local, a `out`/`inout`
    // parameter, a global, a field — stores a fresh or aliased input and is rejected.
    void _checkVarDecl(VarDeclBase* varDecl)
    {
        auto declaredType = varDecl->getType();
        if (!declaredType)
            return;
        auto substitutedType = _substituteCheckedType(declaredType);
        if (!substitutedType ||
            _getRestrictedTypeKind(this, substitutedType) == RestrictedTypeKind::None)
            return;

        if (_getDirectStageInputKind(substitutedType) != Stage::Unknown)
        {
            if (auto param = as<ParamDecl>(varDecl))
            {
                if (getExplicitlyDeclaredParamPassingMode(param) == ParamPassingMode::In &&
                    !param->hasModifier<HLSLUniformModifier>())
                    return;
            }
            else if (
                isLocalVar(varDecl) && !varDecl->hasModifier<HLSLStaticModifier>() &&
                varDecl->initExpr)
            {
                return;
            }
        }
        _reportUse(Diagnostics::StructuralRayTracingInputStorage{
            .type = substitutedType,
            .location = varDecl->loc});
    }

    // A construction whose substituted result is a stage input would manufacture an input the
    // invocation never supplied. The direct spelling fails on the private constructor's
    // access check; through a generic parameter the constructor was resolved abstractly, so
    // the use is reported here.
    void _checkConstructionExpr(Expr* expr)
    {
        auto resultType = expr->type.type;
        if (!resultType)
            return;
        auto substitutedType = _substituteCheckedType(resultType);
        if (!substitutedType || _getDirectStageInputKind(substitutedType) == Stage::Unknown)
            return;
        _reportUse(Diagnostics::StructuralRayTracingInputConstruction{
            .type = substitutedType,
            .location = expr->loc});
    }

    // A call inside the generic body may itself be generic, with arguments still in terms of
    // this generic's parameters. Once the outer substitution makes those arguments concrete,
    // the callee's body needs the same walk — this is how a chain of generic helpers is
    // covered. A call whose arguments were already concrete in the abstract body was checked
    // when the abstract body itself was checked, so only a changed application recurses.
    void _checkInnerSpecialization(GenericAppDeclRef* genericAppDeclRef)
    {
        int diff = 0;
        auto specialized = static_cast<DeclRefBase*>(
            genericAppDeclRef->substituteImpl(getASTBuilder(), subst, &diff));
        if (!diff || !specialized || specialized == genericAppDeclRef)
            return;
        SourceLoc innerLoc =
            sourceLocStack.getCount() ? sourceLocStack.getLast() : instantiationLoc;
        checkSpecializedCallableBody(this, specialized, innerLoc);
    }
};

// Returns true when the declaration tree contains one of the ray-tracing stage-input
// structures (MissInput, ClosestHitInput, and their siblings). Only a declaration with one of
// these annotations classifies as a restricted type, so nothing in a compilation whose loaded
// modules declare none can ever be diagnosed by these checks.
static bool _declTreeDeclaresStageInputs(Decl* decl)
{
    // A generic declaration holds its parameters and constraints as members, but its inner
    // declaration — where a generic struct's own annotations live — is not a member, so follow
    // it explicitly.
    if (auto genericDecl = as<GenericDecl>(decl))
    {
        if (auto inner = getInner(genericDecl))
        {
            if (_declTreeDeclaresStageInputs(inner))
                return true;
        }
    }
    if (auto aggTypeDecl = as<AggTypeDecl>(decl))
    {
        if (getStageInputKind(aggTypeDecl) != Stage::Unknown)
            return true;
    }
    if (auto containerDecl = as<ContainerDecl>(decl))
    {
        for (auto member : containerDecl->getDirectMemberDecls())
        {
            if (_declTreeDeclaresStageInputs(member))
                return true;
        }
    }
    return false;
}

// Returns the per-compilation check state, creating it on first use.
static PortableRTSpecializationCheckState* _getCheckState(Linkage* linkage)
{
    auto& state = linkage->m_portableRTSpecializationCheckState;
    if (!state)
        state = new PortableRTSpecializationCheckState();
    return state;
}

// Returns true when a loaded module of this compilation declares one of the stage-input
// structures. Only such a declaration classifies as a restricted type, so when none is
// loaded, no check below can report anything. The scan runs once per load-count change and
// its result is cached on the linkage.
static bool _compilationCanDeclareStageInputs(Linkage* linkage)
{
    auto state = _getCheckState(linkage);
    if (state->loadedModuleCountAtScan != linkage->loadedModulesList.getCount())
    {
        state->loadedModuleCountAtScan = linkage->loadedModulesList.getCount();
        state->stageInputsLoaded = false;
        for (auto loadedModule : linkage->loadedModulesList)
        {
            auto moduleDecl = loadedModule ? loadedModule->getModuleDecl() : nullptr;
            if (moduleDecl && _declTreeDeclaresStageInputs(moduleDecl))
            {
                state->stageInputsLoaded = true;
                break;
            }
        }
    }
    return state->stageInputsLoaded;
}

void checkSpecializedCallableBody(
    SemanticsVisitor* visitor,
    DeclRefBase* specializedCallableDeclRef,
    SourceLoc instantiationLoc)
{
    if (!specializedCallableDeclRef)
        return;

    auto linkage = visitor->getShared()->m_linkage;
    if (!linkage)
        return;

    // The stage-input module is experimental, so a compilation without the experimental opt-in
    // cannot have loaded it and no walk can report anything.
    if (!linkage->m_optionSet.getBoolOption(CompilerOptionName::ExperimentalFeature))
        return;

    // The opt-in is broader than this feature: every standard-module build passes it too. Only
    // a loaded stage-input declaration can make a type restricted, so a compilation with none
    // has nothing to check.
    auto state = _getCheckState(linkage);
    if (!_compilationCanDeclareStageInputs(linkage))
        return;

    SubstitutionSet subst(specializedCallableDeclRef);
    if (!subst.findGenericAppDeclRef())
        return;

    auto callableDecl = as<CallableDecl>(specializedCallableDeclRef->getDecl());
    if (!callableDecl)
        return;

    // One instantiation is checked once per compilation, however many call sites complete it.
    // Recording the instantiation before walking also stops a recursive generic — a body that
    // calls another instantiation of itself — from cycling.
    if (!state->checkedSpecializationDeclRefs.add(specializedCallableDeclRef))
        return;

    // The walk reads the checked body; the declaration's own checking finishes it if the call
    // site appears before the body's checks have run.
    visitor->ensureDecl(callableDecl, DeclCheckState::DefinitionChecked);

    _SpecializedCallableBodyRTChecker checker(*visitor);
    checker.subst = subst;
    checker.instantiationLoc = instantiationLoc;
    checker.calleeName = getText(callableDecl->getName());
    checker.dispatchIfNotNull(callableDecl);
}

} // namespace PortableRT
} // namespace Slang
