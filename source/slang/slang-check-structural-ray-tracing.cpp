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
// stage types and inputs, stage selection, schema entry uniqueness, API mixing, and the
// boundary between logical shaders and native entry points. Each rule below states its example
// and the representation or API contract it preserves.
//
// Schemas, lists, and hit groups are ordinary values. Empty payloads are ordinary payloads.
// Concrete stage structs are ordinary values too; input generic uses retain the restriction below.

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

// Requires a concrete stage implementation to be a user-defined struct. Its invoke method is
// static, so ordinary fields and instances do not participate in stage dispatch. For example,
// `struct Miss : rt::IMissShader { uint state; ... }` is allowed: selecting Miss as a stage calls
// its static invoke function without constructing Miss or supplying a value for state.
static void _checkStructuralRayTracingStageImplementationType(
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

// Only the library's stage interfaces and input views have restricted value uses. A user struct
// implementing a stage interface is an ordinary value; identifying it never requires inheritance.
enum class RestrictedRayTracingTypeKind
{
    None,
    StageInterface,
    StageInput,
};

static RestrictedRayTracingTypeKind _getDirectRestrictedRayTracingTypeKind(Type* type)
{
    type = unwrapModifiedType(type);
    if (_getDirectStageInputKind(type) != StructuralRayTracingStageKind::Count)
        return RestrictedRayTracingTypeKind::StageInput;
    if (as<RayTracingStageInterfaceType>(type))
        return RestrictedRayTracingTypeKind::StageInterface;
    return RestrictedRayTracingTypeKind::None;
}

// Checks the supplied type and built-in containers without inspecting ordinary struct fields.
// Consider `struct Box<T> { T value; }`: the field is checked when Box is checked, and writing
// Box<MissInput<C>> is rejected by the generic-argument check. Inspecting every ordinary struct
// here would needlessly force unfinished field declarations, e.g. Node's own `Ptr<Node> next`.
//
// Arrays and pointers can name an input without a generic application in source, so follow their
// already-checked element types. An Optional<Input> can hold an existing input or none and is
// allowed. Generic fields such as T.Item need validation after specialization, just like T.Item()
// construction; that separate limitation is tracked in shader-slang/slang#13686.
static RestrictedRayTracingTypeKind _getRestrictedRayTracingTypeKind(Type* type)
{
    type = unwrapModifiedType(type);
    auto kind = _getDirectRestrictedRayTracingTypeKind(type);
    if (kind != RestrictedRayTracingTypeKind::None)
        return kind;

    if (auto arrayType = as<ArrayExpressionType>(type))
        return _getRestrictedRayTracingTypeKind(arrayType->getElementType());
    if (auto optionalType = as<OptionalType>(type))
    {
        if (_getDirectStageInputKind(optionalType->getValueType()) !=
            StructuralRayTracingStageKind::Count)
            return RestrictedRayTracingTypeKind::None;
        return _getRestrictedRayTracingTypeKind(optionalType->getValueType());
    }
    if (auto pointerType = as<PtrTypeBase>(type))
        return _getRestrictedRayTracingTypeKind(pointerType->getValueType());
    if (auto tupleType = as<TupleType>(type))
    {
        for (Index i = 0; i < tupleType->getMemberCount(); ++i)
        {
            auto memberKind = _getRestrictedRayTracingTypeKind(tupleType->getMember(i));
            if (memberKind != RestrictedRayTracingTypeKind::None)
                return memberKind;
        }
    }
    if (auto typePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < typePack->getTypeCount(); ++i)
        {
            auto elementKind = _getRestrictedRayTracingTypeKind(typePack->getElementType(i));
            if (elementKind != RestrictedRayTracingTypeKind::None)
                return elementKind;
        }
    }
    return RestrictedRayTracingTypeKind::None;
}

// Reports an invalid use of an RT builtin type: a stage input or the stage interface itself.
// "RT builtin" means a compiler-recognized library type, not necessarily a C++ BuiltinType.
// Concrete structs implementing the interface remain ordinary types. Returns true when an error
// is reported, or false when the supplied type needs no diagnostic.
static bool _diagnoseInvalidRTBuiltinTypeUse(
    SemanticsVisitor* visitor,
    Type* type,
    SourceLoc location)
{
    switch (_getRestrictedRayTracingTypeKind(type))
    {
    case RestrictedRayTracingTypeKind::StageInterface:
        // The interface itself, e.g. IMissShader, cannot be a runtime value. A concrete struct
        // implementing IMissShader is an ordinary type and does not enter this branch.
        visitor->getSink()->diagnose(
            Diagnostics::StructuralRayTracingStageRuntimeValue{.type = type, .location = location});
        return true;
    case RestrictedRayTracingTypeKind::StageInput:
        // Allowed input parameters and local copies return from the variable checker before
        // reaching this helper. Here the context forbids an input, even in an array or pointer.
        visitor->getSink()->diagnose(
            Diagnostics::StructuralRayTracingInputStorage{.type = type, .location = location});
        return true;
    default:
        // Ordinary types and Optional<Input> need no diagnostic from this check.
        return false;
    }
}

// Checks one explicit or inferred generic type argument. Consider `consume(input)` with
// consume<T>(T value): its checked substitution names MissInput<C> even without written <...>.
// A variadic call groups these same arguments into a ConcreteTypePack, so inspect its elements.
// No inheritance, fields, or conformance witnesses are needed to recognize these builtin types.
static bool _diagnoseRestrictedRayTracingGenericArgument(
    SemanticsVisitor* visitor,
    Type* type,
    SourceLoc location)
{
    if (auto typePack = as<ConcreteTypePack>(type))
    {
        for (Index i = 0; i < typePack->getTypeCount(); ++i)
        {
            if (_diagnoseRestrictedRayTracingGenericArgument(
                    visitor,
                    typePack->getElementType(i),
                    location))
                return true;
        }
        return false;
    }
    switch (_getDirectRestrictedRayTracingTypeKind(type))
    {
    case RestrictedRayTracingTypeKind::StageInput:
        visitor->getSink()->diagnose(Diagnostics::StructuralRayTracingInputGenericArgument{
            .type = type,
            .location = location});
        return true;
    case RestrictedRayTracingTypeKind::StageInterface:
        return _diagnoseInvalidRTBuiltinTypeUse(visitor, type, location);
    default:
        return false;
    }
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
// For example, `struct Miss : rt::IMissShader` must be a user-defined struct and cannot share a
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

    _checkStructuralRayTracingStageImplementationType(this, witnessedType, conformanceLoc);
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

// Finishes stage and module-wide rules after checked conformance witnesses are available.
// For example, a generic extension can supply Miss.invoke; checking its selected stage before
// ordinary conformance and capability inference finish would inspect an incomplete signature.
void SemanticsVisitor::checkStructuralRayTracingModule(ModuleDecl* moduleDecl)
{
    _checkAttributedLegacyEntryPoints(getModule(moduleDecl), moduleDecl, getSink());
    _diagnoseStructuralStageDeclarations(this, moduleDecl);
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
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingVariableType(VarDeclBase* varDecl)
{
    auto type = varDecl->getType();
    // Recognize the input type itself, such as MissInput<C>. Count means it is not a stage input.
    if (_getDirectStageInputKind(type) != StructuralRayTracingStageKind::Count)
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
            diagnoseInvalidStructuralRayTracingExpression(varDecl->initExpr);
            // The local declaration is allowed; any initializer error was reported above.
            return;
        }
    }
    // Reject input declarations that were not accepted above: globals, fields, static locals,
    // disallowed parameters, and `MissInput<C> fresh;` when no initializer was inserted. This also
    // checks restricted types inside built-in containers and the stage interface itself.
    // Reaching this call does not imply an error: ordinary types, including concrete stage
    // structs, pass without a diagnostic.
    _diagnoseInvalidRTBuiltinTypeUse(this, type, varDecl->loc);
}

// Rejects results such as `rt::MissInput<C> makeInput();`. Inputs may be received as in parameters
// and copied locally, but a function cannot promise a new input. Constructors are checked at
// their call expressions instead, so their declarations do not trigger this result check.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingReturnType(CallableDecl* decl)
{
    if (as<ConstructorDecl>(decl))
        return;
    _diagnoseInvalidRTBuiltinTypeUse(
        this,
        decl->returnType.type,
        decl->returnType.exp ? decl->returnType.exp->loc : decl->loc);
}

// Rejects a property such as `property rt::MissInput<C> input { get; }`. Payload and record
// properties have ordinary application-defined result types and remain legal.
void SemanticsVisitor::diagnoseInvalidStructuralRayTracingPropertyType(PropertyDecl* propertyDecl)
{
    _diagnoseInvalidRTBuiltinTypeUse(
        this,
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
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingExpression(Expr* expr)
{
    auto type = expr->type;
    if (_getRestrictedRayTracingTypeKind(type) == RestrictedRayTracingTypeKind::None)
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
        diagnoseOnce(
            Diagnostics::StructuralRayTracingTypeConstruction{.type = type, .location = location});
        return true;
    }
    return _diagnoseInvalidRTBuiltinTypeUse(this, type, location);
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
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericArguments(InvokeExpr* invoke)
{
    auto functionDeclRef = as<DeclRefExpr>(invoke->functionExpr);
    if (!functionDeclRef)
        return false;

    bool diagnosed = false;
    SubstitutionSet(functionDeclRef->declRef)
        .forEachGenericSubstitution(
            [&](GenericDecl* genericDecl, Val::OperandView<Val> arguments)
            {
                // Optional<Input>'s builtin constructors wrap an existing value or produce none.
                // Exempt only Optional's substitution, not a user helper returning Optional<T>.
                if (diagnosed ||
                    _getMagicTypeClass(genericDecl->inner) == ASTNodeType::OptionalType)
                    return;
                for (auto argument : arguments)
                {
                    // Integer arguments and conformance witnesses need no type-use check.
                    if (auto argumentType = as<Type>(argument))
                    {
                        if (_diagnoseRestrictedRayTracingGenericArgument(
                                this,
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
// arguments once normal type resolution succeeds. Optional<Input> is allowed because wrapping
// an existing input or none does not construct Input. A concrete stage struct is an ordinary
// argument, so shader lists and generic helpers require no special exemption for those structs.
bool SemanticsVisitor::diagnoseInvalidStructuralRayTracingGenericTypeApplication(
    GenericAppExpr* genericApplication,
    Expr* checkedResult)
{
    auto applicationTypeType = checkedResult ? as<TypeType>(checkedResult->type) : nullptr;
    bool isOptional = applicationTypeType && as<OptionalType>(applicationTypeType->getType());
    for (auto argument : genericApplication->arguments)
    {
        if (auto argumentType = as<TypeType>(argument->type))
        {
            if (isOptional && _getDirectStageInputKind(argumentType->getType()) !=
                                  StructuralRayTracingStageKind::Count)
                continue;
            if (_diagnoseRestrictedRayTracingGenericArgument(
                    this,
                    argumentType->getType(),
                    argument->loc))
                return true;
        }
    }
    return false;
}

} // namespace Slang
