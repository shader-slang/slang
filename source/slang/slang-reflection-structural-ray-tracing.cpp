#include "slang-reflection-structural-ray-tracing.h"

#include "slang-check-impl.h"
#include "slang-ir-insts.h"
#include "slang-ir-link.h"
#include "slang-ir-optix-ray-tracing-abi.h"
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

struct _StructuralRayTracingHitGroupContract
{
    RefPtr<StructuralRayTracingHitGroupReflection> reflection;
    SubtypeWitness* contextWitness = nullptr;
    Type* payloadType = nullptr;
};

// Resolves the source contract shared by schema reflection and the schema-free catalogue.
//
// Consider `struct Glass : IMaterialHit`, where `IMaterialHit : rt::IHitGroup`. The tagged-
// conformance producer records `Glass` before DCE, while checked AST semantics still own its
// context, primitive, record, and stages. This function projects those associated types from the
// exact `Glass : rt::IHitGroup` witness. It deliberately does not assign a function-table index or
// validate a schema-wide trace context: only a concrete schema has either concept.
static bool _createHitGroupReflection(
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* groupType,
    SubtypeWitness* groupWitness,
    _StructuralRayTracingHitGroupContract& outContract)
{
    auto contextType = registry.resolveAssociatedType(
        astBuilder,
        groupWitness,
        StructuralRayTracingAssociatedTypeKind::HitGroupContext);
    auto contextWitness = registry.resolveAssociatedTypeConstraint(
        astBuilder,
        groupWitness,
        StructuralRayTracingAssociatedTypeKind::HitGroupContext);
    if (!contextType || !contextWitness)
        return false;

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

    RefPtr<StructuralRayTracingHitGroupReflection> group =
        new StructuralRayTracingHitGroupReflection();
    group->groupType = groupType;
    group->contextType = contextType;
    group->recordType = recordType;
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

    outContract.reflection = group;
    outContract.contextWitness = contextWitness;
    outContract.payloadType = payloadType;
    return true;
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
    _StructuralRayTracingHitGroupContract contract;
    if (!_createHitGroupReflection(astBuilder, registry, groupType, groupWitness, contract) ||
        !_doesContextBelongToSchema(result, astBuilder, registry, contract.contextWitness))
    {
        return false;
    }

    auto payload = _findOrAddPayload(result, contract.payloadType);
    if (functionIndex >= 0 && functionIndex != payload->hitGroups.getCount())
        return false;
    auto group = contract.reflection;
    group->functionIndex = functionIndex >= 0 ? functionIndex : payload->hitGroups.getCount();
    group->isLinked = isLinked;
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

struct _StructuralRayTracingMissShaderContract
{
    RefPtr<StructuralRayTracingMissShaderReflection> reflection;
    SubtypeWitness* contextWitness = nullptr;
    Type* payloadType = nullptr;
};

// Resolves the source-level miss declaration without imposing schema membership or slot order.
// The schema-free catalogue and finalized schemas therefore agree on the declaration's context,
// payload, record, and stage while retaining independent reflection objects.
static bool _createMissShaderReflection(
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* shaderType,
    SubtypeWitness* shaderWitness,
    _StructuralRayTracingMissShaderContract& outContract)
{
    auto contextType = registry.resolveAssociatedType(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::MissShaderContext);
    auto contextWitness = registry.resolveAssociatedTypeConstraint(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::MissShaderContext);
    if (!contextType || !contextWitness)
        return false;

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

    RefPtr<StructuralRayTracingMissShaderReflection> shader =
        new StructuralRayTracingMissShaderReflection();
    shader->shaderType = shaderType;
    shader->contextType = contextType;
    shader->recordType = recordType;
    shader->miss =
        _createStageReflection(astBuilder, shaderType, StructuralRayTracingStageKind::Miss);
    if (!shader->miss)
        return false;

    outContract.reflection = shader;
    outContract.contextWitness = contextWitness;
    outContract.payloadType = payloadType;
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
    _StructuralRayTracingMissShaderContract contract;
    if (!_createMissShaderReflection(astBuilder, registry, shaderType, shaderWitness, contract) ||
        !_doesContextBelongToSchema(result, astBuilder, registry, contract.contextWitness))
    {
        return false;
    }

    auto payload = _findOrAddPayload(result, contract.payloadType);
    if (functionIndex >= 0 && functionIndex != payload->missShaders.getCount())
        return false;
    auto shader = contract.reflection;
    shader->functionIndex = functionIndex >= 0 ? functionIndex : payload->missShaders.getCount();
    shader->isLinked = isLinked;
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

struct _StructuralRayTracingCallableShaderContract
{
    RefPtr<StructuralRayTracingCallableShaderReflection> reflection;
    SubtypeWitness* contextWitness = nullptr;
};

// Resolves one callable declaration independently of any schema-wide callable table.
// In particular, callable declarations with different `CallableData` types can coexist in the
// catalogue. A concrete schema still rejects selecting them into one native callable table below.
static bool _createCallableShaderReflection(
    ASTBuilder* astBuilder,
    const StructuralRayTracingDeclRegistry& registry,
    Type* shaderType,
    SubtypeWitness* shaderWitness,
    _StructuralRayTracingCallableShaderContract& outContract)
{
    auto contextType = registry.resolveAssociatedType(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::CallableShaderContext);
    auto contextWitness = registry.resolveAssociatedTypeConstraint(
        astBuilder,
        shaderWitness,
        StructuralRayTracingAssociatedTypeKind::CallableShaderContext);
    if (!contextType || !contextWitness)
        return false;

    RefPtr<StructuralRayTracingCallableShaderReflection> shader =
        new StructuralRayTracingCallableShaderReflection();
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
    shader->callable =
        _createStageReflection(astBuilder, shaderType, StructuralRayTracingStageKind::Callable);
    if (!shader->recordType || !shader->callableDataType || !shader->callable)
        return false;

    outContract.reflection = shader;
    outContract.contextWitness = contextWitness;
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
    _StructuralRayTracingCallableShaderContract contract;
    if (!_createCallableShaderReflection(
            astBuilder,
            registry,
            shaderType,
            shaderWitness,
            contract) ||
        !_doesContextBelongToSchema(result, astBuilder, registry, contract.contextWitness))
    {
        return false;
    }

    auto shader = contract.reflection;
    if (functionIndex >= 0 && functionIndex != result->callableShaders.getCount())
        return false;
    shader->functionIndex = functionIndex >= 0 ? functionIndex : result->callableShaders.getCount();
    shader->isLinked = isLinked;
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

// Reads the canonical type identity from any producer-owned structural entry record. Consumers
// select the record by semantic section kind rather than assuming a shared operand position.
static IRStringLit* _getStructuralRayTracingEntryTypeIdentity(
    IRDecoration* entryInfo,
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return cast<IRStructuralRayTracingHitGroupInfoDecoration>(entryInfo)
            ->getGroupTypeIdentity();
    case StructuralRayTracingSectionKind::MissShaders:
        return cast<IRStructuralRayTracingMissShaderInfoDecoration>(entryInfo)
            ->getMissTypeIdentity();
    case StructuralRayTracingSectionKind::CallableShaders:
        return cast<IRStructuralRayTracingCallableShaderInfoDecoration>(entryInfo)
            ->getCallableTypeIdentity();
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
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
        IRDecoration* entryInfo = nullptr;
        switch (kind)
        {
        case StructuralRayTracingSectionKind::HitGroups:
            entryInfo = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
            break;
        case StructuralRayTracingSectionKind::MissShaders:
            entryInfo = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration);
            break;
        case StructuralRayTracingSectionKind::CallableShaders:
            entryInfo = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
            break;
        default:
            SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
        }
        if (!entryInfo ||
            _getStructuralRayTracingEntryTypeIdentity(entryInfo, kind)->getStringSlice() !=
                typeIdentity)
        {
            continue;
        }
        // Completion deduplicates a section by this canonical identity before adding an entry.
        SLANG_RELEASE_ASSERT(!result);
        result = entryInfo;
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
// the fixed-index IFT that the host must construct. AnyHit and Intersection do not have
// independently bindable Metal symbols: the compiler folds them into one dispatcher per payload
// and geometry kind, including reject-all dispatchers for absent triangle or bounding-box kinds.
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
        bool hasCurve = false;
        bool hasCandidateLogic = false;
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
            else
            {
                // Source reflection keeps the logical placeholder absent, while every placeholder
                // index names the same physical no-op. A dense Metal VFT is then complete even for
                // a payload partition whose hit groups all use `NoClosestHit`.
                group->closestHitEntryPointName =
                    getStructuralRayTracingMetalNoOpClosestHitFunctionName(
                        schemaSourceTypeName,
                        payloadIndex);
            }
            if (group->anyHit)
                group->anyHit->entryPointName = String();
            if (group->intersection)
                group->intersection->entryPointName = String();

            switch (registry.getHitAttributesKind(group->primitiveType))
            {
            case StructuralRayTracingHitAttributesKind::Triangle:
                hasCandidateLogic |= group->anyHit != nullptr;
                break;
            case StructuralRayTracingHitAttributesKind::Curve:
                hasCurve = true;
                hasCandidateLogic |= group->anyHit != nullptr;
                break;
            case StructuralRayTracingHitAttributesKind::Custom:
                hasCandidateLogic |= group->intersection != nullptr;
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

        // A payload with no candidate logic does not consume an IFT. Once candidate logic exists,
        // however, traces of that payload pass one table for every traversed geometry. Always
        // expose exported triangle and bounding-box dispatchers at indices zero and one: a kind
        // absent from the schema is implemented by a reject-all stub, preventing an unrelated
        // record function index from silently accepting a candidate of the wrong primitive kind.
        if (!hasCandidateLogic)
            continue;

        _addMetalIntersectionFunctionReflection(
            payload,
            schemaSourceTypeName,
            payloadIndex,
            StructuralRayTracingMetalCandidateKind::Triangle,
            StructuralRayTracingMetalIntersectionFunctionImplementationKind::ExportedFunction);
        _addMetalIntersectionFunctionReflection(
            payload,
            schemaSourceTypeName,
            payloadIndex,
            StructuralRayTracingMetalCandidateKind::BoundingBox,
            StructuralRayTracingMetalIntersectionFunctionImplementationKind::ExportedFunction);
        if (hasCurve)
        {
            _addMetalIntersectionFunctionReflection(
                payload,
                schemaSourceTypeName,
                payloadIndex,
                StructuralRayTracingMetalCandidateKind::Curve,
                StructuralRayTracingMetalIntersectionFunctionImplementationKind::ExportedFunction);
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

// Returns the public layout used for one structural application's record data. Metal reads the
// data from a raw record buffer using structured-buffer rules; other targets expose their ordinary
// record layout. Both schema reflection and the schema-free catalogue call this single helper so a
// declaration does not acquire a different record ABI merely because a schema selected it.
static TypeLayout* _getStructuralRayTracingRecordTypeLayout(
    TargetRequest* targetRequest,
    Type* recordType)
{
    auto recordRules = isMetalTarget(targetRequest) ? slang::LayoutRules::DefaultStructuredBuffer
                                                    : slang::LayoutRules::Default;
    return targetRequest->getTypeLayout(recordType, recordRules);
}

// Resolves public layouts only after manifest identity has selected the AST types. Payloads use
// the target's ordinary reflected layout. Records use the shared target rule above.
static bool _populateStructuralRayTracingTypeLayouts(
    StructuralRayTracingProgramSchemaReflection* result,
    TargetRequest* targetRequest)
{
    for (auto payload : result->payloads)
    {
        payload->typeLayout =
            targetRequest->getTypeLayout(payload->payloadType, slang::LayoutRules::Default);
        if (!payload->typeLayout)
            return false;
        for (auto group : payload->hitGroups)
        {
            group->recordTypeLayout =
                _getStructuralRayTracingRecordTypeLayout(targetRequest, group->recordType);
            if (!group->recordTypeLayout)
                return false;
        }
        for (auto shader : payload->missShaders)
        {
            shader->recordTypeLayout =
                _getStructuralRayTracingRecordTypeLayout(targetRequest, shader->recordType);
            if (!shader->recordTypeLayout)
                return false;
        }
    }
    for (auto shader : result->callableShaders)
    {
        shader->recordTypeLayout =
            _getStructuralRayTracingRecordTypeLayout(targetRequest, shader->recordType);
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

static StructuralRayTracingReflectionData* _getOrCreateStructuralRayTracingReflectionData(
    ProgramLayout* programLayout)
{
    auto reflectionData = as<StructuralRayTracingReflectionData>(
        programLayout->structuralRayTracingReflectionData.Ptr());
    if (!reflectionData)
    {
        reflectionData = new StructuralRayTracingReflectionData();
        programLayout->structuralRayTracingReflectionData = RefPtr<RefObject>(reflectionData);
    }
    return reflectionData;
}

struct _StructuralRayTracingEntryCatalogueCandidate
{
    String typeIdentity;
    String declLookupName;
};

struct _StructuralRayTracingEntryCatalogueCandidates
{
    List<_StructuralRayTracingEntryCatalogueCandidate> hitGroups;
    List<_StructuralRayTracingEntryCatalogueCandidate> missShaders;
    List<_StructuralRayTracingEntryCatalogueCandidate> callableShaders;
};

static List<_StructuralRayTracingEntryCatalogueCandidate>&
_getStructuralRayTracingEntryCatalogueCandidateList(
    _StructuralRayTracingEntryCatalogueCandidates& candidates,
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return candidates.hitGroups;
    case StructuralRayTracingSectionKind::MissShaders:
        return candidates.missShaders;
    case StructuralRayTracingSectionKind::CallableShaders:
        return candidates.callableShaders;
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

// Finds the semantic entry record placed on a tagged conformance by AST-to-IR lowering. The
// records have intentionally different operand layouts, so this lookup is by typed decoration and
// section role rather than by an operand index shared accidentally by two record kinds.
static IRDecoration* _findStructuralRayTracingTaggedConformanceEntryInfo(
    IRInst* conformanceOwner,
    StructuralRayTracingSectionKind kind)
{
    switch (kind)
    {
    case StructuralRayTracingSectionKind::HitGroups:
        return conformanceOwner->findDecoration<IRStructuralRayTracingHitGroupInfoDecoration>();
    case StructuralRayTracingSectionKind::MissShaders:
        return conformanceOwner->findDecoration<IRStructuralRayTracingMissShaderInfoDecoration>();
    case StructuralRayTracingSectionKind::CallableShaders:
        return conformanceOwner
            ->findDecoration<IRStructuralRayTracingCallableShaderInfoDecoration>();
    default:
        SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
    }
}

// Canonical identity is both the deduplication key and the catalogue order. A composite can visit
// the same declaration through more than one component, and one conformance can carry several
// open tags for the same section. Neither should create multiple host-visible declarations.
static void _sortAndDeduplicateStructuralRayTracingEntryCatalogueCandidates(
    List<_StructuralRayTracingEntryCatalogueCandidate>& candidates)
{
    candidates.sort(
        [](const _StructuralRayTracingEntryCatalogueCandidate& left,
           const _StructuralRayTracingEntryCatalogueCandidate& right) {
            return compare(
                       left.typeIdentity.getUnownedSlice(),
                       right.typeIdentity.getUnownedSlice()) < 0;
        });

    List<_StructuralRayTracingEntryCatalogueCandidate> uniqueCandidates;
    for (const auto& candidate : candidates)
    {
        if (uniqueCandidates.getCount() != 0 &&
            uniqueCandidates.getLast().typeIdentity == candidate.typeIdentity)
        {
            // Canonical type identity is injective. A different declaration lookup key for the
            // same identity would mean that the producer serialized two representations for one
            // semantic type; reflection must not choose one arbitrarily.
            SLANG_RELEASE_ASSERT(
                uniqueCandidates.getLast().declLookupName == candidate.declLookupName);
            continue;
        }
        uniqueCandidates.add(candidate);
    }
    candidates.swapWith(uniqueCandidates);
}

// Collects declaration identities from the pre-DCE module index. This query deliberately does not
// link a manifest and does not scan target-optimized IR. Merely asking what declarations exist
// therefore cannot introduce a liveness root for their stage implementations.
static void _collectStructuralRayTracingEntryCatalogueCandidates(
    ComponentType* program,
    _StructuralRayTracingEntryCatalogueCandidates& candidates)
{
    program->enumerateIRModules(
        [&](IRModule* module)
        {
            auto linkingInfo = module->_getOrCreateLinkingInfo();
            for (auto conformanceOwner : linkingInfo->getStructuralRayTracingTaggedConformances())
            {
                UInt recordedKindMask = 0;
                for (auto decoration : conformanceOwner->getDecorations())
                {
                    auto tagged = as<IRStructuralRayTracingTaggedConformanceDecoration>(decoration);
                    if (!tagged)
                        continue;

                    auto kindValue = tagged->getSectionKind()->getValue();
                    SLANG_RELEASE_ASSERT(
                        kindValue >= 0 &&
                        kindValue < IRIntegerValue(StructuralRayTracingSectionKind::Count));
                    auto kind = StructuralRayTracingSectionKind(kindValue);
                    UInt kindBit = UInt(1) << UInt(kindValue);
                    if (recordedKindMask & kindBit)
                        continue;
                    recordedKindMask |= kindBit;

                    auto entryInfo =
                        _findStructuralRayTracingTaggedConformanceEntryInfo(conformanceOwner, kind);
                    SLANG_RELEASE_ASSERT(entryInfo);
                    auto identity = _getStructuralRayTracingEntryTypeIdentity(entryInfo, kind);
                    auto lookupName = _getStructuralRayTracingEntryDeclLookupName(entryInfo, kind);
                    SLANG_RELEASE_ASSERT(
                        identity && identity->getStringSlice().getLength() != 0 && lookupName &&
                        lookupName->getStringSlice().getLength() != 0);

                    _StructuralRayTracingEntryCatalogueCandidate candidate;
                    candidate.typeIdentity = identity->getStringSlice();
                    candidate.declLookupName = lookupName->getStringSlice();
                    _getStructuralRayTracingEntryCatalogueCandidateList(candidates, kind)
                        .add(_Move(candidate));
                }
            }
        });

    _sortAndDeduplicateStructuralRayTracingEntryCatalogueCandidates(candidates.hitGroups);
    _sortAndDeduplicateStructuralRayTracingEntryCatalogueCandidates(candidates.missShaders);
    _sortAndDeduplicateStructuralRayTracingEntryCatalogueCandidates(candidates.callableShaders);
}

static void _clearSchemaSpecificMetalEntryPointNames(StructuralRayTracingStageReflection* stage)
{
    if (stage)
        stage->entryPointName = String();
}

// Metal stage symbols encode a schema, payload partition, and function index. A declaration-only
// object has none of those values, so publishing its portable source-derived adapter name as a
// physical Metal symbol would be misleading. The stage type and kind remain available; querying a
// finalized schema returns a different object populated with the exact bindable symbol.
static void _clearSchemaSpecificMetalEntryPointNames(StructuralRayTracingHitGroupReflection* group)
{
    group->closestHitEntryPointName = String();
    _clearSchemaSpecificMetalEntryPointNames(group->closestHit);
    _clearSchemaSpecificMetalEntryPointNames(group->anyHit);
    _clearSchemaSpecificMetalEntryPointNames(group->intersection);
}

static bool _addStructuralRayTracingEntryCatalogueSection(
    StructuralRayTracingEntryCatalogueReflection* result,
    ComponentType* program,
    StructuralRayTracingDeclRegistry& registry,
    StructuralRayTracingSectionKind kind,
    const List<_StructuralRayTracingEntryCatalogueCandidate>& candidates,
    TargetRequest* targetRequest)
{
    auto linkage = program->getLinkage();
    auto astBuilder = linkage->getASTBuilder();
    for (const auto& candidate : candidates)
    {
        auto entryType = _findOrRecoverStructuralRayTracingReflectionType(
            program,
            registry,
            candidate.typeIdentity.getUnownedSlice(),
            candidate.declLookupName.getUnownedSlice());
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
                _StructuralRayTracingHitGroupContract contract;
                if (!_createHitGroupReflection(
                        astBuilder,
                        registry,
                        entryType,
                        entryWitness,
                        contract))
                {
                    return false;
                }
                auto group = contract.reflection;
                group->recordTypeLayout =
                    _getStructuralRayTracingRecordTypeLayout(targetRequest, group->recordType);
                if (!group->recordTypeLayout)
                    return false;
                if (isMetalTarget(targetRequest))
                    _clearSchemaSpecificMetalEntryPointNames(group);
                result->hitGroups.add(group);
                break;
            }
        case StructuralRayTracingSectionKind::MissShaders:
            {
                _StructuralRayTracingMissShaderContract contract;
                if (!_createMissShaderReflection(
                        astBuilder,
                        registry,
                        entryType,
                        entryWitness,
                        contract))
                {
                    return false;
                }
                auto shader = contract.reflection;
                shader->recordTypeLayout =
                    _getStructuralRayTracingRecordTypeLayout(targetRequest, shader->recordType);
                if (!shader->recordTypeLayout)
                    return false;
                if (isMetalTarget(targetRequest))
                    _clearSchemaSpecificMetalEntryPointNames(shader->miss);
                result->missShaders.add(shader);
                break;
            }
        case StructuralRayTracingSectionKind::CallableShaders:
            {
                _StructuralRayTracingCallableShaderContract contract;
                if (!_createCallableShaderReflection(
                        astBuilder,
                        registry,
                        entryType,
                        entryWitness,
                        contract))
                {
                    return false;
                }
                auto shader = contract.reflection;
                shader->recordTypeLayout =
                    _getStructuralRayTracingRecordTypeLayout(targetRequest, shader->recordType);
                if (!shader->recordTypeLayout)
                    return false;
                if (isMetalTarget(targetRequest))
                    _clearSchemaSpecificMetalEntryPointNames(shader->callable);
                result->callableShaders.add(shader);
                break;
            }
        default:
            SLANG_UNEXPECTED("invalid structural ray-tracing section kind");
        }
    }
    return true;
}

StructuralRayTracingEntryCatalogueReflection* getStructuralRayTracingEntryCatalogueReflection(
    ProgramLayout* programLayout)
{
    if (!programLayout)
        return nullptr;

    std::lock_guard<std::mutex> reflectionLock(programLayout->structuralRayTracingReflectionMutex);
    auto reflectionData = _getOrCreateStructuralRayTracingReflectionData(programLayout);
    if (reflectionData->entryCatalogue)
        return reflectionData->entryCatalogue;

    auto program = programLayout->getProgram();
    auto linkage = program->getLinkage();
    auto& registry = linkage->getStructuralRayTracingDeclRegistry();
    if (!registry.isInitialized())
        return nullptr;

    _StructuralRayTracingEntryCatalogueCandidates candidates;
    _collectStructuralRayTracingEntryCatalogueCandidates(program, candidates);

    RefPtr<StructuralRayTracingEntryCatalogueReflection> result =
        new StructuralRayTracingEntryCatalogueReflection();
    auto targetRequest = programLayout->getTargetReq();
    if (!_addStructuralRayTracingEntryCatalogueSection(
            result,
            program,
            registry,
            StructuralRayTracingSectionKind::HitGroups,
            candidates.hitGroups,
            targetRequest) ||
        !_addStructuralRayTracingEntryCatalogueSection(
            result,
            program,
            registry,
            StructuralRayTracingSectionKind::MissShaders,
            candidates.missShaders,
            targetRequest) ||
        !_addStructuralRayTracingEntryCatalogueSection(
            result,
            program,
            registry,
            StructuralRayTracingSectionKind::CallableShaders,
            candidates.callableShaders,
            targetRequest))
    {
        return nullptr;
    }

    // Publish only the fully populated catalogue. A failed query therefore cannot cache a partial
    // result, and a later query after the surrounding component has been completed can retry.
    reflectionData->entryCatalogue = result;
    return result;
}

StructuralRayTracingProgramSchemaReflection* findStructuralRayTracingProgramSchemaReflection(
    ProgramLayout* programLayout,
    const char* name)
{
    if (!programLayout || !name)
        return nullptr;

    std::lock_guard<std::mutex> reflectionLock(programLayout->structuralRayTracingReflectionMutex);
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

    auto reflectionData = _getOrCreateStructuralRayTracingReflectionData(programLayout);
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
