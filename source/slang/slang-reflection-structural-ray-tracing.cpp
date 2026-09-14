#include "slang-reflection-structural-ray-tracing.h"

#include "slang-check-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir-link.h"
#include "slang-ir-structural-ray-tracing.h"
#include "slang-linkable-impls.h"
#include "slang-linkable.h"
#include "slang-mangle.h"
#include "slang-module.h"
#include "slang-target-program.h"
#include "slang-target.h"
#include "slang-type-layout.h"

namespace Slang
{

static String _getStageEntryPointName(ASTBuilder* astBuilder, Type* stageType)
{
    auto sourceTypeName = getStructuralRayTracingSourceTypeName(astBuilder, stageType);
    if (sourceTypeName.getLength() == 0)
        return String();

    return getStructuralRayTracingEntryPointName(sourceTypeName.getUnownedSlice());
}

static RefPtr<StructuralRayTracingStageReflection> _createStageReflection(
    ASTBuilder* astBuilder,
    Type* stageType,
    StructuralRayTracingStageKind stageKind)
{
    if (!stageType)
        return nullptr;

    RefPtr<StructuralRayTracingStageReflection> result = new StructuralRayTracingStageReflection();
    result->stageKind = stageKind;
    result->type = stageType;
    result->entryPointName = _getStageEntryPointName(astBuilder, stageType);
    return result;
}

static RefPtr<StructuralRayTracingStageReflection> _createAssociatedStageReflection(
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    SubtypeWitness* groupWitness,
    StructuralRayTracingAssociatedTypeKind associatedTypeKind,
    StructuralRayTracingStageKind stageKind)
{
    auto stageType = registry.resolveAssociatedType(astBuilder, groupWitness, associatedTypeKind);
    if (!stageType || registry.isStagePlaceholder(stageKind, stageType))
        return nullptr;

    return _createStageReflection(astBuilder, stageType, stageKind);
}

static StructuralRayTracingPayloadReflection* _findOrAddPayload(
    StructuralRayTracingProgramSchemaReflection* result,
    Type* payloadType)
{
    for (auto payload : result->payloads)
    {
        if (payload->payloadType->equals(payloadType))
            return payload;
    }

    // Payload partitions are ordered by first appearance. Hit groups are visited before miss
    // shaders, matching the ordering contract exposed by reflection.
    RefPtr<StructuralRayTracingPayloadReflection> payload =
        new StructuralRayTracingPayloadReflection();
    payload->payloadType = payloadType;
    result->payloads.add(payload);
    return payload;
}

static bool _doesContextBelongToSchema(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    SubtypeWitness* contextWitness)
{
    auto traceContextType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::StageTraceContext);
    return traceContextType && traceContextType->equals(result->traceContextType);
}

// Appends one checked hit-group type to its payload partition.
//
// Closed-schema reflection passes `functionIndex == -1` and obtains declaration-order indices.
// Open-schema reflection passes the finalized IR index so reflection agrees exactly with the
// linked adapters and with the record header the host writes.
static bool _addHitGroup(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* groupType,
    SubtypeWitness* groupWitness,
    Index functionIndex,
    bool isLinked)
{
    auto contextType = registry.resolveAssociatedType(
        astBuilder,
        groupWitness,
        StructuralRayTracingAssociatedTypeKind::HitGroupContext);
    auto contextWitness = registry.resolveAssociatedTypeConstraint(
        astBuilder,
        groupWitness,
        StructuralRayTracingAssociatedTypeKind::HitGroupContext);
    if (!contextType || !contextWitness ||
        !_doesContextBelongToSchema(result, astBuilder, registry, contextWitness))
    {
        return false;
    }

    auto payloadType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
    auto recordType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::StageRecord);
    if (!payloadType || !recordType)
        return false;

    auto payload = _findOrAddPayload(result, payloadType);
    if (functionIndex >= 0 && functionIndex != payload->hitGroups.getCount())
        return false;
    RefPtr<StructuralRayTracingHitGroupReflection> group =
        new StructuralRayTracingHitGroupReflection();
    group->functionIndex = functionIndex >= 0 ? functionIndex : payload->hitGroups.getCount();
    group->groupType = groupType;
    group->contextType = contextType;
    group->recordType = recordType;
    group->isLinked = isLinked;
    group->primitiveType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::HitPrimitive);
    auto primitiveWitness = registry.resolveAssociatedTypeConstraint(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::HitPrimitive);
    group->intersectionAttributesType = registry.resolveAssociatedType(
        astBuilder,
        primitiveWitness,
        StructuralRayTracingAssociatedTypeKind::PrimitiveAttributes);
    group->closestHit = _createAssociatedStageReflection(
        astBuilder,
        registry,
        groupWitness,
        StructuralRayTracingAssociatedTypeKind::HitGroupClosestHit,
        StructuralRayTracingStageKind::ClosestHit);
    if (group->closestHit)
        group->closestHitEntryPointName = group->closestHit->entryPointName;
    group->anyHit = _createAssociatedStageReflection(
        astBuilder,
        registry,
        groupWitness,
        StructuralRayTracingAssociatedTypeKind::HitGroupAnyHit,
        StructuralRayTracingStageKind::AnyHit);
    group->intersection = _createAssociatedStageReflection(
        astBuilder,
        registry,
        groupWitness,
        StructuralRayTracingAssociatedTypeKind::HitGroupIntersection,
        StructuralRayTracingStageKind::Intersection);
    if (!group->recordType || !group->primitiveType || !group->intersectionAttributesType)
        return false;
    payload->hitGroups.add(group);
    return true;
}

static bool _addHitGroups(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* groupListType)
{
    auto pack = getStructuralRayTracingEntryPack(astBuilder, groupListType);
    if (pack.types->getTypeCount() != 0 && !pack.witnesses)
        return false;

    for (Index i = 0; i < pack.types->getTypeCount(); ++i)
    {
        auto groupType = pack.types->getElementType(i);
        auto groupWitness = pack.witnesses->getWitness(i);
        if (!_addHitGroup(result, astBuilder, registry, groupType, groupWitness, -1, false))
        {
            return false;
        }
    }
    return true;
}

// Appends one checked miss-shader type to its payload partition, preserving a finalized linked
// index when one was supplied by open-section completion.
static bool _addMissShader(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* shaderType,
    SubtypeWitness* shaderWitness,
    Index functionIndex,
    bool isLinked)
{
    auto contextType = registry.resolveAssociatedType(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::MissShaderContext);
    auto contextWitness = registry.resolveAssociatedTypeConstraint(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::MissShaderContext);
    if (!contextType || !contextWitness ||
        !_doesContextBelongToSchema(result, astBuilder, registry, contextWitness))
    {
        return false;
    }

    auto payloadType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::PayloadContextPayload);
    auto recordType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::StageRecord);
    if (!payloadType || !recordType)
        return false;

    auto payload = _findOrAddPayload(result, payloadType);
    if (functionIndex >= 0 && functionIndex != payload->missShaders.getCount())
        return false;
    RefPtr<StructuralRayTracingMissShaderReflection> shader =
        new StructuralRayTracingMissShaderReflection();
    shader->functionIndex = functionIndex >= 0 ? functionIndex : payload->missShaders.getCount();
    shader->shaderType = shaderType;
    shader->contextType = contextType;
    shader->recordType = recordType;
    shader->isLinked = isLinked;
    shader->miss =
        _createStageReflection(astBuilder, shaderType, StructuralRayTracingStageKind::Miss);
    if (!shader->miss)
        return false;
    payload->missShaders.add(shader);
    return true;
}

static bool _addMissShaders(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* shaderListType)
{
    auto pack = getStructuralRayTracingEntryPack(astBuilder, shaderListType);
    if (pack.types->getTypeCount() != 0 && !pack.witnesses)
        return false;

    for (Index i = 0; i < pack.types->getTypeCount(); ++i)
    {
        auto shaderType = pack.types->getElementType(i);
        auto shaderWitness = pack.witnesses->getWitness(i);
        if (!_addMissShader(result, astBuilder, registry, shaderType, shaderWitness, -1, false))
        {
            return false;
        }
    }
    return true;
}

// Appends one checked callable-shader type to the schema-wide callable table.
//
// Unlike hit and miss indices, callable indices do not restart for each payload because callable
// data is independent of ray payload data.
static bool _addCallableShader(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* shaderType,
    SubtypeWitness* shaderWitness,
    Index functionIndex,
    bool isLinked)
{
    auto contextType = registry.resolveAssociatedType(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::CallableShaderContext);
    auto contextWitness = registry.resolveAssociatedTypeConstraint(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::CallableShaderContext);
    if (!contextType || !contextWitness ||
        !_doesContextBelongToSchema(result, astBuilder, registry, contextWitness))
    {
        return false;
    }

    RefPtr<StructuralRayTracingCallableShaderReflection> shader =
        new StructuralRayTracingCallableShaderReflection();
    if (functionIndex >= 0 && functionIndex != result->callableShaders.getCount())
        return false;
    shader->functionIndex = functionIndex >= 0 ? functionIndex : result->callableShaders.getCount();
    shader->shaderType = shaderType;
    shader->contextType = contextType;
    shader->recordType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::StageRecord);
    shader->callableDataType = registry.resolveAssociatedType(
        astBuilder,
        contextWitness,
        StructuralRayTracingAssociatedTypeKind::CallableData);
    shader->isLinked = isLinked;
    shader->callable =
        _createStageReflection(astBuilder, shaderType, StructuralRayTracingStageKind::Callable);
    if (!shader->recordType || !shader->callableDataType || !shader->callable)
        return false;
    if (result->callableShaders.getCount() != 0 &&
        !result->callableShaders[0]->callableDataType->equals(shader->callableDataType))
    {
        // One schema produces one callable VFT on Metal, so reflection must not publish a
        // layout whose entries require incompatible function signatures. Compilation emits
        // the user-facing schema diagnostic at the linked IR validation boundary.
        return false;
    }
    result->callableShaders.add(shader);
    return true;
}

static bool _addCallableShaders(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* shaderListType)
{
    auto pack = getStructuralRayTracingEntryPack(astBuilder, shaderListType);
    if (pack.types->getTypeCount() != 0 && !pack.witnesses)
        return false;

    for (Index i = 0; i < pack.types->getTypeCount(); ++i)
    {
        auto shaderType = pack.types->getElementType(i);
        auto shaderWitness = pack.witnesses->getWitness(i);
        if (!_addCallableShader(result, astBuilder, registry, shaderType, shaderWitness, -1, false))
        {
            return false;
        }
    }
    return true;
}

// Finds the producer-owned entry metadata paired with one canonical identity in the summary pack.
static IRDecoration* _findFinalizedStructuralRayTracingEntryInfo(
    IRStructuralRayTracingProgramSchema* schema,
    StructuralRayTracingSectionKind kind,
    UnownedStringSlice typeIdentity)
{
    IRDecoration* result = nullptr;
    for (auto decoration : schema->getDecorations())
    {
        IRStringLit* candidateIdentity = nullptr;
        switch (kind)
        {
        case StructuralRayTracingSectionKind::HitGroups:
            if (auto info = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
                candidateIdentity = info->getGroupTypeIdentity();
            break;
        case StructuralRayTracingSectionKind::MissShaders:
            if (auto info = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
                candidateIdentity = info->getMissTypeIdentity();
            break;
        case StructuralRayTracingSectionKind::CallableShaders:
            if (auto info = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration))
                candidateIdentity = info->getCallableTypeIdentity();
            break;
        default:
            SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
        }
        if (!candidateIdentity || candidateIdentity->getStringSlice() != typeIdentity)
            continue;
        // Completion deduplicates a section by this canonical identity before adding an entry.
        SLANG_RELEASE_ASSERT(!result);
        result = decoration;
    }
    return result;
}

static IRStringLit* _getStructuralRayTracingEntryDeclLookupName(
    IRDecoration* entryInfo,
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return cast<IRStructuralRayTracingHitGroupInfoDecoration>(entryInfo)
            ->getGroupDeclLookupName();
    case StructuralRayTracingSectionKind::MissShaders:
        return cast<IRStructuralRayTracingMissShaderInfoDecoration>(entryInfo)
            ->getMissDeclLookupName();
    case StructuralRayTracingSectionKind::CallableShaders:
        return cast<IRStructuralRayTracingCallableShaderInfoDecoration>(entryInfo)
            ->getCallableDeclLookupName();
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

// Returns the checked AST type selected by one finalized structural entry.
//
// There are two producer paths for the same canonical representation. Source-loaded entries pass
// through `_lowerStructuralRayTracingCanonicalTypeIdentity`, which registers their exact semantic
// type directly. A precompiled plugin such as `struct Glass : IMaterialHit` instead contributes
// serialized IR to a fresh session, so that lowering-time registration did not happen in this
// `Linkage`. Its compiler-owned entry metadata carries the exact unobfuscated key already used by
// the defining module's serialized export table. We look up that declaration in the ordinary
// composite's module dependency closure, construct its nominal type, and accept it only if
// recomputing the canonical type identity yields the identity selected by link-time schema
// completion. Thus neither source names nor obfuscated IR linkage names become a second identity
// system.
static Type* _findOrRecoverStructuralRayTracingReflectionType(
    ComponentType* program,
    StructuralRayTracingDeclRegistry& registry,
    UnownedStringSlice typeIdentity,
    UnownedStringSlice declLookupName)
{
    if (auto registeredType = registry.findReflectionType(typeIdentity))
        return registeredType;

    SLANG_RELEASE_ASSERT(
        program && typeIdentity.getLength() != 0 && declLookupName.getLength() != 0);
    auto astBuilder = program->getLinkage()->getASTBuilder();
    Type* recoveredType = nullptr;
    for (auto module : program->getModuleDependencies())
    {
        auto decl = module->findExportedDeclByMangledName(declLookupName);
        auto typeDecl = as<AggTypeDecl>(decl);
        if (!typeDecl)
            continue;

        auto candidateType =
            DeclRefType::create(astBuilder, makeDeclRef(typeDecl))->getCanonicalType();
        auto candidateIdentity = getMangledTypeName(astBuilder, candidateType);
        if (candidateIdentity.getUnownedSlice() != typeIdentity)
            continue;

        // Canonical type identity is injective. Seeing a different canonical type here would mean
        // the producer persisted an ambiguous identity, so reject it at this boundary.
        SLANG_RELEASE_ASSERT(!recoveredType || recoveredType == candidateType);
        recoveredType = candidateType;
    }

    if (recoveredType)
        registry.registerReflectionType(typeIdentity, recoveredType);
    return recoveredType;
}

// Reconstructs the ordinary source-level entry witness used by existing closed-schema reflection.
// The manifest selects identity and index; checked AST semantics remain the source of associated
// payload, context, record, and stage types exposed through the public API.
static SubtypeWitness* _getStructuralRayTracingEntryWitness(
    Linkage* linkage,
    const StructuralRayTracingDeclRegistry& registry,
    Type* entryType,
    StructuralRayTracingSectionKind kind)
{
    auto interfaceDecl = registry.getSectionEntryInterface(kind);
    if (!interfaceDecl)
        return nullptr;
    auto interfaceType = DeclRefType::create(linkage->getASTBuilder(), makeDeclRef(interfaceDecl));
    auto sharedSemanticsContext = linkage->getSemanticsForReflection();
    SemanticsContext semanticsContext(sharedSemanticsContext);
    SemanticsVisitor visitor(semanticsContext);
    return visitor.isSubtype(entryType, interfaceType, IsSubTypeOptions::None);
}

// Converts one finalized manifest section back into the public source-type reflection model.
static bool _addFinalizedStructuralRayTracingSection(
    StructuralRayTracingProgramSchemaReflection* result,
    ComponentType* program,
    Linkage* linkage,
    StructuralRayTracingDeclRegistry& registry,
    IRStructuralRayTracingProgramSchema* schema,
    StructuralRayTracingSectionKind kind,
    IRMakeValuePack* typeIdentities)
{
    auto astBuilder = linkage->getASTBuilder();
    for (UInt i = 0; i < typeIdentities->getOperandCount(); ++i)
    {
        auto identity = as<IRStringLit>(typeIdentities->getOperand(i));
        if (!identity)
            return false;
        auto identityText = identity->getStringSlice();
        auto entryInfo = _findFinalizedStructuralRayTracingEntryInfo(schema, kind, identityText);
        if (!entryInfo)
            return false;
        auto declLookupName = _getStructuralRayTracingEntryDeclLookupName(entryInfo, kind);
        auto entryType = declLookupName ? _findOrRecoverStructuralRayTracingReflectionType(
                                              program,
                                              registry,
                                              identityText,
                                              declLookupName->getStringSlice())
                                        : nullptr;
        if (!entryType)
            return false;
        auto entryWitness =
            _getStructuralRayTracingEntryWitness(linkage, registry, entryType, kind);
        if (!entryWitness)
            return false;

        switch (kind)
        {
        case StructuralRayTracingSectionKind::HitGroups:
            {
                auto info = cast<IRStructuralRayTracingHitGroupInfoDecoration>(entryInfo);
                if (!_addHitGroup(
                        result,
                        astBuilder,
                        registry,
                        entryType,
                        entryWitness,
                        Index(info->getFunctionIndex()->getValue()),
                        info->getIsLinked()->getValue()))
                {
                    return false;
                }
                break;
            }
        case StructuralRayTracingSectionKind::MissShaders:
            {
                auto info = cast<IRStructuralRayTracingMissShaderInfoDecoration>(entryInfo);
                if (!_addMissShader(
                        result,
                        astBuilder,
                        registry,
                        entryType,
                        entryWitness,
                        Index(info->getFunctionIndex()->getValue()),
                        info->getIsLinked()->getValue()))
                {
                    return false;
                }
                break;
            }
        case StructuralRayTracingSectionKind::CallableShaders:
            {
                auto info = cast<IRStructuralRayTracingCallableShaderInfoDecoration>(entryInfo);
                if (!_addCallableShader(
                        result,
                        astBuilder,
                        registry,
                        entryType,
                        entryWitness,
                        Index(info->getFunctionIndex()->getValue()),
                        info->getIsLinked()->getValue()))
                {
                    return false;
                }
                break;
            }
        default:
            SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
        }
    }
    return true;
}

// Finds the link-finalized summary for an exact schema identity.
static IRStructuralRayTracingProgramSchema* _findFinalizedStructuralRayTracingProgramSchema(
    IRModule* manifest,
    UnownedStringSlice schemaTypeIdentity)
{
    IRStructuralRayTracingProgramSchema* result = nullptr;
    for (auto inst : manifest->getGlobalInsts())
    {
        auto schema = as<IRStructuralRayTracingProgramSchema>(inst);
        if (!schema || schema->getSchemaTypeIdentity()->getStringSlice() != schemaTypeIdentity)
            continue;
        // An internally requested schema normally has one summary. If the client already composed
        // the same exact conformance, either summary is equivalent because both are completed from
        // the same manifest conformance index; prefer the first without merging metadata.
        if (!result)
            result = schema;
    }
    return result;
}

// Builds the reflection-only composite that requests one exact schema summary.
//
// The user's normal program remains unchanged. Adding the compiler-created conformance component
// here gives the whole-program linker a liveness root even when no entry point calls `trace`.
static RefPtr<IRModule> _getFinalizedStructuralRayTracingProgramSchemaManifest(
    ProgramLayout* programLayout,
    SubtypeWitness* schemaWitness,
    DiagnosticSink* sink)
{
    auto program = programLayout->getProgram();
    auto linkage = program->getLinkage();
    RefPtr<TypeConformance> schemaRequest = new TypeConformance(linkage, schemaWitness, -1, sink);
    List<RefPtr<ComponentType>> components;
    components.add(program);
    components.add(schemaRequest);
    auto reflectionProgram = CompositeComponentType::create(linkage, components);

    // Consider a schema and a plugin that both import a context module containing their
    // `IHitContext` conformances. The ordinary program may still expose that context module as an
    // unsatisfied component requirement: `getLayout()` does not implicitly perform the public
    // `link()` operation. Complete the private reflection composite through the same component
    // requirement path so its IR link sees the imported witness definitions as well as the
    // declarations that reference them.
    auto linkedReflectionProgram = fillRequirements(reflectionProgram);
    SLANG_RELEASE_ASSERT(linkedReflectionProgram);
    auto targetProgram = linkedReflectionProgram->getTargetProgram(programLayout->getTargetReq());
    return getOrCreateStructuralRayTracingProgramManifest(targetProgram, sink);
}

// Populates public reflection from the three entry lists completed in the target manifest.
static bool _addFinalizedStructuralRayTracingProgramSchema(
    StructuralRayTracingProgramSchemaReflection* result,
    ComponentType* program,
    Linkage* linkage,
    StructuralRayTracingDeclRegistry& registry,
    IRStructuralRayTracingProgramSchema* schema)
{
    result->name = schema->getSchemaSourceTypeName()->getStringSlice();
    result->hitGroupSectionOpen = schema->getHitGroupSectionOpen()->getValue();
    result->missShaderSectionOpen = schema->getMissShaderSectionOpen()->getValue();
    result->callableShaderSectionOpen = schema->getCallableShaderSectionOpen()->getValue();
    return _addFinalizedStructuralRayTracingSection(
               result,
               program,
               linkage,
               registry,
               schema,
               StructuralRayTracingSectionKind::HitGroups,
               schema->getHitGroupTypeIdentities()) &&
           _addFinalizedStructuralRayTracingSection(
               result,
               program,
               linkage,
               registry,
               schema,
               StructuralRayTracingSectionKind::MissShaders,
               schema->getMissShaderTypeIdentities()) &&
           _addFinalizedStructuralRayTracingSection(
               result,
               program,
               linkage,
               registry,
               schema,
               StructuralRayTracingSectionKind::CallableShaders,
               schema->getCallableShaderTypeIdentities());
}

static void _addMetalIntersectionFunctionReflection(
    StructuralRayTracingPayloadReflection* payload,
    UnownedStringSlice schemaSourceTypeName,
    Index payloadIndex,
    StructuralRayTracingMetalCandidateKind geometryKind,
    StructuralRayTracingMetalIntersectionFunctionImplementationKind implementationKind)
{
    RefPtr<StructuralRayTracingIntersectionFunctionReflection> function =
        new StructuralRayTracingIntersectionFunctionReflection();
    function->intersectionFunctionTableIndex = Index(geometryKind);
    function->geometryKind = geometryKind;
    function->implementationKind = implementationKind;
    if (implementationKind ==
        StructuralRayTracingMetalIntersectionFunctionImplementationKind::ExportedFunction)
    {
        function->entryPointName = getStructuralRayTracingMetalCandidateDispatcherName(
            schemaSourceTypeName,
            payloadIndex,
            geometryKind);
    }
    payload->intersectionFunctionTableSize = Math::Max(
        payload->intersectionFunctionTableSize,
        function->intersectionFunctionTableIndex + 1);
    payload->intersectionFunctions.add(function);
}

// Replaces portable source-stage names with the exact schema-specific Metal symbols and describes
// the sparse IFT that the host must construct. AnyHit and Intersection do not have independently
// bindable Metal symbols: the compiler folds them into one dispatcher per payload and geometry.
static bool _populateMetalFunctionReflection(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    TargetRequest* targetRequest)
{
    if (!isMetalTarget(targetRequest))
        return true;

    if (result->name.getLength() == 0)
        return false;
    auto schemaSourceTypeName = result->name.getUnownedSlice();

    for (Index payloadIndex = 0; payloadIndex < result->payloads.getCount(); ++payloadIndex)
    {
        auto payload = result->payloads[payloadIndex];
        bool hasTriangle = false;
        bool hasCurve = false;
        bool hasBoundingBox = false;
        bool hasTriangleDispatcher = false;
        bool hasCurveDispatcher = false;
        bool hasBoundingBoxDispatcher = false;
        bool hasClosestHitTable = false;
        for (auto group : payload->hitGroups)
            hasClosestHitTable |= group->closestHit != nullptr;
        for (auto group : payload->hitGroups)
        {
            auto groupSourceTypeName =
                getStructuralRayTracingSourceTypeName(astBuilder, group->groupType);
            if (groupSourceTypeName.getLength() == 0)
                return false;
            if (group->closestHit)
            {
                auto stageSourceTypeName =
                    getStructuralRayTracingSourceTypeName(astBuilder, group->closestHit->type);
                if (stageSourceTypeName.getLength() == 0)
                    return false;
                group->closestHit->entryPointName =
                    getStructuralRayTracingMetalClosestHitFunctionName(
                        schemaSourceTypeName,
                        payloadIndex,
                        group->functionIndex,
                        groupSourceTypeName.getUnownedSlice(),
                        stageSourceTypeName.getUnownedSlice());
                group->closestHitEntryPointName = group->closestHit->entryPointName;
            }
            else if (hasClosestHitTable)
            {
                // The logical placeholder remains absent from stage reflection, while this exact
                // physical symbol tells the host what must fill its dense Metal VFT slot.
                group->closestHitEntryPointName =
                    getStructuralRayTracingMetalNoOpClosestHitFunctionName(
                        schemaSourceTypeName,
                        payloadIndex,
                        group->functionIndex,
                        groupSourceTypeName.getUnownedSlice());
            }
            if (group->anyHit)
                group->anyHit->entryPointName = String();
            if (group->intersection)
                group->intersection->entryPointName = String();

            switch (registry.getHitAttributesKind(group->primitiveType))
            {
            case StructuralRayTracingHitAttributesKind::Triangle:
                hasTriangle = true;
                hasTriangleDispatcher |= group->anyHit != nullptr;
                break;
            case StructuralRayTracingHitAttributesKind::Curve:
                hasCurve = true;
                hasCurveDispatcher |= group->anyHit != nullptr;
                break;
            case StructuralRayTracingHitAttributesKind::Custom:
                hasBoundingBox = true;
                hasBoundingBoxDispatcher |= group->intersection != nullptr;
                break;
            default:
                return false;
            }
        }
        for (auto shader : payload->missShaders)
        {
            auto stageSourceTypeName =
                getStructuralRayTracingSourceTypeName(astBuilder, shader->miss->type);
            if (stageSourceTypeName.getLength() == 0)
                return false;
            shader->miss->entryPointName = getStructuralRayTracingMetalMissFunctionName(
                schemaSourceTypeName,
                payloadIndex,
                shader->functionIndex,
                stageSourceTypeName.getUnownedSlice());
        }

        // A payload with no generated candidate dispatcher does not consume an IFT. Once any
        // geometry does need one, enumerate the built-in opaque functions for other geometry
        // kinds used by the same payload so the host can populate the fixed sparse indices.
        if (!(hasTriangleDispatcher || hasCurveDispatcher || hasBoundingBoxDispatcher))
            continue;
        if (hasTriangle)
        {
            _addMetalIntersectionFunctionReflection(
                payload,
                schemaSourceTypeName,
                payloadIndex,
                StructuralRayTracingMetalCandidateKind::Triangle,
                hasTriangleDispatcher
                    ? StructuralRayTracingMetalIntersectionFunctionImplementationKind::
                          ExportedFunction
                    : StructuralRayTracingMetalIntersectionFunctionImplementationKind::
                          OpaqueTriangle);
        }
        if (hasBoundingBox && hasBoundingBoxDispatcher)
        {
            _addMetalIntersectionFunctionReflection(
                payload,
                schemaSourceTypeName,
                payloadIndex,
                StructuralRayTracingMetalCandidateKind::BoundingBox,
                StructuralRayTracingMetalIntersectionFunctionImplementationKind::ExportedFunction);
        }
        if (hasCurve)
        {
            _addMetalIntersectionFunctionReflection(
                payload,
                schemaSourceTypeName,
                payloadIndex,
                StructuralRayTracingMetalCandidateKind::Curve,
                hasCurveDispatcher
                    ? StructuralRayTracingMetalIntersectionFunctionImplementationKind::
                          ExportedFunction
                    : StructuralRayTracingMetalIntersectionFunctionImplementationKind::OpaqueCurve);
        }
    }

    for (auto shader : result->callableShaders)
    {
        auto stageSourceTypeName =
            getStructuralRayTracingSourceTypeName(astBuilder, shader->callable->type);
        if (stageSourceTypeName.getLength() == 0)
            return false;
        shader->callable->entryPointName = getStructuralRayTracingMetalCallableFunctionName(
            schemaSourceTypeName,
            shader->functionIndex,
            stageSourceTypeName.getUnownedSlice());
    }
    return true;
}

// Computes the exact stride used by Metal lowering for one source record type. Structural records
// are stored in a raw device buffer, so their application-data portion follows the target's
// structured-buffer layout rather than constant-buffer packing rules.
static bool _tryGetMetalRecordStride(
    TargetRequest* targetRequest,
    Type* recordType,
    size_t& outStride)
{
    auto typeLayout =
        targetRequest->getTypeLayout(recordType, slang::LayoutRules::DefaultStructuredBuffer);
    if (!typeLayout)
        return false;

    UInt64 dataSize = 0;
    if (auto uniformInfo = typeLayout->FindResourceInfo(LayoutResourceKind::Uniform))
    {
        if (!uniformInfo->count.isFinite())
            return false;
        dataSize = uniformInfo->count.getFiniteValue().getValidValue();
    }
    outStride = size_t(getStructuralRayTracingMetalRecordStride(dataSize));
    return true;
}

// Resolves public layouts only after manifest identity has selected the AST types. Payloads use
// the target's ordinary reflected layout. Metal records use the structured-buffer rule consumed by
// the generated raw record-buffer access; other backends retain the target's ordinary type layout.
static bool _populateStructuralRayTracingTypeLayouts(
    StructuralRayTracingProgramSchemaReflection* result,
    TargetRequest* targetRequest)
{
    auto recordRules = isMetalTarget(targetRequest) ? slang::LayoutRules::DefaultStructuredBuffer
                                                    : slang::LayoutRules::Default;
    for (auto payload : result->payloads)
    {
        payload->typeLayout =
            targetRequest->getTypeLayout(payload->payloadType, slang::LayoutRules::Default);
        if (!payload->typeLayout)
            return false;
        for (auto group : payload->hitGroups)
        {
            group->recordTypeLayout = targetRequest->getTypeLayout(group->recordType, recordRules);
            if (!group->recordTypeLayout)
                return false;
        }
        for (auto shader : payload->missShaders)
        {
            shader->recordTypeLayout =
                targetRequest->getTypeLayout(shader->recordType, recordRules);
            if (!shader->recordTypeLayout)
                return false;
        }
    }
    for (auto shader : result->callableShaders)
    {
        shader->recordTypeLayout = targetRequest->getTypeLayout(shader->recordType, recordRules);
        if (!shader->recordTypeLayout)
            return false;
    }
    return true;
}

// Finds the target-manifest entry paired with a reflected AST type. The canonical mangled type
// identity is already the schema completion key; using it here avoids coupling ABI reflection to
// source names, list positions, or decoration operand indices.
static IRDecoration* _findFinalizedStructuralRayTracingEntryInfo(
    ASTBuilder* astBuilder,
    IRStructuralRayTracingProgramSchema* schema,
    StructuralRayTracingSectionKind kind,
    Type* entryType)
{
    SLANG_RELEASE_ASSERT(astBuilder && schema && entryType);
    auto identity = getMangledTypeName(astBuilder, entryType->getCanonicalType());
    return _findFinalizedStructuralRayTracingEntryInfo(schema, kind, identity.getUnownedSlice());
}

// Publishes the native pipeline-interface sizes derived from the same canonical target IR that
// target legalization consumes. Ordinary `TypeLayout` is intentionally not used here: constant-
// buffer packing is not the Vulkan scalar block rule, and OptiX payload/attribute registers are a
// transport ABI rather than a packed source struct.
static bool _populateNativeRayTracingABISizes(
    StructuralRayTracingProgramSchemaReflection* result,
    ASTBuilder* astBuilder,
    TargetRequest* targetRequest,
    IRModule* manifest,
    IRStructuralRayTracingProgramSchema* finalizedSchema)
{
    if (!(isD3DTarget(targetRequest) || isKhronosTarget(targetRequest) ||
          isCUDATarget(targetRequest)))
    {
        return true;
    }
    if (!manifest || !finalizedSchema)
        return false;

    IRBuilder builder(manifest);
    for (auto payload : result->payloads)
    {
        IRType* payloadType = nullptr;
        IRType* payloadSemanticType = nullptr;
        if (payload->hitGroups.getCount() != 0)
        {
            auto entryInfo = as<IRStructuralRayTracingHitGroupInfoDecoration>(
                _findFinalizedStructuralRayTracingEntryInfo(
                    astBuilder,
                    finalizedSchema,
                    StructuralRayTracingSectionKind::HitGroups,
                    payload->hitGroups[0]->groupType));
            if (!entryInfo)
                return false;
            payloadType = entryInfo->getPayloadType();
            payloadSemanticType = entryInfo->getPayloadSemanticType();
        }
        else if (payload->missShaders.getCount() != 0)
        {
            auto entryInfo = as<IRStructuralRayTracingMissShaderInfoDecoration>(
                _findFinalizedStructuralRayTracingEntryInfo(
                    astBuilder,
                    finalizedSchema,
                    StructuralRayTracingSectionKind::MissShaders,
                    payload->missShaders[0]->shaderType));
            if (!entryInfo)
                return false;
            payloadType = entryInfo->getPayloadType();
            payloadSemanticType = entryInfo->getPayloadSemanticType();
        }
        else
        {
            SLANG_UNEXPECTED("a reflected payload partition has no hit or miss entry");
        }

        IRIntegerValue nativeSize = 0;
        if (SLANG_FAILED(getStructuralRayTracingNativePayloadSize(
                targetRequest,
                &builder,
                payloadType,
                payloadSemanticType,
                &nativeSize)) ||
            nativeSize < 0)
        {
            return false;
        }
        payload->nativePayloadSize = size_t(nativeSize);
    }

    for (auto payload : result->payloads)
    {
        for (auto group : payload->hitGroups)
        {
            auto entryInfo = as<IRStructuralRayTracingHitGroupInfoDecoration>(
                _findFinalizedStructuralRayTracingEntryInfo(
                    astBuilder,
                    finalizedSchema,
                    StructuralRayTracingSectionKind::HitGroups,
                    group->groupType));
            if (!entryInfo)
                return false;

            IRIntegerValue nativeSize = 0;
            auto attributesKind = StructuralRayTracingHitAttributesKind(
                entryInfo->getHitAttributesKind()->getValue());
            switch (attributesKind)
            {
            case StructuralRayTracingHitAttributesKind::Triangle:
                // Native triangle attributes are two 32-bit barycentric coordinates. The source
                // `TriangleData` view also exposes properties backed by other built-ins, so its
                // ordinary struct layout is not the native hit-attribute ABI.
                nativeSize = 8;
                break;
            case StructuralRayTracingHitAttributesKind::Curve:
            case StructuralRayTracingHitAttributesKind::None:
                // Curves are Metal-only in the structural API; Metal has no native host maximum.
                break;
            case StructuralRayTracingHitAttributesKind::Custom:
                if (SLANG_FAILED(getStructuralRayTracingNativeHitAttributeSize(
                        targetRequest,
                        &builder,
                        entryInfo->getHitAttributesType(),
                        &nativeSize)))
                {
                    return false;
                }
                break;
            default:
                SLANG_UNEXPECTED("invalid structural ray-tracing hit-attribute kind");
            }
            if (nativeSize < 0)
                return false;
            result->maxNativeHitAttributeSize =
                Math::Max(result->maxNativeHitAttributeSize, size_t(nativeSize));
        }
    }
    if (isCUDATarget(targetRequest))
    {
        // Every OptiX pipeline reserves at least two attribute registers, even if this schema has
        // no hit group or a custom type with only one scalar leaf. Triangle attributes naturally
        // occupy the same two-register baseline.
        result->maxNativeHitAttributeSize = Math::Max(
            result->maxNativeHitAttributeSize,
            size_t(kOptiXMinHitAttributeRegisterCount * kOptiXRayTracingRegisterSize));
    }
    return true;
}

// Publishes the compiler-owned Metal record-buffer ABI through schema reflection. Other targets
// use native shader-table representations whose strides remain host-defined, so their reflected
// values deliberately stay zero.
static bool _populateMetalRecordStrides(
    StructuralRayTracingProgramSchemaReflection* result,
    TargetRequest* targetRequest)
{
    if (!isMetalTarget(targetRequest))
        return true;

    result->metalRecordHeaderSize = size_t(kStructuralRayTracingMetalRecordHeaderSize);

    auto payloadCount = result->payloads.getCount();
    for (Index payloadIndex = 0; payloadIndex < payloadCount; ++payloadIndex)
    {
        const StructuralRayTracingDescriptorResourceKind payloadResourceKinds[] = {
            StructuralRayTracingDescriptorResourceKind::IntersectionFunctionTable,
            StructuralRayTracingDescriptorResourceKind::MissVisibleFunctionTable,
            StructuralRayTracingDescriptorResourceKind::ClosestHitVisibleFunctionTable,
        };
        for (auto kind : payloadResourceKinds)
        {
            StructuralRayTracingProgramSchemaReflection::DescriptorResource resource;
            resource.kind = kind;
            resource.payloadIndex = payloadIndex;
            resource.name = getStructuralRayTracingMetalDescriptorResourceName(
                kind,
                payloadIndex,
                payloadCount);
            result->descriptorResources.add(_Move(resource));
        }
    }
    for (auto kind :
         {StructuralRayTracingDescriptorResourceKind::CallableVisibleFunctionTable,
          StructuralRayTracingDescriptorResourceKind::Records})
    {
        StructuralRayTracingProgramSchemaReflection::DescriptorResource resource;
        resource.kind = kind;
        resource.name = getStructuralRayTracingMetalDescriptorResourceName(kind, -1, payloadCount);
        result->descriptorResources.add(_Move(resource));
    }

    result->hitRecordStride = result->metalRecordHeaderSize;
    result->missRecordStride = result->metalRecordHeaderSize;
    for (auto payload : result->payloads)
    {
        for (auto group : payload->hitGroups)
        {
            size_t stride = 0;
            if (!_tryGetMetalRecordStride(targetRequest, group->recordType, stride))
                return false;
            result->hitRecordStride = Math::Max(result->hitRecordStride, stride);
        }
        for (auto shader : payload->missShaders)
        {
            size_t stride = 0;
            if (!_tryGetMetalRecordStride(targetRequest, shader->recordType, stride))
                return false;
            result->missRecordStride = Math::Max(result->missRecordStride, stride);
        }
    }

    result->callableRecordStride = result->metalRecordHeaderSize;
    for (auto shader : result->callableShaders)
    {
        size_t stride = 0;
        if (!_tryGetMetalRecordStride(targetRequest, shader->recordType, stride))
            return false;
        result->callableRecordStride = Math::Max(result->callableRecordStride, stride);
    }
    return true;
}

StructuralRayTracingProgramSchemaReflection* findStructuralRayTracingProgramSchemaReflection(
    ProgramLayout* programLayout,
    const char* name)
{
    if (!programLayout || !name)
        return nullptr;

    auto program = programLayout->getProgram();
    auto linkage = program->getLinkage();
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return nullptr;

    DiagnosticSink sink(linkage->getSourceManager(), Lexer::sourceLocationLexer);
    Type* schemaType = nullptr;
    try
    {
        schemaType = program->getTypeFromString(name, &sink);
    }
    catch (...)
    {
        return nullptr;
    }
    schemaType = schemaType ? as<Type>(schemaType->resolve()) : nullptr;
    if (!schemaType || as<ErrorType>(schemaType))
        return nullptr;

    auto reflectionData = as<StructuralRayTracingReflectionData>(
        programLayout->structuralRayTracingReflectionData.Ptr());
    if (!reflectionData)
    {
        reflectionData = new StructuralRayTracingReflectionData();
        programLayout->structuralRayTracingReflectionData = RefPtr<RefObject>(reflectionData);
    }
    for (auto existing : reflectionData->programSchemas)
    {
        if (existing->schemaType == schemaType)
            return existing;
    }

    auto astBuilder = linkage->getASTBuilder();
    auto schemaInterface =
        registry.getMetadataInterface(StructuralRayTracingMetadataKind::TraceProgramSchema);
    auto schemaInterfaceType =
        schemaInterface ? DeclRefType::create(astBuilder, makeDeclRef(schemaInterface)) : nullptr;
    if (!schemaInterfaceType)
        return nullptr;

    auto sharedSemanticsContext = linkage->getSemanticsForReflection();
    SemanticsContext semanticsContext(sharedSemanticsContext);
    SemanticsVisitor visitor(semanticsContext);
    auto schemaWitness = visitor.isSubtype(schemaType, schemaInterfaceType, IsSubTypeOptions::None);
    if (!schemaWitness)
        return nullptr;

    auto traceContextType = registry.resolveAssociatedType(
        astBuilder,
        schemaWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramTraceContext);
    auto hitGroupsType = registry.resolveAssociatedType(
        astBuilder,
        schemaWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramHitGroups);
    auto missShadersType = registry.resolveAssociatedType(
        astBuilder,
        schemaWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramMissShaders);
    auto callableShadersType = registry.resolveAssociatedType(
        astBuilder,
        schemaWitness,
        StructuralRayTracingAssociatedTypeKind::ProgramCallableShaders);
    if (!traceContextType || !hitGroupsType || !missShadersType || !callableShadersType)
        return nullptr;

    RefPtr<StructuralRayTracingProgramSchemaReflection> result =
        new StructuralRayTracingProgramSchemaReflection();
    result->name = getStructuralRayTracingSourceTypeName(astBuilder, schemaType);
    if (result->name.getLength() == 0)
        return nullptr;
    result->schemaType = schemaType;
    result->traceContextType = traceContextType;
    StructuralRayTracingOpenSectionInfo openSectionInfo;
    bool hasOpenHitGroups = registry.tryGetOpenSectionInfo(
        astBuilder,
        hitGroupsType,
        StructuralRayTracingSectionKind::HitGroups,
        openSectionInfo);
    bool hasOpenMissShaders = registry.tryGetOpenSectionInfo(
        astBuilder,
        missShadersType,
        StructuralRayTracingSectionKind::MissShaders,
        openSectionInfo);
    bool hasOpenCallableShaders = registry.tryGetOpenSectionInfo(
        astBuilder,
        callableShadersType,
        StructuralRayTracingSectionKind::CallableShaders,
        openSectionInfo);
    bool hasOpenSection = hasOpenHitGroups || hasOpenMissShaders || hasOpenCallableShaders;

    bool entriesAdded = false;
    RefPtr<IRModule> manifest;
    IRStructuralRayTracingProgramSchema* finalizedSchema = nullptr;
    auto targetRequest = programLayout->getTargetReq();
    bool needsNativeABISizes =
        isD3DTarget(targetRequest) || isKhronosTarget(targetRequest) || isCUDATarget(targetRequest);
    if (hasOpenSection || needsNativeABISizes)
    {
        manifest = _getFinalizedStructuralRayTracingProgramSchemaManifest(
            programLayout,
            schemaWitness,
            &sink);
        auto schemaTypeIdentity = getMangledTypeName(astBuilder, schemaType->getCanonicalType());
        finalizedSchema = manifest ? _findFinalizedStructuralRayTracingProgramSchema(
                                         manifest,
                                         schemaTypeIdentity.getUnownedSlice())
                                   : nullptr;
    }
    if (hasOpenSection)
    {
        entriesAdded = sink.getErrorCount() == 0 && finalizedSchema &&
                       _addFinalizedStructuralRayTracingProgramSchema(
                           result,
                           programLayout->getProgram(),
                           linkage,
                           registry,
                           finalizedSchema);
    }
    else
    {
        // Closed schemas still enumerate entries through their checked AST lists. A native ABI
        // query may create a target manifest above, but that manifest supplies only the
        // target-specialized types used for size calculation; it does not become a second source
        // of list membership or function indices.
        entriesAdded = _addHitGroups(result, astBuilder, registry, hitGroupsType) &&
                       _addMissShaders(result, astBuilder, registry, missShadersType) &&
                       _addCallableShaders(result, astBuilder, registry, callableShadersType);
    }

    if (sink.getErrorCount() != 0 || !entriesAdded ||
        !_populateStructuralRayTracingTypeLayouts(result, targetRequest) ||
        !_populateNativeRayTracingABISizes(
            result,
            astBuilder,
            targetRequest,
            manifest,
            finalizedSchema) ||
        !_populateMetalFunctionReflection(result, astBuilder, registry, targetRequest) ||
        !_populateMetalRecordStrides(result, targetRequest))
    {
        return nullptr;
    }

    reflectionData->programSchemas.add(result);
    return result;
}

} // namespace Slang
