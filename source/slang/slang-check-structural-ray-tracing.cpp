#include "slang-check-impl.h"
#include "slang-lookup.h"
#include "slang-session.h"
#include "slang-syntax.h"

namespace Slang
{

static Stage _getNativeStage(StructuralRayTracingStageKind kind);
static StructuralRayTracingStageKind _getStructuralStage(Stage stage);
static StructuralRayTracingStageKind _getDirectStageInputKind(
    const StructuralRayTracingCheckingState& state,
    Type* type);

static FunctionDeclBase* _getStageImplementation(
    const StructuralRayTracingCheckingState& state,
    InterfaceDecl* stageInterface,
    WitnessTable* witnessTable)
{
    auto invokeRequirement = state.getStageInvokeRequirement(stageInterface);
    RequirementWitness invokeWitness;
    if (!invokeRequirement || !witnessTable ||
        !witnessTable->getRequirementDictionary().tryGetValue(invokeRequirement, invokeWitness) ||
        invokeWitness.getFlavor() != RequirementWitness::Flavor::declRef)
    {
        return nullptr;
    }

    Decl* implementation = invokeWitness.getDeclRef().getDecl();
    while (auto genericDecl = as<GenericDecl>(implementation))
        implementation = genericDecl->inner;
    return as<FunctionDeclBase>(implementation);
}

// Diagnoses direct instance fields on one stage or base-struct declaration.
static bool _validateStructuralRayTracingStageFields(
    StructuralRayTracingCheckingState& state,
    AggTypeDecl* stageType,
    DiagnosticSink* sink)
{
    const bool shouldDiagnose = state.beginStageRepresentationDeclarationCheck(stageType);
    bool isValid = true;
    for (auto field : stageType->getFields())
    {
        if (isEffectivelyStatic(field))
            continue;

        isValid = false;
        if (shouldDiagnose)
            sink->diagnose(Diagnostics::StructuralRayTracingStageInstanceField{.field = field});
    }
    return isValid;
}

// Rejects storage on a concrete structural stage wherever its conformance is selected.
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
// The checking state suppresses duplicate diagnostics per inspected declaration because conformance
// checking may publish the same completed witness more than once, and several stages may share one
// stateful base struct.
static bool _validateStructuralRayTracingStageStorage(
    StructuralRayTracingCheckingState& state,
    ASTBuilder* astBuilder,
    Type* stageType,
    SourceLoc conformanceLoc,
    DiagnosticSink* sink)
{
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
        return true;

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
        const bool shouldDiagnose =
            hasSourceAggregateDeclaration
                ? state.beginStageRepresentationDeclarationCheck(stageDeclRef.getDecl())
                : state.beginStageRepresentationTypeCheck(witnessedStageType);
        if (shouldDiagnose)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingStageImplementationMustBeStruct{
                .stageType = witnessedStageType,
                .location =
                    hasSourceAggregateDeclaration ? stageDeclRef.getDecl()->loc : conformanceLoc});
        }
        return false;
    }

    bool isValid = true;
    auto structType = stageDeclRef.as<StructDecl>();
    // A base struct contributes storage to the compiler-created stage value even though its
    // fields are not direct members of the concrete implementation. Follow the checked base
    // declaration references so generic base specializations use the same semantic inheritance
    // path as ordinary struct layout.
    for (auto currentType = structType; currentType;)
    {
        if (!_validateStructuralRayTracingStageFields(state, currentType.getDecl(), sink))
        {
            isValid = false;
        }
        currentType = findBaseStructDeclRef(astBuilder, currentType);
    }
    return isValid;
}

static void _registerRayTracingAPIUse(
    Linkage* linkage,
    Module* module,
    RayTracingAPIFamily family,
    Decl* decl,
    DiagnosticSink* sink)
{
    auto& state = linkage->getStructuralRayTracingCheckingState();

    Decl* otherDecl = nullptr;
    if (!state.registerAPIUse(module, family, decl, &otherDecl))
        return;

    auto currentAPI = family == RayTracingAPIFamily::Structural ? "structural" : "legacy";
    auto otherAPI = family == RayTracingAPIFamily::Structural ? "legacy" : "structural";
    sink->diagnose(Diagnostics::MixedRayTracingApis{
        .currentAPI = currentAPI,
        .otherAPI = otherAPI,
        .currentDecl = decl,
        .otherDecl = otherDecl});
}

void registerRayTracingAPICall(
    Linkage* linkage,
    FunctionDeclBase* caller,
    FunctionDeclBase* callee,
    DiagnosticSink* sink)
{
    auto& state = linkage->getStructuralRayTracingCheckingState();
    if (!caller || !callee)
        return;

    auto callerModule = getModule(caller);
    if (!callerModule || state.isTraceMethod(caller) || state.isCallShaderMethod(caller))
        return;

    if (state.isTraceMethod(callee) || state.isCallShaderMethod(callee))
    {
        _registerRayTracingAPIUse(
            linkage,
            callerModule,
            RayTracingAPIFamily::Structural,
            caller,
            sink);
    }
    else if (isCoreLegacyRayTracingPipelineMethod(callee))
    {
        _registerRayTracingAPIUse(linkage, callerModule, RayTracingAPIFamily::Legacy, caller, sink);
    }
}

void SemanticsVisitor::registerStructuralRayTracingStageConformance(
    DeclRef<InterfaceDecl> superInterfaceDeclRef,
    WitnessTable* witnessTable,
    SourceLoc conformanceLoc)
{
    auto& state = getLinkage()->getStructuralRayTracingCheckingState();
    auto stageKind = state.getStageKind(superInterfaceDeclRef.getDecl());
    auto metadataKind = state.getMetadataKind(superInterfaceDeclRef.getDecl());
    if ((stageKind == StructuralRayTracingStageKind::Count &&
         metadataKind == StructuralRayTracingMetadataKind::Count) ||
        !witnessTable)
        return;

    auto witnessedType = witnessTable->witnessedType;
    auto witnessedDeclRef =
        isDeclRefTypeOf<AggTypeDecl>(witnessedType ? as<Type>(witnessedType->resolve()) : nullptr);
    auto witnessedDecl = witnessedDeclRef ? witnessedDeclRef.getDecl() : nullptr;
    if (witnessedDecl)
    {
        _registerRayTracingAPIUse(
            getLinkage(),
            getModule(witnessedDecl),
            RayTracingAPIFamily::Structural,
            witnessedDecl,
            getSink());
    }

    if (stageKind == StructuralRayTracingStageKind::Count)
        return;

    _validateStructuralRayTracingStageStorage(
        state,
        getASTBuilder(),
        witnessedType,
        conformanceLoc,
        getSink());

    state.registerStageImplementation(
        _getStageImplementation(state, superInterfaceDeclRef.getDecl(), witnessTable),
        stageKind);
}

// Returns the section name for a structural schema requirement used in duplicate-entry diagnostics,
// or null for other associated-type requirements.
static const char* _getStructuralRayTracingSchemaSectionName(
    const StructuralRayTracingCheckingState& state,
    AssocTypeDecl* requirement)
{
    switch (state.getAssociatedTypeKind(requirement))
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
    auto& state = getLinkage()->getStructuralRayTracingCheckingState();

    auto sectionName = _getStructuralRayTracingSchemaSectionName(state, associatedTypeRequirement);
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

void diagnoseMixedRayTracingAPIUse(EntryPoint* entryPoint, DiagnosticSink* sink)
{
    if (!_isLegacyRayTracingStage(entryPoint->getStage()))
        return;

    auto entryPointDecl = entryPoint->getFuncDecl();
    auto family = entryPoint->isStructuralRayTracingEntryPoint() ? RayTracingAPIFamily::Structural
                                                                 : RayTracingAPIFamily::Legacy;
    _registerRayTracingAPIUse(
        entryPoint->getLinkage(),
        getModule(entryPointDecl),
        family,
        entryPointDecl,
        sink);
}

static void _registerAttributedLegacyEntryPoints(
    Linkage* linkage,
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
                    _registerRayTracingAPIUse(
                        linkage,
                        module,
                        RayTracingAPIFamily::Legacy,
                        functionDecl,
                        sink);
                }
            }
        }

        if (auto childContainer = as<ContainerDecl>(innerMember))
            _registerAttributedLegacyEntryPoints(linkage, module, childContainer, sink);
    }
}

static void _diagnoseInvalidStructuralStageCapabilities(
    StructuralRayTracingCheckingState& state,
    ContainerDecl* containerDecl,
    DiagnosticSink* sink)
{
    // A structural stage has no `[shader]` attribute from which ordinary capability checking can
    // obtain its native stage. Compare the completed `invoke` requirements with the stage implied
    // by their annotated interface so helpers that use stage-restricted intrinsics receive the same
    // validation as a legacy entry point.
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto innerMember = member;
        if (auto genericDecl = as<GenericDecl>(innerMember))
            innerMember = genericDecl->inner;

        if (auto functionDecl = as<FunctionDeclBase>(innerMember))
        {
            auto stageKind = state.getStageKind(functionDecl);
            auto stage = _getNativeStage(stageKind);
            auto capabilities = functionDecl->inferredCapabilityRequirements;
            if (stage != Stage::Unknown && capabilities &&
                capabilities->isIncompatibleWith(getAtomFromStage(stage)))
            {
                sink->diagnose(Diagnostics::DeclHasDependenciesNotCompatibleOnStage{
                    .stage = getStageName(stage),
                    .decl = functionDecl});
            }
        }

        if (auto childContainer = as<ContainerDecl>(innerMember))
        {
            _diagnoseInvalidStructuralStageCapabilities(state, childContainer, sink);
        }
    }
}

// Returns the native stage that constrains a function accepting a structural stage input.
//
// Consider this example:
//
//     [shader("anyhit")]
//     void nativeAnyHit(rt::ClosestHitInput<C> input) { ... }
//
// `nativeAnyHit` is not an implementation of a structural stage interface, so it has no entry in
// the structural-stage checking state. Its checked `EntryPointAttribute` is nevertheless an
// explicit any-hit contract and must win over the fallback that infers a stage from an
// otherwise-unannotated helper's first stage-input parameter. Preserve the native `Stage` here:
// mapping a compute or miss entry point to `StructuralRayTracingStageKind::Count` would make a
// known mismatched stage look the same as an unconstrained helper.
static Stage _getRequiredStageForStructuralInput(
    StructuralRayTracingCheckingState& state,
    FunctionDeclBase* functionDecl)
{
    auto stageKind = state.getStageKind(functionDecl);
    if (stageKind != StructuralRayTracingStageKind::Count)
        return _getNativeStage(stageKind);

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

static void _diagnoseInvalidStructuralStageInputParameters(
    Linkage* linkage,
    StructuralRayTracingCheckingState& state,
    ContainerDecl* containerDecl,
    DiagnosticSink* sink)
{
    // A stage-input value is a view of native state supplied only in one stage. Check every direct
    // parameter after declarations and capabilities are complete: an unannotated helper inherits
    // its requirement from its first input, while a structural or legacy entry point must agree
    // with that input explicitly.
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto innerMember = member;
        if (auto genericDecl = as<GenericDecl>(innerMember))
            innerMember = genericDecl->inner;

        if (auto functionDecl = as<FunctionDeclBase>(innerMember))
        {
            auto functionStage = _getRequiredStageForStructuralInput(state, functionDecl);
            for (auto parameter : functionDecl->getParameters())
            {
                auto inputStage = _getDirectStageInputKind(state, parameter->type.type);
                if (inputStage == StructuralRayTracingStageKind::Count)
                    continue;

                // A stage-input parameter is itself a use of the structural API. Recording that
                // fact here makes a native entry point such as
                //
                //     [shader("closesthit")]
                //     void closestHit(ClosestHitInput<C> input);
                //
                // participate in the same-module mixed-API rule even though its native stage
                // happens to match the input view. The stage match below answers a different
                // question: whether a structural implementation or ordinary helper is restricted
                // to the stage that can supply this input.
                _registerRayTracingAPIUse(
                    linkage,
                    getModule(functionDecl),
                    RayTracingAPIFamily::Structural,
                    functionDecl,
                    sink);

                auto requiredInputStage = _getNativeStage(inputStage);
                if (functionStage == Stage::Unknown)
                {
                    // A stage-input parameter implicitly restricts an otherwise-unannotated
                    // helper. Additional stage-input parameters must agree with that stage.
                    functionStage = requiredInputStage;
                    continue;
                }
                if (requiredInputStage == functionStage)
                    continue;

                auto location = parameter->type.exp ? parameter->type.exp->loc : parameter->loc;
                sink->diagnose(Diagnostics::StructuralRayTracingInputStageMismatch{
                    .type = parameter->type.type,
                    .stage = getStageName(requiredInputStage),
                    .function = functionDecl,
                    .location = location});
            }
        }

        if (auto childContainer = as<ContainerDecl>(innerMember))
            _diagnoseInvalidStructuralStageInputParameters(linkage, state, childContainer, sink);
    }
}

void diagnoseMixedRayTracingAPIsInModule(Linkage* linkage, Module* module, DiagnosticSink* sink)
{
    auto& state = linkage->getStructuralRayTracingCheckingState();
    _registerAttributedLegacyEntryPoints(linkage, module, module->getModuleDecl(), sink);
    _diagnoseInvalidStructuralStageCapabilities(state, module->getModuleDecl(), sink);
    _diagnoseInvalidStructuralStageInputParameters(linkage, state, module->getModuleDecl(), sink);
}

static DeclRef<FuncDecl> _getStageImplementationFromSubtypeWitness(
    ASTBuilder* astBuilder,
    const StructuralRayTracingCheckingState& state,
    InterfaceDecl* stageInterface,
    SubtypeWitness* witness)
{
    witness = witness ? as<SubtypeWitness>(witness->resolve()) : nullptr;
    auto invokeRequirement = state.getStageInvokeRequirement(stageInterface);
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

static StructuralRayTracingStageKind _findStageImplementationFromParentConformance(
    SemanticsVisitor* visitor,
    StructuralRayTracingCheckingState& state,
    FunctionDeclBase* functionDecl)
{
    // A serialized stage can implement a refinement such as IMyMiss : IMissShader. Its direct
    // base is an ordinary interface, so inspect that interface's checked facets and project the
    // existing conformance witness to the executable stage contract. Comparing the selected
    // requirement witness identifies invoke without treating other methods as shader bodies.
    Decl* parent = functionDecl->parentDecl;
    while (auto genericDecl = as<GenericDecl>(parent))
        parent = genericDecl->parentDecl;
    auto container = as<ContainerDecl>(parent);
    if (!container)
        return StructuralRayTracingStageKind::Count;

    auto astBuilder = visitor->getASTBuilder();
    for (auto inheritanceDecl : container->getDirectMemberDeclsOfType<InheritanceDecl>())
    {
        auto witnessTable = inheritanceDecl->witnessTable;
        auto interfaceType = as<DeclRefType>(inheritanceDecl->base.type);
        if (!witnessTable || !interfaceType || !interfaceType->getDeclRef().as<InterfaceDecl>())
            continue;
        auto inheritanceDeclRef =
            createDefaultSubstitutionsIfNeeded(astBuilder, visitor, makeDeclRef(inheritanceDecl));
        auto witness = astBuilder->getDeclaredSubtypeWitness(
            witnessTable->witnessedType,
            interfaceType,
            inheritanceDeclRef);
        for (auto facet : visitor->getShared()->getInheritanceInfo(interfaceType).facets)
        {
            auto stageInterface = facet->origin.declRef.as<InterfaceDecl>();
            if (!state.isExecutableStageInterface(stageInterface.getDecl()))
                continue;
            auto stageWitness =
                visitor->getShared()->tryProjectInterfaceSubtypeWitness(witness, facet->getType());
            auto implementation = _getStageImplementationFromSubtypeWitness(
                astBuilder,
                state,
                stageInterface.getDecl(),
                stageWitness);
            if (implementation.getDecl() != functionDecl)
                continue;
            auto stageKind = state.getStageKind(stageInterface.getDecl());
            state.registerStageImplementation(functionDecl, stageKind);
            return stageKind;
        }
    }
    return StructuralRayTracingStageKind::Count;
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

static StructuralRayTracingAssociatedTypeKind _getStructuralStageContextRequirement(
    StructuralRayTracingStageKind stageKind)
{
    switch (stageKind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
        return StructuralRayTracingAssociatedTypeKind::ClosestHitShaderContext;
    case StructuralRayTracingStageKind::AnyHit:
        return StructuralRayTracingAssociatedTypeKind::AnyHitShaderContext;
    case StructuralRayTracingStageKind::Intersection:
        return StructuralRayTracingAssociatedTypeKind::IntersectionStageContext;
    case StructuralRayTracingStageKind::Miss:
        return StructuralRayTracingAssociatedTypeKind::MissShaderContext;
    case StructuralRayTracingStageKind::Callable:
        return StructuralRayTracingAssociatedTypeKind::CallableShaderContext;
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing stage kind");
    }
}

static bool _populateStructuralEntryPointInfo(
    StructuralRayTracingCheckingState& state,
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
    auto contextRequirement = _getStructuralStageContextRequirement(stageKind);
    outInfo->contextType =
        state.resolveAssociatedType(astBuilder, stageWitness, contextRequirement);
    auto contextWitness =
        state.resolveAssociatedTypeConstraint(astBuilder, stageWitness, contextRequirement);
    if (!outInfo->contextType || !contextWitness)
        return false;

    switch (stageKind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
    case StructuralRayTracingStageKind::AnyHit:
    case StructuralRayTracingStageKind::Intersection:
        {
            outInfo->recordType = state.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::StageRecord);
            if (stageKind != StructuralRayTracingStageKind::Intersection)
            {
                outInfo->payloadType = state.resolveAssociatedType(
                    astBuilder,
                    contextWitness,
                    StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
            }

            auto primitiveWitness = state.resolveAssociatedTypeConstraint(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::HitPrimitive);
            outInfo->hitAttributesType = state.resolveAssociatedType(
                astBuilder,
                primitiveWitness,
                StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes);
            outInfo->primitiveType = state.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::HitPrimitive);
            outInfo->hitAttributesKind = state.getHitAttributesKind(outInfo->primitiveType);
            return (stageKind == StructuralRayTracingStageKind::Intersection ||
                    outInfo->payloadType) &&
                   outInfo->recordType && outInfo->primitiveType && outInfo->hitAttributesType &&
                   outInfo->hitAttributesKind != StructuralRayTracingHitAttributesKind::None;
        }
    case StructuralRayTracingStageKind::Miss:
        {
            outInfo->payloadType = state.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
            outInfo->recordType = state.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::StageRecord);
            return outInfo->payloadType && outInfo->recordType;
        }
    case StructuralRayTracingStageKind::Callable:
        outInfo->callableDataType = state.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::CallableData);
        outInfo->recordType = state.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::StageRecord);
        return outInfo->callableDataType && outInfo->recordType;
    default:
        return false;
    }
}

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
    // explicit
    // `-stage`), and return the specialized witness method that ordinary entry-point lowering can
    // compile. The public entry name remains the struct name and is stored separately on
    // `EntryPoint`.
    *outFoundStruct = false;
    *outInfo = {};
    auto& state = linkage->getStructuralRayTracingCheckingState();
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
        auto kind = state.getStageKind(interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingStageKind::Count)
        {
            // `IIntersectionShader` inherits the non-executable `IIntersectionStage` marker, so
            // inheritance discovery reports both facets for one implementation. Only the
            // executable interface has an `invoke` requirement; do not let the marker's empty
            // lookup erase the implementation found through `IIntersectionShader`.
            if (auto implementation = _getStageImplementationFromSubtypeWitness(
                    visitor.getASTBuilder(),
                    state,
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

    if (!_validateStructuralRayTracingStageStorage(
            state,
            linkage->getASTBuilder(),
            DeclRefType::create(linkage->getASTBuilder(), stageTypeDeclRef),
            stageTypeDeclRef.getLoc(),
            sink))
        return DeclRef<FuncDecl>();

    auto invokeMethod = stageImplementations[int(selectedStage)];
    if (!invokeMethod)
    {
        sink->diagnose(Diagnostics::InternalCompilerError{.location = stageTypeDeclRef.getLoc()});
        return DeclRef<FuncDecl>();
    }

    outInfo->stageInterface = stageInterfaces[int(selectedStage)];
    if (!_populateStructuralEntryPointInfo(
            state,
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
    Metadata,
};

static StructuralRayTracingStageKind _getDirectStageInputKind(
    const StructuralRayTracingCheckingState& state,
    Type* type)
{
    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return state.getStageInputKind(typeDecl);
}

static StructuralRayTracingRuntimeTypeKind _getInterfaceRuntimeTypeKind(
    const StructuralRayTracingCheckingState& state,
    InterfaceDecl* interfaceDecl)
{
    if (state.getStageKind(interfaceDecl) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::Stage;
    if (state.getMetadataKind(interfaceDecl) != StructuralRayTracingMetadataKind::Count)
        return StructuralRayTracingRuntimeTypeKind::Metadata;
    return StructuralRayTracingRuntimeTypeKind::None;
}

// Completed inheritance facets include direct bases, refinements, extension conformances, and
// generic constraints. Type-use checks are deferred during module checking so this query never
// forces an unfinished signature merely to classify its eventual runtime use.
static StructuralRayTracingRuntimeTypeKind _getDirectStructuralRuntimeTypeKind(
    SemanticsVisitor* visitor,
    const StructuralRayTracingCheckingState& state,
    Type* type)
{
    for (auto facet : visitor->getShared()->getInheritanceInfo(type).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = _getInterfaceRuntimeTypeKind(state, interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingRuntimeTypeKind::None)
            return kind;
    }
    return StructuralRayTracingRuntimeTypeKind::None;
}

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

    auto& state = visitor->getLinkage()->getStructuralRayTracingCheckingState();
    if (_getDirectStageInputKind(state, type) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::StageInput;
    auto directKind = _getDirectStructuralRuntimeTypeKind(visitor, state, type);
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
    else if (kind == StructuralRayTracingRuntimeTypeKind::Metadata)
    {
        visitor->getSink()->diagnose(Diagnostics::StructuralRayTracingMetadataRuntimeValue{
            .type = type,
            .location = location});
    }
}

// Returns a checked generic argument when it directly denotes a structural stage, stage-input, or
// metadata type. Type aliases are resolved first because they are alternate names for the same
// semantic type. Callers exempt compiler-provided structural types whose own generic arguments
// form their intended compile-time representation.
static Type* _getDirectStructuralRuntimeGenericArgument(
    SemanticsVisitor* visitor,
    const StructuralRayTracingCheckingState& state,
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
                    state,
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

    if (_getDirectStageInputKind(state, type) != StructuralRayTracingStageKind::Count)
    {
        outKind = StructuralRayTracingRuntimeTypeKind::StageInput;
        return type;
    }

    auto kind = _getDirectStructuralRuntimeTypeKind(visitor, state, type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return nullptr;
    outKind = kind;
    return type;
}

// Returns whether a generic application denotes a compiler-recognized structural container.
// These declarations intentionally consume structural metadata as compile-time schema arguments.
// For example, `RayTracer<MySchema>` and `TraceProgramDescriptor<MySchema>` must remain legal even
// though passing `MySchema` to an arbitrary user generic would let the metadata escape the schema
// language and potentially be materialized after specialization.
static bool _isStructuralRayTracingTypeApplication(Type* type)
{
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return typeDecl && isStructuralRayTracingDeclaration(typeDecl);
}

// Checks an already-typed use after inheritance can be queried safely. The source form only
// determines the diagnostic and the two exceptions: read-only stage inputs and schema-owned
// generic arguments. Every type and substitution was produced by ordinary semantic checking.
static bool _checkStructuralRayTracingTypeUse(
    SemanticsVisitor* visitor,
    const StructuralRayTracingTypeUse& use)
{
    auto& state = visitor->getLinkage()->getStructuralRayTracingCheckingState();
    if (use.kind == StructuralRayTracingTypeUse::Kind::GenericArguments)
    {
        auto applicationType = use.genericApplicationType;
        if (applicationType &&
            (_getDirectStageInputKind(state, applicationType) !=
                 StructuralRayTracingStageKind::Count ||
             _getDirectStructuralRuntimeTypeKind(visitor, state, applicationType) !=
                 StructuralRayTracingRuntimeTypeKind::None ||
             _isStructuralRayTracingTypeApplication(applicationType)))
        {
            return false;
        }

        for (auto argument : use.genericArguments)
        {
            auto invalidKind = StructuralRayTracingRuntimeTypeKind::None;
            auto invalidType = _getDirectStructuralRuntimeGenericArgument(
                visitor,
                state,
                argument.type,
                invalidKind);
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
        _getDirectStageInputKind(state, use.type) != StructuralRayTracingStageKind::Count)
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

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingPropertyType(PropertyDecl* propertyDecl)
{
    StructuralRayTracingTypeUse use;
    use.type = propertyDecl->type.type;
    use.location = propertyDecl->type.exp ? propertyDecl->type.exp->loc : propertyDecl->loc;
    _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

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

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingInvokeResult(InvokeExpr* invoke)
{
    StructuralRayTracingTypeUse use;
    use.type = invoke->type;
    use.location = invoke->loc;
    return _diagnoseOrDeferStructuralRayTracingTypeUse(this, use);
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericArguments(InvokeExpr* invoke)
{
    auto& state = getLinkage()->getStructuralRayTracingCheckingState();
    auto functionDeclRef = as<DeclRefExpr>(invoke->functionExpr);
    if (!functionDeclRef)
        return false;

    // A RayTracer<S> method carries S in its enclosing container's substitutions. That schema
    // is part of the container contract, but a user extension's own helper<T> is still an
    // ordinary generic method: helper<MyStage>() must not let a stage escape through T.
    auto functionDecl = as<FunctionDeclBase>(functionDeclRef->declRef.getDecl());
    if (state.isTraceMethod(functionDecl) || state.isCallShaderMethod(functionDecl))
        return false;

    StructuralRayTracingTypeUse use;
    use.kind = StructuralRayTracingTypeUse::Kind::GenericArguments;
    SubstitutionSet(functionDeclRef->declRef)
        .forEachGenericSubstitution(
            [&](GenericDecl* genericDecl, Val::OperandView<Val> arguments)
            {
                if (isStructuralRayTracingDeclaration(genericDecl->inner))
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

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingEmptyPayloadArgument(InvokeExpr* invoke)
{
    auto functionDeclRefExpr = as<DeclRefExpr>(invoke->functionExpr);
    auto functionDecl = functionDeclRefExpr
                            ? as<FunctionDeclBase>(functionDeclRefExpr->declRef.getDecl())
                            : nullptr;
    if (!functionDecl)
        return false;

    auto& state = getLinkage()->getStructuralRayTracingCheckingState();
    auto traceMethodInfo = state.getTraceMethodInfo(functionDecl);
    if (traceMethodInfo.kind != StructuralRayTracingTraceMethodKind::ExplicitPayload)
        return false;

    auto parameters = functionDecl->getParameters();
    SLANG_RELEASE_ASSERT(
        traceMethodInfo.payloadParameterIndex >= 0 &&
        traceMethodInfo.payloadParameterIndex < parameters.getCount() &&
        traceMethodInfo.payloadParameterIndex < invoke->arguments.getCount());
    auto payloadParameter = parameters[traceMethodInfo.payloadParameterIndex];
    auto payloadType =
        functionDeclRefExpr->declRef.substitute(m_astBuilder, payloadParameter->type.type);

    if (!isSemanticallyEmptyStructuralRayTracingPayload(m_astBuilder, payloadType))
        return false;

    getSink()->diagnose(Diagnostics::StructuralRayTracingEmptyPayloadValue{
        .payloadType = payloadType,
        .location = invoke->arguments[traceMethodInfo.payloadParameterIndex]->loc});
    return true;
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingEmptyPayloadAccess(
    DeclRefExpr* propertyExpr)
{
    auto& state = getLinkage()->getStructuralRayTracingCheckingState();

    auto propertyDeclRef = propertyExpr->declRef.as<PropertyDecl>();
    if (!propertyDeclRef)
        return false;

    // A checked `input.payload` remains a property-valued `MemberExpr`; choosing its `ref`
    // accessor is deliberately deferred until storage lowering. Authenticate the property through
    // that annotated accessor, but use the checked member expression's specialized type as the
    // semantic source of truth for `Context.Payload`.
    bool isPayloadProperty = false;
    for (auto accessorDeclRef :
         getMembersOfType<AccessorDecl>(m_astBuilder, propertyDeclRef.as<ContainerDecl>()))
    {
        if (state.isPayloadStageInputAccessor(accessorDeclRef.getDecl()))
        {
            isPayloadProperty = true;
            break;
        }
    }
    if (!isPayloadProperty ||
        !isSemanticallyEmptyStructuralRayTracingPayload(m_astBuilder, propertyExpr->type.type))
    {
        return false;
    }

    getSink()->diagnose(Diagnostics::StructuralRayTracingEmptyPayloadValue{
        .payloadType = propertyExpr->type.type,
        .location = propertyExpr->loc});
    return true;
}

bool SemanticsVisitor::diagnoseDirectStructuralRayTracingStageInvoke(
    InvokeExpr* invoke,
    FunctionDeclBase* functionDecl)
{
    auto& state = getLinkage()->getStructuralRayTracingCheckingState();
    auto stageKind = state.getStageKind(functionDecl);
    if (stageKind == StructuralRayTracingStageKind::Count)
        stageKind = _findStageImplementationFromParentConformance(this, state, functionDecl);
    if (stageKind == StructuralRayTracingStageKind::Count)
        return false;

    getSink()->diagnose(
        Diagnostics::DirectStructuralRayTracingStageInvoke{.location = invoke->functionExpr->loc});
    return true;
}

} // namespace Slang
