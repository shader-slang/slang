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

// This file owns the structural ray-tracing rules and their declaration identities.
// Ordinary interfaces, associated-type constraints, overload resolution, and capability inference
// remain in the normal checker. Their call sites delegate here only for the additional rules:
// stage receivers and inputs, stage selection, schema entry uniqueness, API mixing, and the
// boundary between logical shaders and native entry points. Each rule below states its example
// and the representation or API contract it preserves.
//
// Schemas, lists, and hit groups are ordinary values. Empty payloads are ordinary payloads.
// Their ordinary generic uses are allowed; stage/input generic uses retain the restriction below.

// =============================================================================
// File-local helpers
// =============================================================================

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

static StructuralRayTracingStageKind _getDirectStageInputKind(Type* type)
{
    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return getStructuralRayTracingStageInputKind(typeDecl);
}

// Requires stage implementations to be ordinary structs without instance fields.
//
// Consider this example:
//
//     struct StatefulClosestHit : rt::IClosestHitShader
//     {
//         uint state;
//         ...
//     }
//
// The RT API selects StatefulClosestHit by type. It does not receive an application-created
// StatefulClosestHit object containing a value for state, so this field is rejected. Application
// data can be supplied through the payload or shader record instead.
// This check runs after StatefulClosestHit passes ordinary interface conformance checking in its
// module. It rejects the field even if no entry point or schema ever uses StatefulClosestHit.
// Entry-point selection is not what triggers this check.
//
// Ordinary declaration and conformance checking reuse completed results. Separate conformances
// can still reach this helper and inspect the same field again, for example when two stages inherit
// from one base struct. diagnoseOnce suppresses repeated error messages; it does not skip the walk.
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

    auto stageDeclRef = isDeclRefTypeOf<AggTypeDecl>(resolvedStageType);
    const bool isCompilerBuiltinType =
        stageDeclRef && (stageDeclRef.getDecl()->findModifier<BuiltinTypeModifier>() ||
                         stageDeclRef.getDecl()->findModifier<MagicTypeModifier>());

    // A user-defined interface may refine a stage contract with additional requirements. It is
    // not itself an executable implementation and therefore has no receiver to validate here.
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
// the same module conflict. Dispatch calls do not select the module's API family: an ordinary
// library wrapper such as rt::RayTracer.trace may call TraceRay in its native target branch.
// The compiler handles the resulting intrinsic operation without treating that implementation
// detail as a legacy shader declaration.
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

enum class StructuralRayTracingRuntimeTypeKind
{
    None,
    Stage,
    StageInput,
};

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
// manufacturing it to a generic method. An `in` parameter and its initialized local copies refer
// to the same invocation. Schema/group/list values and ordinary generic arguments are unrestricted.
static bool _checkRayTracingStageOrInputUse(
    SemanticsVisitor* visitor,
    const RayTracingStageOrInputUse& use)
{
    if (use.kind == RayTracingStageOrInputUse::Kind::GenericArguments)
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

    auto type = use.variable ? use.variable->getType() : use.type;
    auto location = use.variable ? use.variable->loc : use.location;
    auto kind = _findStructuralRuntimeType(visitor, type, location);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return false;
    if (use.variable && kind == StructuralRayTracingRuntimeTypeKind::StageInput &&
        _getDirectStageInputKind(type) != StructuralRayTracingStageKind::Count)
    {
        // Consider `void helper(in MissInput<C> input) { let local = input; }`.
        // Parameter direction and local scope already have shared AST helpers. Initialization
        // checking supplies the checked copy expression; it does not need an RT-specific copy
        // operation. Fresh constructors and initializer lists are checked at their expression.
        if (auto param = as<ParamDecl>(use.variable))
        {
            if (getExplicitlyDeclaredParamPassingMode(param) == ParamPassingMode::In &&
                !param->hasModifier<HLSLUniformModifier>())
                return false;
        }
        else if (
            isLocalVar(use.variable) && !use.variable->hasModifier<HLSLStaticModifier>() &&
            use.variable->initExpr)
        {
            return false;
        }
    }
    if (use.kind == RayTracingStageOrInputUse::Kind::Construction)
    {
        // The same constructor expression can be checked at its call and again when coerced
        // into a variable or argument. Use the checker's existing diagnostic suppression.
        visitor->diagnoseOnce(
            Diagnostics::StructuralRayTracingTypeConstruction{.type = type, .location = location});
    }
    else
    {
        _diagnoseInvalidStructuralRayTracingRuntimeType(visitor, kind, type, location);
    }
    return true;
}

/// Checks a type use now, or saves it until the current module has finished ordinary checking.
///
/// Some uses require finding which interfaces a type implements. During module checking, that
/// lookup can revisit a declaration whose generic constraints are still being checked. Save the
/// record instead; diagnosePendingRayTracingStageOrInputUses checks it after the module's
/// declarations and function bodies are checked. When deferral is disabled, check immediately.
/// There may be no later module-completion step, such as when reflecting an already-checked type.
///
/// Consider this example:
///
///     void helper<C : rt::IPayloadContext>()
///     {
///         rt::MissInput<C> fresh;
///     }
///
/// The uninitialized input is invalid. While checking the module, this function saves that use
/// and returns false; the pending-use check reports the error later. With deferral disabled,
/// this function checks the same use immediately and returns true.
///
/// @param visitor The semantic checker whose shared state controls whether to defer the check.
/// @param use The declaration or type-use record to check, including its diagnostic location.
/// @retval true The immediate check rejects this use. Expression callers can return an error
/// expression; a repeated diagnostic may have been suppressed.
/// @retval false The use was queued, skipped, or checked without an RT-use rejection. Skipped uses
/// have no type arguments to check, or have a missing, erroneous, or concrete scalar/void type.
/// A queued use has not passed validation yet; false also does not rule out errors from other
/// semantic checks.
static bool _diagnoseOrDeferRayTracingStageOrInputUse(
    SemanticsVisitor* visitor,
    const RayTracingStageOrInputUse& use)
{
    if (use.kind == RayTracingStageOrInputUse::Kind::GenericArguments)
    {
        if (use.genericArguments.getCount() == 0)
            return false;
    }
    else
    {
        auto type = unwrapModifiedType(use.variable ? use.variable->getType() : use.type);
        // A call returning float or void cannot produce a stage or stage input. Built-in scalar
        // types cannot contain fields, and stage conformance checking rejects them as stage
        // implementations. Skip these uses before queueing or querying inheritance. Generic
        // arguments still need their separate check: make<MissInput<C>>() may return void.
        if (!type || as<ErrorType>(type) || as<BasicExpressionType>(type))
            return false;
    }

    auto shared = visitor->getShared();
    if (shared->m_deferRayTracingStageOrInputUses)
    {
        shared->m_pendingRayTracingStageOrInputUses.add(use);
        return false;
    }
    return _checkRayTracingStageOrInputUse(visitor, use);
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

// Prevents reachable structural dispatch operations from reaching emission before lowering.
//
// Consider a ray-generation entry point calling tracer.trace(desc, scene, program). Its ordinary
// library wrapper creates an empty payload and calls the payload overload. On Metal, that body
// calls __metalTraceRay or __metalTraceRayMotion, which lowers directly to IRMetalTraceRay. Follow
// the ordinary reference graph through wrappers and generic specializations, then inspect each
// reached function's instructions. linkIR has already selected target-switch branches, so native
// paths do not acquire a Metal diagnostic. An unreachable function is harmless. Callable dispatch
// still uses its existing function decoration until its own instruction is introduced.
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
        const char* operationName = nullptr;
        if (marker)
        {
            auto operationKindInst = as<IRIntLit>(marker->getOperationKind());
            SLANG_RELEASE_ASSERT(operationKindInst);
            SLANG_RELEASE_ASSERT(
                StructuralRayTracingSourceOperationKind(operationKindInst->getValue()) ==
                StructuralRayTracingSourceOperationKind::CallShader);
            operationName = "callShader";
        }
        else
        {
            for (auto block : func->getBlocks())
            {
                for (auto instruction : block->getOrdinaryInsts())
                {
                    if (as<IRMetalTraceRay>(instruction))
                    {
                        operationName = "trace";
                        break;
                    }
                }
                if (operationName)
                    break;
            }
        }

        if (!operationName)
            continue;

        // Diagnose the user's entry point rather than the operation's installed-library location.
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

// =============================================================================
// Shared functions used by other compiler files
// =============================================================================

// Allows the ray-tracing library to select its stage-interface and descriptor AST types.
// For example, `__magic_type(MissShaderType) interface IMissShader` and
// `__magic_type(TraceProgramDescriptorType) struct TraceProgramDescriptor<Schema>` both store
// their checked declaration reference. Existing type construction can therefore preserve their
// generic arguments without a custom representation. Other core magic classes may require
// different operands and cannot safely be applied to an ordinary declaration.
bool isRayTracingBuiltinTypeModifier(MagicTypeModifier* modifier, Decl* decl)
{
    if (!modifier || !modifier->magicNodeType || !modifier->magicNodeType.getInfo()->createFunc)
        return false;
    return (as<InterfaceDecl>(decl) &&
            modifier->magicNodeType.isSubClassOf<RayTracingStageInterfaceType>()) ||
           (as<StructDecl>(decl) &&
            modifier->magicNodeType.isSubClassOf<TraceProgramDescriptorType>());
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

// Identifies requirements whose concrete types are recorded for later backend code generation.
// Consider `SkyMiss.Context = SkyContext`, with `SkyContext.Payload = ColorPayload`: generating
// the native miss function will need ColorPayload, so checking must find these requirements and
// read their ordinary Slang conformance results. Looking them up by name would also work. We use
// KnownBuiltin roles so renaming Context to ShaderContext while keeping StageContext requires no
// compiler lookup change; it is a maintenance choice, not a shortcut for resolving the types.
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
    if (!result.types)
        result.types = astBuilder->getTypePack(ArrayView<Type*>());
    SLANG_RELEASE_ASSERT(
        !result.witnesses || result.witnesses->getCount() == result.types->getTypeCount());
    return result;
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

// =============================================================================
// SemanticsVisitor methods
// =============================================================================

// Checks the extra RT API restrictions after ordinary interface conformance checking succeeds.
// For example, `struct Miss : rt::IMissShader` must have no instance fields and cannot share a
// module with legacy ray-tracing stages. Ordinary conformance checking has already verified its
// required members, such as Context and invoke. Applying these restrictions at that point also
// covers a Miss listed in a schema without being selected directly with `-entry Miss`.
// A conformance to an ordinary interface, such as ITraceProgramSchema, needs no stage restrictions.
void SemanticsVisitor::checkStructuralRayTracingStageRestrictions(
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

void SemanticsVisitor::diagnoseDuplicateStructuralRayTracingSchemaEntries(
    Type* entryListType,
    AssocTypeDecl* associatedTypeRequirement,
    Type* schemaType,
    SourceLoc satisfyingDeclLoc)
{

    auto sectionName = _getStructuralRayTracingSchemaSectionName(associatedTypeRequirement);
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
            .location = satisfyingDeclLoc});
    }
}

// Defers storage checks until ordinary module checking has completed type inheritance.
// For example, inspecting Box<T> while T's generic constraints are still being checked must not
// recursively ask the inheritance checker to finish that same unfinished declaration.
void SemanticsVisitor::beginStructuralRayTracingModule()
{
    SLANG_RELEASE_ASSERT(!getShared()->m_deferRayTracingStageOrInputUses);
    getShared()->m_deferRayTracingStageOrInputUses = true;
}

// Finishes stage and module-wide rules after checked conformance witnesses are available.
// For example, a generic extension can supply Miss.invoke; checking its selected stage before
// ordinary conformance and capability inference finish would inspect an incomplete signature.
void SemanticsVisitor::checkStructuralRayTracingModule(ModuleDecl* moduleDecl)
{
    _checkAttributedLegacyEntryPoints(getModule(moduleDecl), moduleDecl, getSink());
    _diagnoseStructuralStageDeclarations(this, moduleDecl);
}

void SemanticsVisitor::diagnosePendingRayTracingStageOrInputUses()
{
    auto shared = getShared();
    SLANG_RELEASE_ASSERT(shared->m_deferRayTracingStageOrInputUses);
    // Inheritance queries can finish a lazily synthesized declaration. Its own uses join this
    // queue, so copy the current record before checking and drain newly added records as well.
    for (Index i = 0; i < shared->m_pendingRayTracingStageOrInputUses.getCount(); ++i)
    {
        auto use = shared->m_pendingRayTracingStageOrInputUses[i];
        _checkRayTracingStageOrInputUse(this, use);
    }
    shared->m_pendingRayTracingStageOrInputUses.clear();
    shared->m_deferRayTracingStageOrInputUses = false;
}

/// Checks ray-tracing restrictions on parameters, local variables, globals, and fields.
/// VarDeclBase represents all of these declarations, including ParamDecl. Consider this example:
///
///     void helper(in rt::MissInput<C> input)
///     {
///         rt::MissInput<C> copy = input;
///         rt::MissInput<C> fresh;
///     }
///
/// A stage input is allowed as a non-uniform `in` parameter or an initialized, non-static local
/// copy. Here, input receives the view from its caller, and copy refers to that same view. Neither
/// needs to create invocation state. The uninitialized fresh declaration is rejected, as are
/// globals and fields of this type. Constructor expressions are checked separately.
///
/// Retain the VarDeclBase so deferred checking can inspect its parameter direction, scope,
/// modifiers, and checked initializer instead of keeping only its type.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingVariableType(VarDeclBase* varDecl)
{
    RayTracingStageOrInputUse use;
    use.kind = RayTracingStageOrInputUse::Kind::Variable;
    use.variable = varDecl;
    _diagnoseOrDeferRayTracingStageOrInputUse(this, use);
}

/// Checks function and method return types for shader-stage or stage-input values.
/// For example, `rt::MissInput<C> getInput();` is rejected because the current input-view rules
/// allow inputs only as `in` parameters and initialized local copies. This checks the declared
/// return type; it does not inspect the function body. Constructors are checked at their
/// construction expressions instead of their declarations.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingReturnType(CallableDecl* decl)
{
    if (as<ConstructorDecl>(decl))
        return;
    RayTracingStageOrInputUse use;
    use.type = decl->returnType.type;
    use.location = decl->returnType.exp ? decl->returnType.exp->loc : decl->loc;
    _diagnoseOrDeferRayTracingStageOrInputUse(this, use);
}

// Prevents properties from manufacturing stage/input views outside the adapter parameter.
// For example, `property rt::MissInput<C> input { get; }` promises a value with no source storage.
// Payload and record properties have ordinary application-defined result types and remain legal.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingPropertyType(PropertyDecl* propertyDecl)
{
    RayTracingStageOrInputUse use;
    use.type = propertyDecl->type.type;
    use.location = propertyDecl->type.exp ? propertyDecl->type.exp->loc : propertyDecl->loc;
    _diagnoseOrDeferRayTracingStageOrInputUse(this, use);
}

// Checks actual construction and call results after normal expression checking. Consider:
//
//     use(rt::MissInput<C>(input)); // Same-type coercion returns `input` itself: allowed.
//     use(rt::MissInput<C>());      // A constructor creates a fresh view: rejected.
//     use({});                     // Aggregate/default initialization is also fresh: rejected.
//
// ResolveInvoke and coercion already distinguish these cases. Use their checked
// expression rather than treating every `Type(...)` spelling as construction. A specialized
// ordinary call such as `makeDefault<MissInput<C>>()` is still an invalid input result.
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingExpression(Expr* expr)
{
    RayTracingStageOrInputUse use;
    use.type = expr->type;
    use.location = expr->loc;
    if (auto invoke = as<InvokeExpr>(expr))
    {
        if (auto callee = as<DeclRefExpr>(invoke->functionExpr))
        {
            if (as<ConstructorDecl>(callee->declRef.getDecl()))
            {
                use.kind = RayTracingStageOrInputUse::Kind::Construction;
                use.location = invoke->functionExpr->loc;
            }
        }
    }
    else if (as<InitializerListExpr>(expr) || as<DefaultConstructExpr>(expr))
    {
        use.kind = RayTracingStageOrInputUse::Kind::Construction;
    }
    else
    {
        return false;
    }
    return _diagnoseOrDeferRayTracingStageOrInputUse(this, use);
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
    if (isStructuralRayTracingCallShaderMethod(functionDecl))
        return false;

    RayTracingStageOrInputUse use;
    use.kind = RayTracingStageOrInputUse::Kind::GenericArguments;
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
    return _diagnoseOrDeferRayTracingStageOrInputUse(this, use);
}

// Rejects stage/input arguments in arbitrary generic types for the same specialization reason.
// For example, even an empty Factory<T> can define a method that constructs T internally.
// The standard shader lists explicitly represent type packs and do not manufacture their entries.
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericTypeApplication(
    GenericAppExpr* genericApplication,
    Expr* checkedResult)
{
    RayTracingStageOrInputUse use;
    use.kind = RayTracingStageOrInputUse::Kind::GenericArguments;
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
    return _diagnoseOrDeferRayTracingStageOrInputUse(this, use);
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

} // namespace Slang
