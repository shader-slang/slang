#include "slang-reflection-structural-ray-tracing.h"

#include "slang-check-impl.h"
#include "slang-linkable.h"
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

        auto payload = _findOrAddPayload(result, payloadType);

        RefPtr<StructuralRayTracingHitGroupReflection> group =
            new StructuralRayTracingHitGroupReflection();
        group->functionIndex = payload->hitGroups.getCount();
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
        payload->hitGroups.add(group);
    }
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

        auto payload = _findOrAddPayload(result, payloadType);
        RefPtr<StructuralRayTracingMissShaderReflection> shader =
            new StructuralRayTracingMissShaderReflection();
        shader->functionIndex = payload->missShaders.getCount();
        shader->shaderType = shaderType;
        shader->contextType = contextType;
        shader->recordType = recordType;
        shader->miss =
            _createStageReflection(astBuilder, shaderType, StructuralRayTracingStageKind::Miss);
        if (!shader->miss)
            return false;
        payload->missShaders.add(shader);
    }
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
            return false;

        RefPtr<StructuralRayTracingCallableShaderReflection> shader =
            new StructuralRayTracingCallableShaderReflection();
        shader->functionIndex = result->callableShaders.getCount();
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
        if (result->callableShaders.getCount() != 0 &&
            !result->callableShaders[0]->callableDataType->equals(shader->callableDataType))
        {
            // One schema produces one callable VFT on Metal, so reflection must not publish a
            // layout whose entries require incompatible function signatures. Compilation emits
            // the user-facing schema diagnostic at the linked IR validation boundary.
            return false;
        }
        result->callableShaders.add(shader);
    }
    return true;
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

// Publishes the compiler-owned Metal record-buffer ABI through schema reflection. Other targets
// use native shader-table representations whose strides remain host-defined, so their reflected
// values deliberately stay zero.
static bool _populateMetalRecordStrides(
    StructuralRayTracingProgramSchemaReflection* result,
    TargetRequest* targetRequest)
{
    if (!isMetalTarget(targetRequest))
        return true;

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

    result->hitRecordStride = size_t(getStructuralRayTracingMetalRecordStride(/* dataSize */ 0));
    result->missRecordStride = size_t(getStructuralRayTracingMetalRecordStride(/* dataSize */ 0));
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

    result->callableRecordStride =
        size_t(getStructuralRayTracingMetalRecordStride(/* dataSize */ 0));
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
    if (!_addHitGroups(result, astBuilder, registry, hitGroupsType) ||
        !_addMissShaders(result, astBuilder, registry, missShadersType) ||
        !_addCallableShaders(result, astBuilder, registry, callableShadersType) ||
        !_populateMetalFunctionReflection(
            result,
            astBuilder,
            registry,
            programLayout->getTargetReq()) ||
        !_populateMetalRecordStrides(result, programLayout->getTargetReq()))
    {
        return nullptr;
    }

    reflectionData->programSchemas.add(result);
    return result;
}

} // namespace Slang
