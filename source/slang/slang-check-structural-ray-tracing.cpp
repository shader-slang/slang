#include "slang-check-impl.h"
#include "slang-lookup.h"
#include "slang-session.h"
#include "slang-syntax.h"

namespace Slang
{

static Stage _getNativeStage(StructuralRayTracingStageKind kind);
static StructuralRayTracingStageKind _getStructuralStage(Stage stage);
static StructuralRayTracingStageKind _getDirectStageInputKind(
    const StructuralRayTracingDeclRegistry& registry,
    Type* type);

// Structural runtime-type checks must only inspect a module that can actually name the trusted
// API. The registry is linkage-wide, so merely importing `slang.raytracing` in an earlier module
// must not make later unrelated modules query inheritance while their generic declarations are
// still being checked. Such eager queries can cache an incomplete conformance result.
static bool _isStructuralRayTracingVisible(
    Module* module,
    const StructuralRayTracingDeclRegistry& registry)
{
    if (!registry.isInitialized())
        return false;

    if (!module)
        return true;

    for (auto dependency : module->getModuleDependencies())
    {
        if (registry.isTrustedModule(dependency))
            return true;
    }
    return false;
}

static bool _isStructuralRayTracingVisible(
    SemanticsVisitor* visitor,
    const StructuralRayTracingDeclRegistry& registry)
{
    return _isStructuralRayTracingVisible(visitor->getShared()->getModule(), registry);
}

static FunctionDeclBase* _getStageImplementation(
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind stageKind,
    WitnessTable* witnessTable)
{
    auto invokeRequirement = registry.getStageInvokeRequirement(stageKind);
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
    StructuralRayTracingDeclRegistry& registry,
    AggTypeDecl* stageType,
    DiagnosticSink* sink)
{
    const bool shouldDiagnose = registry.beginStageRepresentationDeclarationCheck(stageType);
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
// compiler-created receiver valid for closed schemas, open-section entries, and standalone stages.
// The registry suppresses duplicate diagnostics per inspected declaration because conformance
// checking may publish the same completed witness more than once, and several stages may share one
// stateful base struct.
static bool _validateStructuralRayTracingStageStorage(
    StructuralRayTracingDeclRegistry& registry,
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

    // A user-defined interface may inherit a stage contract to act as an open-section tag. It is
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
                ? registry.beginStageRepresentationDeclarationCheck(stageDeclRef.getDecl())
                : registry.beginStageRepresentationTypeCheck(witnessedStageType);
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
        if (!_validateStructuralRayTracingStageFields(registry, currentType.getDecl(), sink))
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
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return;

    Decl* otherDecl = nullptr;
    if (!registry.registerAPIUse(module, family, decl, &otherDecl))
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
    SourceLoc callLoc,
    DiagnosticSink* sink)
{
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    registry.registerFunctionCall(caller, callee, callLoc);
    if (!registry.isInitialized() || !caller || !callee)
        return;

    auto callerModule = getModule(caller);
    if (!callerModule || registry.isTrustedModule(callerModule))
        return;

    if (registry.isTraceMethod(callee) || registry.isCallShaderMethod(callee))
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
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    auto stageKind = registry.getStageKind(superInterfaceDeclRef.getDecl());
    auto metadataKind = registry.getMetadataKind(superInterfaceDeclRef.getDecl());
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
        registry,
        getASTBuilder(),
        witnessedType,
        conformanceLoc,
        getSink());

    registry.registerStageImplementation(
        _getStageImplementation(registry, stageKind, witnessTable),
        stageKind);
}

struct _StructuralRayTracingSchemaSectionInfo
{
    StructuralRayTracingSectionKind kind = StructuralRayTracingSectionKind::Count;
    const char* name = nullptr;
    const char* entryInterfaceName = nullptr;
};

static bool _tryGetStructuralRayTracingSchemaSectionInfo(
    const StructuralRayTracingDeclRegistry& registry,
    AssocTypeDecl* requirement,
    _StructuralRayTracingSchemaSectionInfo& outInfo)
{
    if (requirement == registry.getAssociatedTypeRequirement(
                           StructuralRayTracingAssociatedTypeKind::ProgramHitGroups))
    {
        outInfo = {StructuralRayTracingSectionKind::HitGroups, "hit-group", "IHitGroup"};
        return true;
    }
    if (requirement == registry.getAssociatedTypeRequirement(
                           StructuralRayTracingAssociatedTypeKind::ProgramMissShaders))
    {
        outInfo = {StructuralRayTracingSectionKind::MissShaders, "miss-shader", "IMissShader"};
        return true;
    }
    if (requirement == registry.getAssociatedTypeRequirement(
                           StructuralRayTracingAssociatedTypeKind::ProgramCallableShaders))
    {
        outInfo = {
            StructuralRayTracingSectionKind::CallableShaders,
            "callable-shader",
            "ICallableShader"};
        return true;
    }
    return false;
}

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingOpenSectionTag(
    Type* entryListType,
    AssocTypeDecl* associatedTypeRequirement,
    Decl* satisfyingDecl)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return;

    _StructuralRayTracingSchemaSectionInfo section;
    if (!_tryGetStructuralRayTracingSchemaSectionInfo(registry, associatedTypeRequirement, section))
    {
        return;
    }

    SLANG_RELEASE_ASSERT(entryListType && satisfyingDecl);
    StructuralRayTracingOpenSectionInfo openSection;
    if (!registry.tryGetOpenSectionInfo(
            getASTBuilder(),
            entryListType->getCanonicalType(),
            section.kind,
            openSection))
    {
        return;
    }

    auto tagType = openSection.tagType->getCanonicalType();
    auto tagDeclRefType = as<DeclRefType>(tagType);
    if (as<ErrorType>(tagType) ||
        (tagDeclRefType && tagDeclRefType->getDeclRef().as<InterfaceDecl>()))
        return;

    // `OpenHitGroups<ConcreteHit>` satisfies the ordinary `Tag : IHitGroup` generic constraint,
    // but it cannot discover other conformers because a concrete type is not an interface tag.
    // An unresolved generic parameter has the same problem: `T : IHitGroup` admits both interface
    // tags and concrete hit groups, and Slang has no interface-kind constraint that proves every
    // specialization is a valid tag. Diagnose the declaration while it still provides a precise
    // source location instead of accepting a contract that specialization does not re-check.
    getSink()->diagnose(Diagnostics::StructuralRayTracingOpenTagNotEntryInterface{
        .section = section.name,
        .tag = tagType,
        .entryInterface = section.entryInterfaceName,
        .location = satisfyingDecl->loc});
}

void SemanticsVisitor::diagnoseDuplicateStructuralRayTracingSchemaEntries(
    Type* entryListType,
    AssocTypeDecl* associatedTypeRequirement,
    Type* schemaType,
    Decl* satisfyingDecl)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return;

    _StructuralRayTracingSchemaSectionInfo section;
    if (!_tryGetStructuralRayTracingSchemaSectionInfo(registry, associatedTypeRequirement, section))
        return;

    SLANG_RELEASE_ASSERT(entryListType && schemaType && satisfyingDecl);

    // Consider this example:
    //
    //     typealias HitGroups = OpenHitGroups<IHitTag, OpaqueHit, OpaqueHit>;
    //
    // The type pack in this checked associated-type witness is the source declaration's exact
    // list of known implementations. Each implementation receives one dense function index; it
    // does not stand for a physical SBT record. A host can therefore reuse `OpaqueHit` in any
    // number of records, but listing it twice here would assign one implementation two indices and
    // make reflection ambiguous. Reject that source contract before lowering, because the IR and
    // reflection representations intentionally rely on unique entries.
    //
    // This check concerns only repeated explicit pack elements. If a listed open-section entry
    // also conforms to the section tag, link completion still forms a union and retains that type
    // once. Canonical AST identity makes aliases name the same entry without reconstructing or
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
            .section = section.name,
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

static void _diagnoseInvalidCallableDispatchStages(
    StructuralRayTracingDeclRegistry& registry,
    ContainerDecl* containerDecl,
    DiagnosticSink* sink)
{
    // Callable dispatch is legal from some ray-tracing stages but not from any-hit or
    // intersection. The source operation may be hidden behind ordinary helpers, so query the
    // registry's checked call graph from each structural stage implementation instead of checking
    // only the stage's `invoke` body.
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto innerMember = member;
        if (auto genericDecl = as<GenericDecl>(innerMember))
            innerMember = genericDecl->inner;

        if (auto functionDecl = as<FunctionDeclBase>(innerMember))
        {
            auto stageKind = registry.getStageKind(functionDecl);
            if (stageKind == StructuralRayTracingStageKind::AnyHit ||
                stageKind == StructuralRayTracingStageKind::Intersection)
            {
                SourceLoc callLoc;
                if (registry.findReachableCallShader(functionDecl, callLoc))
                {
                    auto stageName = stageKind == StructuralRayTracingStageKind::AnyHit
                                         ? "any-hit"
                                         : "intersection";
                    sink->diagnose(Diagnostics::StructuralRayTracingCallableStageMismatch{
                        .stage = stageName,
                        .location = callLoc});
                }
            }
        }

        if (auto childContainer = as<ContainerDecl>(innerMember))
            _diagnoseInvalidCallableDispatchStages(registry, childContainer, sink);
    }
}

static void _diagnoseInvalidStructuralStageCapabilities(
    StructuralRayTracingDeclRegistry& registry,
    ContainerDecl* containerDecl,
    DiagnosticSink* sink)
{
    // A structural stage has no `[shader]` attribute from which ordinary capability checking can
    // obtain its native stage. Compare the completed `invoke` requirements with the stage implied
    // by their trusted interface so helpers that use stage-restricted intrinsics receive the same
    // validation as a legacy entry point.
    for (auto member : containerDecl->getDirectMemberDecls())
    {
        auto innerMember = member;
        if (auto genericDecl = as<GenericDecl>(innerMember))
            innerMember = genericDecl->inner;

        if (auto functionDecl = as<FunctionDeclBase>(innerMember))
        {
            auto stageKind = registry.getStageKind(functionDecl);
            auto stage = _getNativeStage(stageKind);
            auto capabilities = functionDecl->inferredCapabilityRequirements;
            SourceLoc callShaderLoc;
            auto hasSpecificCallableDiagnostic =
                (stageKind == StructuralRayTracingStageKind::AnyHit ||
                 stageKind == StructuralRayTracingStageKind::Intersection) &&
                registry.findReachableCallShader(functionDecl, callShaderLoc);
            if (!hasSpecificCallableDiagnostic && stage != Stage::Unknown && capabilities &&
                capabilities->isIncompatibleWith(getAtomFromStage(stage)))
            {
                sink->diagnose(Diagnostics::DeclHasDependenciesNotCompatibleOnStage{
                    .stage = getStageName(stage),
                    .decl = functionDecl});
            }
        }

        if (auto childContainer = as<ContainerDecl>(innerMember))
        {
            _diagnoseInvalidStructuralStageCapabilities(registry, childContainer, sink);
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
// the structural-stage registry. Its checked `EntryPointAttribute` is nevertheless an explicit
// any-hit contract and must win over the fallback that infers a stage from an otherwise-unannotated
// helper's first stage-input parameter. Preserve the native `Stage` here: mapping a compute or miss
// entry point to `StructuralRayTracingStageKind::Count` would make a known mismatched stage look
// the same as an unconstrained helper.
static Stage _getRequiredStageForStructuralInput(
    StructuralRayTracingDeclRegistry& registry,
    FunctionDeclBase* functionDecl)
{
    auto stageKind = registry.getStageKind(functionDecl);
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
    StructuralRayTracingDeclRegistry& registry,
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
            auto functionStage = _getRequiredStageForStructuralInput(registry, functionDecl);
            for (auto parameter : functionDecl->getParameters())
            {
                auto inputStage = _getDirectStageInputKind(registry, parameter->type.type);
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
            _diagnoseInvalidStructuralStageInputParameters(linkage, registry, childContainer, sink);
    }
}

void diagnoseMixedRayTracingAPIsInModule(Linkage* linkage, Module* module, DiagnosticSink* sink)
{
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return;
    _registerAttributedLegacyEntryPoints(linkage, module, module->getModuleDecl(), sink);
    _diagnoseInvalidCallableDispatchStages(registry, module->getModuleDecl(), sink);
    _diagnoseInvalidStructuralStageCapabilities(registry, module->getModuleDecl(), sink);
    _diagnoseInvalidStructuralStageInputParameters(
        linkage,
        registry,
        module->getModuleDecl(),
        sink);
}

static StructuralRayTracingStageKind _findStageImplementationFromParentConformance(
    StructuralRayTracingDeclRegistry& registry,
    FunctionDeclBase* functionDecl)
{
    // Conformance checking normally registers an `invoke` implementation before its body is
    // checked. Deserialized declarations and some demand-driven paths can ask about the function
    // first, so recover the same identity from the parent's completed witness table rather than
    // inferring a stage from the method name.
    Decl* parent = functionDecl->parentDecl;
    while (auto genericDecl = as<GenericDecl>(parent))
        parent = genericDecl->parentDecl;
    auto container = as<ContainerDecl>(parent);
    if (!container)
        return StructuralRayTracingStageKind::Count;

    for (auto inheritanceDecl : container->getDirectMemberDeclsOfType<InheritanceDecl>())
    {
        auto interfaceType = as<DeclRefType>(inheritanceDecl->base.type);
        auto interfaceDeclRef = interfaceType ? interfaceType->getDeclRef().as<InterfaceDecl>()
                                              : DeclRef<InterfaceDecl>();
        auto stageKind = registry.getStageKind(interfaceDeclRef.getDecl());
        if (stageKind == StructuralRayTracingStageKind::Count)
            continue;

        auto implementation =
            _getStageImplementation(registry, stageKind, inheritanceDecl->witnessTable);
        if (implementation == functionDecl)
        {
            registry.registerStageImplementation(functionDecl, stageKind);
            return stageKind;
        }
    }
    return StructuralRayTracingStageKind::Count;
}

static DeclRef<FuncDecl> _getStageImplementationFromSubtypeWitness(
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingStageKind stageKind,
    SubtypeWitness* witness)
{
    witness = witness ? as<SubtypeWitness>(witness->resolve()) : nullptr;
    auto invokeRequirement = registry.getStageInvokeRequirement(stageKind);
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
    StructuralRayTracingDeclRegistry& registry,
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
        registry.resolveAssociatedType(astBuilder, stageWitness, contextRequirement);
    auto contextWitness =
        registry.resolveAssociatedTypeConstraint(astBuilder, stageWitness, contextRequirement);
    if (!outInfo->contextType || !contextWitness)
        return false;

    switch (stageKind)
    {
    case StructuralRayTracingStageKind::ClosestHit:
    case StructuralRayTracingStageKind::AnyHit:
    case StructuralRayTracingStageKind::Intersection:
        {
            outInfo->recordType = registry.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::StageRecord);
            if (stageKind != StructuralRayTracingStageKind::Intersection)
            {
                outInfo->payloadType = registry.resolveAssociatedType(
                    astBuilder,
                    contextWitness,
                    StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
            }

            auto primitiveWitness = registry.resolveAssociatedTypeConstraint(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::HitPrimitive);
            outInfo->hitAttributesType = registry.resolveAssociatedType(
                astBuilder,
                primitiveWitness,
                StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes);
            outInfo->primitiveType = registry.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::HitPrimitive);
            outInfo->hitAttributesKind = registry.getHitAttributesKind(outInfo->primitiveType);
            return (stageKind == StructuralRayTracingStageKind::Intersection ||
                    outInfo->payloadType) &&
                   outInfo->recordType && outInfo->primitiveType && outInfo->hitAttributesType &&
                   outInfo->hitAttributesKind != StructuralRayTracingHitAttributesKind::None;
        }
    case StructuralRayTracingStageKind::Miss:
        {
            outInfo->payloadType = registry.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
            outInfo->recordType = registry.resolveAssociatedType(
                astBuilder,
                contextWitness,
                StructuralRayTracingAssociatedTypeKind::StageRecord);
            return outInfo->payloadType && outInfo->recordType;
        }
    case StructuralRayTracingStageKind::Callable:
        outInfo->callableDataType = registry.resolveAssociatedType(
            astBuilder,
            contextWitness,
            StructuralRayTracingAssociatedTypeKind::CallableData);
        outInfo->recordType = registry.resolveAssociatedType(
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
    // struct in the requested module, select exactly one trusted stage conformance (or the explicit
    // `-stage`), and return the specialized witness method that ordinary entry-point lowering can
    // compile. The public entry name remains the struct name and is stored separately on
    // `EntryPoint`.
    *outFoundStruct = false;
    *outInfo = {};
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    // The registry is linkage-wide because imported modules share one compiler session, but entry
    // lookup is local to the requested source module. A different translation unit may have loaded
    // `slang.raytracing`; that must not reinterpret an ordinary struct in this module as a failed
    // structural stage unless this module can actually name the trusted API.
    if (!_isStructuralRayTracingVisible(module, registry))
        return DeclRef<FuncDecl>();

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
    auto stageType = DeclRefType::create(linkage->getASTBuilder(), stageTypeDeclRef);
    outInfo->stageType = stageType;
    for (auto facet : visitor.getShared()->getInheritanceInfo(stageType).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = registry.getStageKind(interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingStageKind::Count)
        {
            // `IIntersectionShader` inherits the non-executable `IIntersectionStage` marker, so
            // inheritance discovery reports both facets for one implementation. Only the
            // executable interface has an `invoke` requirement; do not let the marker's empty
            // lookup erase the implementation found through `IIntersectionShader`.
            if (auto implementation = _getStageImplementationFromSubtypeWitness(
                    visitor.getASTBuilder(),
                    registry,
                    kind,
                    facet->subtypeWitness))
            {
                stageImplementations[int(kind)] = implementation;
                stageWitnesses[int(kind)] = facet->subtypeWitness;
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

    if (!_validateStructuralRayTracingStageStorage(
            registry,
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

    if (!_populateStructuralEntryPointInfo(
            registry,
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
    const StructuralRayTracingDeclRegistry& registry,
    Type* type)
{
    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return registry.getStageInputKind(typeDecl);
}

static StructuralRayTracingRuntimeTypeKind _getInterfaceRuntimeTypeKind(
    const StructuralRayTracingDeclRegistry& registry,
    InterfaceDecl* interfaceDecl)
{
    if (registry.getStageKind(interfaceDecl) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::Stage;
    if (registry.getMetadataKind(interfaceDecl) != StructuralRayTracingMetadataKind::Count)
        return StructuralRayTracingRuntimeTypeKind::Metadata;
    return StructuralRayTracingRuntimeTypeKind::None;
}

static StructuralRayTracingRuntimeTypeKind _getDirectStructuralRuntimeTypeKind(
    SemanticsVisitor* visitor,
    const StructuralRayTracingDeclRegistry& registry,
    Type* type)
{
    if (auto declRefType = as<DeclRefType>(type))
    {
        if (auto typeDecl = declRefType->getDeclRef().as<AggTypeDecl>())
        {
            auto kind =
                _getInterfaceRuntimeTypeKind(registry, as<InterfaceDecl>(typeDecl.getDecl()));
            if (kind != StructuralRayTracingRuntimeTypeKind::None)
                return kind;

            // Consider a parameter declared as `TestHitGroup group`, where `TestHitGroup` directly
            // inherits `IHitGroup`. Parameter signatures can be checked before the compiler has
            // built `TestHitGroup`'s conformance witness. Recognizing the declared base only needs
            // that base's type, and forcing the complete witness here can diagnose otherwise-valid
            // associated-type constraints before dependent stage conformances have populated their
            // witness tables. Inspect direct bases first so this runtime restriction is independent
            // of that order.
            for (auto inheritanceDecl :
                 typeDecl.getDecl()->getDirectMemberDeclsOfType<InheritanceDecl>())
            {
                visitor->ensureDecl(inheritanceDecl, DeclCheckState::CanUseBaseOfInheritanceDecl);
                auto baseType = as<DeclRefType>(inheritanceDecl->base.type);
                auto baseInterface = baseType ? baseType->getDeclRef().as<InterfaceDecl>()
                                              : DeclRef<InterfaceDecl>();
                kind = _getInterfaceRuntimeTypeKind(registry, baseInterface.getDecl());
                if (kind != StructuralRayTracingRuntimeTypeKind::None)
                    return kind;
            }
        }
    }

    for (auto facet : visitor->getShared()->getInheritanceInfo(type).facets)
    {
        auto interfaceDeclRef = facet->origin.declRef.as<InterfaceDecl>();
        auto kind = _getInterfaceRuntimeTypeKind(registry, interfaceDeclRef.getDecl());
        if (kind != StructuralRayTracingRuntimeTypeKind::None)
            return kind;
    }
    return StructuralRayTracingRuntimeTypeKind::None;
}

static StructuralRayTracingRuntimeTypeKind _findStructuralRuntimeType(
    SemanticsVisitor* visitor,
    Type* type,
    HashSet<Type*>& seenTypes)
{
    if (!type || as<ErrorType>(type))
        return StructuralRayTracingRuntimeTypeKind::None;

    while (auto modifiedType = as<ModifiedType>(type))
        type = modifiedType->getBase();

    auto& registry = visitor->getLinkage()->getStructuralRayTracingDeclRegistry();
    if (!_isStructuralRayTracingVisible(visitor, registry))
        return StructuralRayTracingRuntimeTypeKind::None;
    if (_getDirectStageInputKind(registry, type) != StructuralRayTracingStageKind::Count)
        return StructuralRayTracingRuntimeTypeKind::StageInput;
    auto directKind = _getDirectStructuralRuntimeTypeKind(visitor, registry, type);
    if (directKind != StructuralRayTracingRuntimeTypeKind::None)
        return directKind;

    if (auto typePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < typePack->getTypeCount(); ++i)
        {
            auto kind = _findStructuralRuntimeType(visitor, typePack->getElementType(i), seenTypes);
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
                result = _findStructuralRuntimeType(visitor, fieldType, seenTypes);
                if (result != StructuralRayTracingRuntimeTypeKind::None)
                    break;
            }
            seenTypes.remove(canonicalStructType);
            if (result != StructuralRayTracingRuntimeTypeKind::None)
                return result;
        }
    }

    if (auto arrayType = as<ArrayExpressionType>(type))
        return _findStructuralRuntimeType(visitor, arrayType->getElementType(), seenTypes);
    if (auto optionalType = as<OptionalType>(type))
        return _findStructuralRuntimeType(visitor, optionalType->getValueType(), seenTypes);
    if (auto pointerType = as<PtrTypeBase>(type))
        return _findStructuralRuntimeType(visitor, pointerType->getValueType(), seenTypes);
    if (auto tupleType = as<TupleType>(type))
    {
        for (Index i = 0; i < tupleType->getMemberCount(); ++i)
        {
            auto kind = _findStructuralRuntimeType(visitor, tupleType->getMember(i), seenTypes);
            if (kind != StructuralRayTracingRuntimeTypeKind::None)
                return kind;
        }
    }
    return StructuralRayTracingRuntimeTypeKind::None;
}

static StructuralRayTracingRuntimeTypeKind _findStructuralRuntimeType(
    SemanticsVisitor* visitor,
    Type* type)
{
    if (!type || as<ErrorType>(type))
        return StructuralRayTracingRuntimeTypeKind::None;
    HashSet<Type*> seenTypes;
    return _findStructuralRuntimeType(visitor, type, seenTypes);
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

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingVariableType(VarDeclBase* varDecl)
{
    auto type = varDecl->type.type;
    auto kind = _findStructuralRuntimeType(this, type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return;

    auto paramDecl = as<ParamDecl>(varDecl);
    auto isReadOnlyValueParameter = paramDecl && !paramDecl->hasModifier<OutModifier>() &&
                                    !paramDecl->hasModifier<RefModifier>() &&
                                    !paramDecl->hasModifier<BorrowModifier>() &&
                                    !paramDecl->hasModifier<HLSLPayloadModifier>();
    if (kind == StructuralRayTracingRuntimeTypeKind::StageInput && isReadOnlyValueParameter &&
        _getDirectStageInputKind(getLinkage()->getStructuralRayTracingDeclRegistry(), type) !=
            StructuralRayTracingStageKind::Count)
    {
        return;
    }

    _diagnoseInvalidStructuralRayTracingRuntimeType(this, kind, type, varDecl->loc);
}

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingCallableResult(CallableDecl* callableDecl)
{
    if (as<ConstructorDecl>(callableDecl))
        return;
    auto type = callableDecl->returnType.type;
    auto kind = _findStructuralRuntimeType(this, type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return;

    auto location =
        callableDecl->returnType.exp ? callableDecl->returnType.exp->loc : callableDecl->loc;
    _diagnoseInvalidStructuralRayTracingRuntimeType(this, kind, type, location);
}

void SemanticsVisitor::diagnoseInvalidStructuralRayTracingPropertyType(PropertyDecl* propertyDecl)
{
    auto type = propertyDecl->type.type;
    _diagnoseInvalidStructuralRayTracingRuntimeType(
        this,
        _findStructuralRuntimeType(this, type),
        type,
        propertyDecl->type.exp ? propertyDecl->type.exp->loc : propertyDecl->loc);
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingConstruction(InvokeExpr* invoke)
{
    auto typeType = as<TypeType>(invoke->functionExpr->type);
    if (!typeType)
        return false;
    auto type = typeType->getType();
    if (_findStructuralRuntimeType(this, type) == StructuralRayTracingRuntimeTypeKind::None)
        return false;

    getSink()->diagnose(Diagnostics::StructuralRayTracingTypeConstruction{
        .type = type,
        .location = invoke->functionExpr->loc});
    return true;
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingInvokeResult(InvokeExpr* invoke)
{
    auto kind = _findStructuralRuntimeType(this, invoke->type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return false;

    _diagnoseInvalidStructuralRayTracingRuntimeType(this, kind, invoke->type, invoke->loc);
    return true;
}

// Returns a checked generic argument when it directly denotes a structural stage, stage-input, or
// metadata type. Type aliases are resolved first because they are alternate names for the same
// semantic type. Callers exempt compiler-provided structural types whose own generic arguments
// form their intended compile-time representation.
static Type* _getDirectStructuralRuntimeGenericArgument(
    SemanticsVisitor* visitor,
    const StructuralRayTracingDeclRegistry& registry,
    Val* argument,
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
                    registry,
                    typePack->getElementType(i),
                    outKind))
            {
                return invalidType;
            }
        }
        return nullptr;
    }

    auto type = argument ? as<Type>(argument->resolve()) : nullptr;
    if (!type)
        return nullptr;

    if (_getDirectStageInputKind(registry, type) != StructuralRayTracingStageKind::Count)
    {
        outKind = StructuralRayTracingRuntimeTypeKind::StageInput;
        return type;
    }

    auto kind = _getDirectStructuralRuntimeTypeKind(visitor, registry, type);
    if (kind == StructuralRayTracingRuntimeTypeKind::None)
        return nullptr;
    outKind = kind;
    return type;
}

// Returns whether a generic application denotes a type declared by the trusted ray-tracing
// module. Those types intentionally consume structural metadata as compile-time schema arguments.
// For example, `RayTracer<MySchema>` and `TraceProgramDescriptor<MySchema>` must remain legal even
// though passing `MySchema` to an arbitrary user generic would let the metadata escape the schema
// language and potentially be materialized after specialization.
static bool _isTrustedStructuralRayTracingTypeApplication(
    const StructuralRayTracingDeclRegistry& registry,
    Type* type)
{
    auto declRefType = as<DeclRefType>(type);
    auto typeDecl = declRefType ? declRefType->getDeclRef().as<AggTypeDecl>().getDecl() : nullptr;
    return typeDecl && registry.isTrustedModule(getModule(typeDecl));
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericArguments(InvokeExpr* invoke)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    if (!_isStructuralRayTracingVisible(this, registry))
        return false;

    auto functionDeclRef = as<DeclRefExpr>(invoke->functionExpr);
    if (!functionDeclRef)
        return false;

    // Methods on compiler-provided schema containers necessarily carry the enclosing schema in
    // their substitution set. That is the trusted contract boundary, not an escape into an
    // arbitrary user generic. Their ordinary value parameters are still checked separately, so a
    // structural type cannot be supplied as payload or callable data through this exemption.
    auto functionDecl = as<FunctionDeclBase>(functionDeclRef->declRef.getDecl());
    if (functionDecl && registry.isTrustedModule(getModule(functionDecl)))
        return false;

    Type* invalidType = nullptr;
    auto invalidKind = StructuralRayTracingRuntimeTypeKind::None;
    SubstitutionSet(functionDeclRef->declRef)
        .forEachSubstitutionArg(
            [&](Val* argument)
            {
                if (invalidType)
                    return;
                invalidType = _getDirectStructuralRuntimeGenericArgument(
                    this,
                    registry,
                    argument,
                    invalidKind);
            });
    if (!invalidType)
        return false;

    _diagnoseInvalidStructuralRayTracingRuntimeType(
        this,
        invalidKind,
        invalidType,
        invoke->functionExpr->loc);
    return true;
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericTypeApplication(
    GenericAppExpr* genericApplication,
    Expr* checkedResult)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    if (!_isStructuralRayTracingVisible(this, registry))
        return false;

    // Compiler-provided structural types own their generic arguments. For example,
    // `ClosestHitInput<C>`, `NoAnyHit<C>`, and `MissShaderList<MyMiss>` must name their context or
    // stage types so the contract and schema can be checked. Exempt those applications themselves;
    // an ordinary outer container such as `Phantom<ClosestHitInput<C>>` is still diagnosed when its
    // direct argument is checked.
    auto applicationTypeType = checkedResult ? as<TypeType>(checkedResult->type) : nullptr;
    auto applicationType = applicationTypeType ? applicationTypeType->getType() : nullptr;
    if (applicationType &&
        (_getDirectStageInputKind(registry, applicationType) !=
             StructuralRayTracingStageKind::Count ||
         _getDirectStructuralRuntimeTypeKind(this, registry, applicationType) !=
             StructuralRayTracingRuntimeTypeKind::None ||
         _isTrustedStructuralRayTracingTypeApplication(registry, applicationType)))
    {
        return false;
    }

    // Consider these declarations:
    //
    //     struct Phantom<T> {}
    //     typealias Bad = Phantom<rt::ClosestHitInput<C>>;
    //     ConstantBuffer<rt::ClosestHitInput<C>> buffer;
    //
    // Neither `Phantom` nor a resource wrapper physically stores a field that the ordinary
    // runtime-type walk can inspect. The checked generic argument is the semantic source of truth,
    // however: its `TypeType` already carries the resolved type, including type aliases. Nested
    // applications are checked inside-out, so each one validates only its own direct arguments
    // instead of walking arbitrary substitution graphs to rediscover a producer error. Structural
    // metadata follows the same rule: only the trusted schema containers above may consume it.
    for (auto argument : genericApplication->arguments)
    {
        auto argumentType = as<TypeType>(argument->type);
        if (!argumentType)
            continue;

        auto invalidKind = StructuralRayTracingRuntimeTypeKind::None;
        auto invalidType = _getDirectStructuralRuntimeGenericArgument(
            this,
            registry,
            argumentType->getType(),
            invalidKind);
        if (!invalidType)
            continue;

        _diagnoseInvalidStructuralRayTracingRuntimeType(
            this,
            invalidKind,
            invalidType,
            argument->loc);
        return true;
    }
    return false;
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingEmptyPayloadArgument(InvokeExpr* invoke)
{
    auto functionDeclRefExpr = as<DeclRefExpr>(invoke->functionExpr);
    auto functionDecl = functionDeclRefExpr
                            ? as<FunctionDeclBase>(functionDeclRefExpr->declRef.getDecl())
                            : nullptr;
    if (!functionDecl)
        return false;

    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    auto traceMethodInfo = registry.getTraceMethodInfo(functionDecl);
    if (!traceMethodInfo ||
        traceMethodInfo->kind != StructuralRayTracingTraceMethodKind::ExplicitPayload)
        return false;

    auto parameters = functionDecl->getParameters();
    SLANG_RELEASE_ASSERT(
        traceMethodInfo->payloadParameterIndex >= 0 &&
        traceMethodInfo->payloadParameterIndex < parameters.getCount() &&
        traceMethodInfo->payloadParameterIndex < invoke->arguments.getCount());
    auto payloadParameter = parameters[traceMethodInfo->payloadParameterIndex];
    auto payloadType =
        functionDeclRefExpr->declRef.substitute(m_astBuilder, payloadParameter->type.type);

    if (!isSemanticallyEmptyStructuralRayTracingPayload(m_astBuilder, payloadType))
        return false;

    getSink()->diagnose(Diagnostics::StructuralRayTracingEmptyPayloadValue{
        .payloadType = payloadType,
        .location = invoke->arguments[traceMethodInfo->payloadParameterIndex]->loc});
    return true;
}

bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingEmptyPayloadAccess(
    DeclRefExpr* propertyExpr)
{
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return false;

    auto propertyDeclRef = propertyExpr->declRef.as<PropertyDecl>();
    if (!propertyDeclRef)
        return false;

    // A checked `input.payload` remains a property-valued `MemberExpr`; choosing its `ref`
    // accessor is deliberately deferred until storage lowering. Authenticate the property through
    // that trusted accessor, but use the checked member expression's specialized type as the
    // semantic source of truth for `Context.Payload`.
    bool isPayloadProperty = false;
    for (auto accessorDeclRef :
         getMembersOfType<AccessorDecl>(m_astBuilder, propertyDeclRef.as<ContainerDecl>()))
    {
        if (registry.isPayloadStageInputAccessor(accessorDeclRef.getDecl()))
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
    auto& registry = getLinkage()->getStructuralRayTracingDeclRegistry();
    auto stageKind = registry.getStageKind(functionDecl);
    if (stageKind == StructuralRayTracingStageKind::Count)
        stageKind = _findStageImplementationFromParentConformance(registry, functionDecl);
    if (stageKind == StructuralRayTracingStageKind::Count)
        return false;

    getSink()->diagnose(
        Diagnostics::DirectStructuralRayTracingStageInvoke{.location = invoke->functionExpr->loc});
    return true;
}

} // namespace Slang
