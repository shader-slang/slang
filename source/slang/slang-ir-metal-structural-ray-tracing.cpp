#include "slang-ir-metal-structural-ray-tracing.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-structural-ray-tracing.h"
#include "slang-ir-synthesize-structural-ray-tracing.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-structural-ray-tracing.h"
#include "slang-target-program.h"

namespace Slang
{

void prepareMetalStructuralRayTracingEntryPoints(IRModule* module, List<IRFunc*>& ioEntryPoints)
{
    HashSet<IRFunc*> logicalEntryPoints;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            if (func->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
                logicalEntryPoints.add(func);
        }
    }

    List<IRFunc*> selectedEntryPoints = ioEntryPoints;
    ioEntryPoints.clear();
    for (auto entryPoint : selectedEntryPoints)
    {
        if (!logicalEntryPoints.contains(entryPoint))
            ioEntryPoints.add(entryPoint);
    }

    for (auto entryPoint : logicalEntryPoints)
    {
        if (auto decoration = entryPoint->findDecoration<IREntryPointDecoration>())
            decoration->removeAndDeallocate();
    }
}

static bool _supportsMetalLib31(TargetRequest* targetRequest)
{
    auto& options = targetRequest->getOptionSet();
    if (!options.hasOption(CompilerOptionName::Profile) ||
        options.getProfile().getVersion() == ProfileVersion::Unknown)
    {
        return true;
    }
    return targetRequest->getTargetCaps().implies(CapabilityAtom::metallib_3_1);
}

static void _collectStructuralProgramOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectStructuralProgramOperations(child, operations);
        if (child->getOp() == kIROp_StructuralRayTracingTrace ||
            child->getOp() == kIROp_StructuralRayTracingCallShader)
            operations.add(child);
    }
}

static IRInst* _getStructuralRayTracingProgramLayout(IRInst* operation)
{
    if (auto trace = as<IRStructuralRayTracingTrace>(operation))
        return trace->getProgramLayout();
    if (auto call = as<IRStructuralRayTracingCallShader>(operation))
        return call->getProgramLayout();
    SLANG_UNEXPECTED("expected a structural ray-tracing program operation");
}

static IRType* _getStructuralRayTracingAccelerationStructureType(IRInst* operation)
{
    if (auto trace = as<IRStructuralRayTracingTrace>(operation))
        return trace->getAccelerationStructure()->getDataType();
    if (auto call = as<IRStructuralRayTracingCallShader>(operation))
        return call->getAccelerationStructureType();
    SLANG_UNEXPECTED("expected a structural ray-tracing program operation");
}

static IRIntLit* _getStructuralRayTracingMotionKind(IRInst* operation)
{
    if (auto trace = as<IRStructuralRayTracingTrace>(operation))
        return cast<IRIntLit>(trace->getMotionKind());
    if (auto call = as<IRStructuralRayTracingCallShader>(operation))
        return call->getMotionKind();
    SLANG_UNEXPECTED("expected a structural ray-tracing program operation");
}

static bool _isStructuralHitGroupForPayload(
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    IRType* payloadType)
{
    return group->getPayloadType() == payloadType;
}

static bool _isStructuralMissShaderForPayload(
    IRStructuralRayTracingMissShaderInfoDecoration* entry,
    IRType* payloadType)
{
    return entry->getPayloadType() == payloadType;
}

static bool _hasStructuralShaderEntries(IRStructuralRayTracingTrace* trace)
{
    auto payloadType = trace->getPayloadType();
    for (auto decoration = trace->getFirstDecoration(); decoration;
         decoration = decoration->getNextDecoration())
    {
        if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
        {
            if (_isStructuralHitGroupForPayload(group, payloadType))
                return true;
        }
        else if (auto entry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
        {
            if (_isStructuralMissShaderForPayload(entry, payloadType))
                return true;
        }
    }
    return false;
}

static IRFunc* _findEnclosingFunc(IRInst* inst)
{
    for (auto parent = inst->getParent(); parent; parent = parent->getParent())
    {
        if (auto func = as<IRFunc>(parent))
            return func;
    }
    return nullptr;
}

struct MetalStageRequirements
{
    bool record = false;
    bool callableDispatch = false;
    bool hitAttributes = false;
    bool triangleBarycentricCoord = false;
    bool triangleFrontFacing = false;
    bool curveParameter = false;
    bool minDistance = false;
    bool distance = false;
    bool rayTime = false;
    bool rayFlags = false;
    bool hitKind = false;
    bool worldSpaceOrigin = false;
    bool worldSpaceDirection = false;
    bool objectSpaceRay = false;
    bool objectToWorld = false;
    bool worldToObject = false;
    bool dispatchRaysIndex = false;
    bool dispatchRaysDimensions = false;
    bool primitiveIndex = false;
    bool geometryIndex = false;
    bool instanceIndex = false;
    bool instanceID = false;
};

static void _collectMetalStageRequirements(
    IRFunc* function,
    MetalStageRequirements& requirements,
    HashSet<IRFunc*>& visited)
{
    if (!function || !visited.add(function))
        return;

    for (auto block : function->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (auto call = as<IRCall>(inst))
            {
                _collectMetalStageRequirements(
                    as<IRFunc>(call->getCallee()),
                    requirements,
                    visited);
            }
            switch (inst->getOp())
            {
            case kIROp_StructuralRayTracingCallShader:
            case kIROp_MetalStructuralRayTracingCallShader:
                requirements.callableDispatch = true;
                break;
            case kIROp_StructuralRayTracingGetRecord:
                requirements.record = true;
                break;
            case kIROp_StructuralRayTracingGetHitAttributes:
                requirements.hitAttributes = true;
                break;
            case kIROp_StructuralRayTracingGetTriangleBarycentricCoord:
                requirements.triangleBarycentricCoord = true;
                break;
            case kIROp_StructuralRayTracingGetTriangleFrontFacing:
                requirements.triangleFrontFacing = true;
                break;
            case kIROp_StructuralRayTracingGetCurveParameter:
                requirements.curveParameter = true;
                break;
            case kIROp_StructuralRayTracingGetRayTMin:
                requirements.minDistance = true;
                break;
            case kIROp_StructuralRayTracingGetRayTCurrent:
                requirements.distance = true;
                break;
            case kIROp_StructuralRayTracingGetRayTime:
                requirements.rayTime = true;
                break;
            case kIROp_StructuralRayTracingGetRayFlags:
                requirements.rayFlags = true;
                break;
            case kIROp_StructuralRayTracingGetHitKind:
                requirements.hitKind = true;
                break;
            case kIROp_StructuralRayTracingGetWorldRayOrigin:
                requirements.worldSpaceOrigin = true;
                break;
            case kIROp_StructuralRayTracingGetWorldRayDirection:
                requirements.worldSpaceDirection = true;
                break;
            case kIROp_StructuralRayTracingGetObjectSpaceRay:
                requirements.objectSpaceRay = true;
                break;
            case kIROp_StructuralRayTracingGetPrimitiveIndex:
                requirements.primitiveIndex = true;
                break;
            case kIROp_StructuralRayTracingGetGeometryIndex:
                requirements.geometryIndex = true;
                break;
            case kIROp_StructuralRayTracingGetInstanceIndex:
                requirements.instanceIndex = true;
                break;
            case kIROp_StructuralRayTracingGetInstanceID:
                requirements.instanceID = true;
                break;
            case kIROp_StructuralRayTracingGetObjectToWorld:
                requirements.objectToWorld = true;
                break;
            case kIROp_StructuralRayTracingGetWorldToObject:
                requirements.worldToObject = true;
                break;
            case kIROp_StructuralRayTracingGetDispatchRaysIndex:
                requirements.dispatchRaysIndex = true;
                break;
            case kIROp_StructuralRayTracingGetDispatchRaysDimensions:
                requirements.dispatchRaysDimensions = true;
                break;
            default:
                break;
            }
        }
    }
}

static MetalStageRequirements _getMetalStageRequirements(IRInst* invokeValue)
{
    MetalStageRequirements result;
    HashSet<IRFunc*> visited;
    _collectMetalStageRequirements(as<IRFunc>(invokeValue), result, visited);
    return result;
}

static MetalStageRequirements _combineMetalStageRequirements(
    const MetalStageRequirements& left,
    const MetalStageRequirements& right)
{
    MetalStageRequirements result;
#define SLANG_COMBINE_REQUIREMENT(NAME) result.NAME = left.NAME || right.NAME
    SLANG_COMBINE_REQUIREMENT(record);
    SLANG_COMBINE_REQUIREMENT(callableDispatch);
    SLANG_COMBINE_REQUIREMENT(hitAttributes);
    SLANG_COMBINE_REQUIREMENT(triangleBarycentricCoord);
    SLANG_COMBINE_REQUIREMENT(triangleFrontFacing);
    SLANG_COMBINE_REQUIREMENT(curveParameter);
    SLANG_COMBINE_REQUIREMENT(minDistance);
    SLANG_COMBINE_REQUIREMENT(distance);
    SLANG_COMBINE_REQUIREMENT(rayTime);
    SLANG_COMBINE_REQUIREMENT(rayFlags);
    SLANG_COMBINE_REQUIREMENT(hitKind);
    SLANG_COMBINE_REQUIREMENT(worldSpaceOrigin);
    SLANG_COMBINE_REQUIREMENT(worldSpaceDirection);
    SLANG_COMBINE_REQUIREMENT(objectSpaceRay);
    SLANG_COMBINE_REQUIREMENT(objectToWorld);
    SLANG_COMBINE_REQUIREMENT(worldToObject);
    SLANG_COMBINE_REQUIREMENT(dispatchRaysIndex);
    SLANG_COMBINE_REQUIREMENT(dispatchRaysDimensions);
    SLANG_COMBINE_REQUIREMENT(primitiveIndex);
    SLANG_COMBINE_REQUIREMENT(geometryIndex);
    SLANG_COMBINE_REQUIREMENT(instanceIndex);
    SLANG_COMBINE_REQUIREMENT(instanceID);
#undef SLANG_COMBINE_REQUIREMENT
    return result;
}

static UInt _getMetalStageRequirementMask(const MetalStageRequirements& requirements)
{
    UInt result = 0;
#define SLANG_ADD_REQUIREMENT(NAME, ENUM_NAME) \
    if (requirements.NAME)                     \
    result |= UInt(MetalStructuralRayTracingStageRequirement::ENUM_NAME)
    SLANG_ADD_REQUIREMENT(record, Record);
    SLANG_ADD_REQUIREMENT(callableDispatch, CallableDispatch);
    SLANG_ADD_REQUIREMENT(hitAttributes, HitAttributes);
    SLANG_ADD_REQUIREMENT(triangleBarycentricCoord, TriangleBarycentricCoord);
    SLANG_ADD_REQUIREMENT(triangleFrontFacing, TriangleFrontFacing);
    SLANG_ADD_REQUIREMENT(curveParameter, CurveParameter);
    SLANG_ADD_REQUIREMENT(distance, Distance);
    SLANG_ADD_REQUIREMENT(hitKind, HitKind);
    SLANG_ADD_REQUIREMENT(worldSpaceOrigin, WorldSpaceOrigin);
    SLANG_ADD_REQUIREMENT(worldSpaceDirection, WorldSpaceDirection);
    SLANG_ADD_REQUIREMENT(primitiveIndex, PrimitiveIndex);
    SLANG_ADD_REQUIREMENT(geometryIndex, GeometryIndex);
    SLANG_ADD_REQUIREMENT(instanceIndex, InstanceIndex);
    SLANG_ADD_REQUIREMENT(instanceID, InstanceID);
    SLANG_ADD_REQUIREMENT(objectSpaceRay, ObjectSpaceRay);
    SLANG_ADD_REQUIREMENT(objectToWorld, ObjectToWorld);
    SLANG_ADD_REQUIREMENT(worldToObject, WorldToObject);
#undef SLANG_ADD_REQUIREMENT
    return result;
}

static MetalStageRequirements _getMetalStageRequirements(
    IRInst* schemaOperation,
    StructuralRayTracingStageKind stageKind,
    IRType* payloadType)
{
    MetalStageRequirements result;
    for (auto decoration : schemaOperation->getDecorations())
    {
        IRInst* invoke = nullptr;
        if (stageKind == StructuralRayTracingStageKind::ClosestHit)
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                if (_isStructuralHitGroupForPayload(group, payloadType))
                {
                    invoke = getStructuralRayTracingHitGroupStageInvoke(
                        group,
                        StructuralRayTracingStageKind::ClosestHit);
                }
            }
        }
        else if (stageKind == StructuralRayTracingStageKind::Miss)
        {
            if (auto entry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                if (_isStructuralMissShaderForPayload(entry, payloadType))
                    invoke = entry->getMiss();
            }
        }
        if (invoke)
            result = _combineMetalStageRequirements(result, _getMetalStageRequirements(invoke));
    }
    return result;
}

class MetalRayDataInfo : public RefObject
{
public:
    IRStructType* type = nullptr;
    IRStructKey* payloadKey = nullptr;
    IRStructKey* recordDataKey = nullptr;
    IRStructKey* sbtOffsetKey = nullptr;
    IRStructKey* sbtStrideKey = nullptr;
    IRStructKey* minDistanceKey = nullptr;
    IRStructKey* rayTimeKey = nullptr;
    IRStructKey* rayFlagsKey = nullptr;
    IRStructKey* dispatchRaysIndexKey = nullptr;
    IRStructKey* dispatchRaysDimensionsKey = nullptr;
    IRStructKey* customHitKindKey = nullptr;
    Dictionary<IRInst*, IRStructKey*> customAttributeKeys;
};

static RefPtr<MetalRayDataInfo> _createMetalRayDataInfo(
    IRModule* module,
    IRInst* schemaOperation,
    IRType* payloadType,
    Index payloadIndex,
    bool hasMultiplePayloadPartitions,
    TargetRequest* targetRequest)
{
    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    auto result = RefPtr<MetalRayDataInfo>(new MetalRayDataInfo());
    result->type = builder.createStructType();
    StringBuilder name;
    auto programLayout = _getStructuralRayTracingProgramLayout(schemaOperation);
    if (auto nameHint = programLayout->findDecoration<IRNameHintDecoration>())
        name << nameHint->getName();
    else
        name << "StructuralRayTracingProgram";
    if (hasMultiplePayloadPartitions)
        name << ".payload" << payloadIndex;
    name << ".rayData";
    builder.addNameHintDecoration(result->type, name.getUnownedSlice());

    result->payloadKey = builder.createStructKey();
    builder.addNameHintDecoration(result->payloadKey, UnownedTerminatedStringSlice("payload"));
    builder.createStructField(result->type, result->payloadKey, payloadType);

    // Consider `struct Payload { Empty value; Empty values[2]; }`, where `Empty` has no fields.
    // This is a non-empty source payload, but its target layout has no storage. Metal visible
    // functions still exchange the generated ray-data carrier by pointer, so resource-type
    // legalization must not remove that carrier along with its payload field. Keep the
    // compiler-private ABI carrier concrete whenever the target payload size is zero; the sentinel
    // is never exposed as part of the user's payload.
    IRSizeAndAlignment payloadSizeAndAlignment;
    SLANG_RELEASE_ASSERT(
        SLANG_SUCCEEDED(
            getNaturalSizeAndAlignment(targetRequest, payloadType, &payloadSizeAndAlignment)) &&
        payloadSizeAndAlignment.size != IRSizeAndAlignment::kIndeterminateSize);
    if (payloadSizeAndAlignment.size == 0)
    {
        auto sentinelKey = builder.createStructKey();
        builder.addNameHintDecoration(
            sentinelKey,
            UnownedTerminatedStringSlice("emptyPayloadSentinel"));
        builder.createStructField(result->type, sentinelKey, builder.getUIntType());
    }

    bool needsRecordData = false;
    bool needsMinDistance = false;
    bool needsRayTime = false;
    bool needsRayFlags = false;
    bool needsDispatchRaysIndex = false;
    bool needsDispatchRaysDimensions = false;
    bool needsCustomHitKind = false;
    bool needsCandidateRecordSelection = false;
    for (auto decoration : schemaOperation->getDecorations())
    {
        if (auto missEntry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
        {
            if (!_isStructuralMissShaderForPayload(missEntry, payloadType))
                continue;
            auto requirements = _getMetalStageRequirements(missEntry->getMiss());
            needsRecordData |= requirements.record || requirements.callableDispatch;
            needsMinDistance |= requirements.minDistance || requirements.objectSpaceRay;
            needsRayTime |= requirements.rayTime;
            needsRayFlags |= requirements.rayFlags;
            needsDispatchRaysIndex |= requirements.dispatchRaysIndex;
            needsDispatchRaysDimensions |= requirements.dispatchRaysDimensions;
            continue;
        }
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (!group || !_isStructuralHitGroupForPayload(group, payloadType))
            continue;

        auto closestHitInvoke = getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::ClosestHit);
        auto anyHitInvoke = getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::AnyHit);
        auto intersectionInvoke = getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::Intersection);

        if (StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue()) !=
            StructuralRayTracingHitAttributesKind::Custom)
        {
            auto closestHitRequirements = _getMetalStageRequirements(closestHitInvoke);
            auto anyHitRequirements = _getMetalStageRequirements(anyHitInvoke);
            auto intersectionRequirements = _getMetalStageRequirements(intersectionInvoke);
            needsRecordData |=
                closestHitRequirements.record || closestHitRequirements.callableDispatch;
            needsRecordData |= anyHitRequirements.record;
            needsRecordData |= intersectionRequirements.record;
            if (anyHitInvoke || intersectionInvoke)
            {
                needsCandidateRecordSelection = true;
                needsRecordData = true;
            }
            needsMinDistance |=
                closestHitRequirements.minDistance || closestHitRequirements.objectSpaceRay ||
                anyHitRequirements.minDistance || anyHitRequirements.objectSpaceRay ||
                intersectionRequirements.minDistance || intersectionRequirements.objectSpaceRay;
            needsRayTime |= closestHitRequirements.rayTime || anyHitRequirements.rayTime ||
                            intersectionRequirements.rayTime;
            needsRayFlags |= closestHitRequirements.rayFlags || anyHitRequirements.rayFlags ||
                             intersectionRequirements.rayFlags;
            needsDispatchRaysIndex |= closestHitRequirements.dispatchRaysIndex ||
                                      anyHitRequirements.dispatchRaysIndex ||
                                      intersectionRequirements.dispatchRaysIndex;
            needsDispatchRaysDimensions |= closestHitRequirements.dispatchRaysDimensions ||
                                           anyHitRequirements.dispatchRaysDimensions ||
                                           intersectionRequirements.dispatchRaysDimensions;
            continue;
        }

        auto requirements = _getMetalStageRequirements(closestHitInvoke);
        auto anyHitRequirements = _getMetalStageRequirements(anyHitInvoke);
        auto intersectionRequirements = _getMetalStageRequirements(intersectionInvoke);
        needsRecordData |= requirements.record || requirements.callableDispatch;
        needsRecordData |= anyHitRequirements.record;
        needsRecordData |= intersectionRequirements.record;
        if (anyHitInvoke || intersectionInvoke)
        {
            needsCandidateRecordSelection = true;
            needsRecordData = true;
        }
        needsMinDistance |= requirements.minDistance || requirements.objectSpaceRay ||
                            anyHitRequirements.minDistance || anyHitRequirements.objectSpaceRay ||
                            intersectionRequirements.minDistance ||
                            intersectionRequirements.objectSpaceRay;
        needsRayTime |=
            requirements.rayTime || anyHitRequirements.rayTime || intersectionRequirements.rayTime;
        needsRayFlags |= requirements.rayFlags || anyHitRequirements.rayFlags ||
                         intersectionRequirements.rayFlags;
        needsDispatchRaysIndex |= requirements.dispatchRaysIndex ||
                                  anyHitRequirements.dispatchRaysIndex ||
                                  intersectionRequirements.dispatchRaysIndex;
        needsDispatchRaysDimensions |= requirements.dispatchRaysDimensions ||
                                       anyHitRequirements.dispatchRaysDimensions ||
                                       intersectionRequirements.dispatchRaysDimensions;
        if (requirements.hitKind)
            needsCustomHitKind = true;
        if (!requirements.hitAttributes)
            continue;

        auto key = builder.createStructKey();
        StringBuilder fieldName;
        if (auto groupName = group->getGroupType()->findDecoration<IRNameHintDecoration>())
            fieldName << groupName->getName();
        else
            fieldName << "hitGroup";
        fieldName << ".attributes";
        builder.addNameHintDecoration(key, fieldName.getUnownedSlice());
        builder.createStructField(result->type, key, cast<IRType>(group->getHitAttributesType()));
        result->customAttributeKeys.add(group->getGroupType(), key);
    }

    if (needsRecordData)
    {
        result->recordDataKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->recordDataKey,
            UnownedTerminatedStringSlice("descriptorData"));
        builder.createStructField(
            result->type,
            result->recordDataKey,
            builder.getPtrType(builder.getUIntType(), AddressSpace::Global));
    }
    if (needsCandidateRecordSelection)
    {
        result->sbtOffsetKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->sbtOffsetKey,
            UnownedTerminatedStringSlice("sbtOffset"));
        builder.createStructField(result->type, result->sbtOffsetKey, builder.getUIntType());
        result->sbtStrideKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->sbtStrideKey,
            UnownedTerminatedStringSlice("sbtStride"));
        builder.createStructField(result->type, result->sbtStrideKey, builder.getUIntType());
    }

    if (needsMinDistance)
    {
        result->minDistanceKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->minDistanceKey,
            UnownedTerminatedStringSlice("minDistance"));
        builder.createStructField(result->type, result->minDistanceKey, builder.getFloatType());
    }

    if (needsRayFlags)
    {
        result->rayFlagsKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->rayFlagsKey,
            UnownedTerminatedStringSlice("rayFlags"));
        builder.createStructField(result->type, result->rayFlagsKey, builder.getUIntType());
    }

    if (needsRayTime)
    {
        result->rayTimeKey = builder.createStructKey();
        builder.addNameHintDecoration(result->rayTimeKey, UnownedTerminatedStringSlice("rayTime"));
        builder.createStructField(result->type, result->rayTimeKey, builder.getFloatType());
    }

    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    if (needsDispatchRaysIndex)
    {
        result->dispatchRaysIndexKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->dispatchRaysIndexKey,
            UnownedTerminatedStringSlice("dispatchRaysIndex"));
        builder.createStructField(result->type, result->dispatchRaysIndexKey, uint3Type);
    }

    if (needsDispatchRaysDimensions)
    {
        result->dispatchRaysDimensionsKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->dispatchRaysDimensionsKey,
            UnownedTerminatedStringSlice("dispatchRaysDimensions"));
        builder.createStructField(result->type, result->dispatchRaysDimensionsKey, uint3Type);
    }

    if (needsCustomHitKind)
    {
        result->customHitKindKey = builder.createStructKey();
        builder.addNameHintDecoration(
            result->customHitKindKey,
            UnownedTerminatedStringSlice("customHitKind"));
        builder.createStructField(result->type, result->customHitKindKey, builder.getUIntType());
    }
    return result;
}

static bool _getMetalAccelerationStructureTopology(
    IRInst* schemaOperation,
    TargetRequest* targetRequest,
    DiagnosticSink* sink,
    UInt& outTagMask,
    IRIntegerValue& outMaxLevels)
{
    outTagMask = UInt(MetalStructuralRayTracingTag::Instancing);
    outMaxLevels = 0;

    auto accelerationStructureType = as<IRRaytracingAccelerationStructureType>(
        _getStructuralRayTracingAccelerationStructureType(schemaOperation));
    if (!accelerationStructureType || accelerationStructureType->getOperandCount() == 0)
        return true;

    auto levelCount = as<IRIntLit>(accelerationStructureType->getOperand(0));
    // A physical Metal acceleration-structure type has two operands: its logical topology and its
    // motion/instancing tags. Portable AccelerationStructure uses topology zero. This form can be
    // observed by a second trace that shares a value already specialized for an earlier trace.
    if (accelerationStructureType->getOperandCount() == 2 && levelCount &&
        levelCount->getValue() == 0)
    {
        return true;
    }
    if (!levelCount || levelCount->getValue() < 1 || levelCount->getValue() > 32)
    {
        sink->diagnose(Diagnostics::InvalidStructuralRayTracingMaxLevelCount{
            .levelCount = levelCount ? Int64(levelCount->getValue()) : Int64(-1),
            .location = schemaOperation->sourceLoc});
        return false;
    }

    if (levelCount->getValue() == 1)
        outTagMask = 0;
    else
    {
        if (!_supportsMetalLib31(targetRequest))
        {
            sink->diagnose(Diagnostics::StructuralRayTracingMultilevelRequiresMetallib31{
                .location = schemaOperation->sourceLoc});
            return false;
        }
        outMaxLevels = levelCount->getValue();
    }

    return true;
}

static bool _validateMetalCurveSupport(
    IRInst* schemaOperation,
    TargetRequest* targetRequest,
    DiagnosticSink* sink)
{
    if (_supportsMetalLib31(targetRequest))
        return true;
    for (auto decoration : schemaOperation->getDecorations())
    {
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (group &&
            StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue()) ==
                StructuralRayTracingHitAttributesKind::Curve)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingCurveRequiresMetallib31{
                .location = schemaOperation->sourceLoc});
            return false;
        }
    }
    return true;
}

static bool _addMetalMotionTags(IRInst* schemaOperation, DiagnosticSink* sink, UInt& ioTagMask)
{
    auto motionKind = StructuralRayTracingMotionKind(
        _getStructuralRayTracingMotionKind(schemaOperation)->getValue());
    if (motionKind == StructuralRayTracingMotionKind::Invalid ||
        UInt(motionKind) > UInt(StructuralRayTracingMotionKind::Primitive) +
                               UInt(StructuralRayTracingMotionKind::Instance))
    {
        sink->diagnose(
            Diagnostics::InvalidStructuralRayTracingMotion{.location = schemaOperation->sourceLoc});
        return false;
    }

    if ((UInt(motionKind) & UInt(StructuralRayTracingMotionKind::Primitive)) != 0)
        ioTagMask |= UInt(MetalStructuralRayTracingTag::PrimitiveMotion);
    if ((UInt(motionKind) & UInt(StructuralRayTracingMotionKind::Instance)) != 0)
    {
        if ((ioTagMask & UInt(MetalStructuralRayTracingTag::Instancing)) == 0)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingInstanceMotionRequiresInstancing{
                .location = schemaOperation->sourceLoc});
            return false;
        }
        ioTagMask |= UInt(MetalStructuralRayTracingTag::InstanceMotion);
    }
    return true;
}

struct MetalTraceContextRequirements
{
    UInt tagMask = 0;
    IRIntegerValue maxLevels = 0;
};

// Validates target-dependent trace-context requirements once and records their canonical Metal
// representation. Descriptor partition synthesis and operation lowering both consume this record,
// so neither path can silently derive a different topology or motion signature.
static bool _tryGetMetalTraceContextRequirements(
    IRInst* schemaOperation,
    TargetRequest* targetRequest,
    DiagnosticSink* sink,
    MetalTraceContextRequirements& outRequirements)
{
    if (!_getMetalAccelerationStructureTopology(
            schemaOperation,
            targetRequest,
            sink,
            outRequirements.tagMask,
            outRequirements.maxLevels) ||
        !_validateMetalCurveSupport(schemaOperation, targetRequest, sink) ||
        !_addMetalMotionTags(schemaOperation, sink, outRequirements.tagMask))
    {
        return false;
    }
    return true;
}

static IRType* _getMetalAccelerationStructureType(
    IRBuilder& builder,
    IRRaytracingAccelerationStructureType* sourceType,
    UInt tagMask)
{
    IRInst* topology = sourceType->getOperandCount() == 0
                           ? builder.getIntValue(builder.getIntType(), 0)
                           : sourceType->getOperand(0);
    IRInst* operands[] = {
        topology,
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(
                tagMask & (UInt(MetalStructuralRayTracingTag::Instancing) |
                           UInt(MetalStructuralRayTracingTag::PrimitiveMotion) |
                           UInt(MetalStructuralRayTracingTag::InstanceMotion)))),
    };
    return builder.getType(
        kIROp_RaytracingAccelerationStructureType,
        SLANG_COUNT_OF(operands),
        operands);
}

static bool _setMetalAccelerationStructureType(
    IRBuilder& builder,
    IRInst* value,
    UInt tagMask,
    Dictionary<IRInst*, IRType*>& assignedTypes,
    DiagnosticSink* sink)
{
    auto sourceType = as<IRRaytracingAccelerationStructureType>(value->getDataType());
    if (!sourceType)
        return false;

    auto physicalType = _getMetalAccelerationStructureType(builder, sourceType, tagMask);
    if (auto assignedType = assignedTypes.tryGetValue(value))
    {
        if (*assignedType != physicalType)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingAccelerationStructureMotionConflict{
                .location = value->sourceLoc});
            return false;
        }
        return true;
    }
    assignedTypes.add(value, physicalType);
    value->setFullType(physicalType);

    if (auto param = as<IRParam>(value))
    {
        auto block = as<IRBlock>(param->getParent());
        auto func = block ? as<IRFunc>(block->getParent()) : nullptr;
        if (func && block == func->getFirstBlock())
        {
            auto paramIndex = block->getParamIndex(param);
            fixUpFuncType(func);
            for (auto use = func->firstUse; use; use = use->nextUse)
            {
                auto call = as<IRCall>(use->getUser());
                if (!call || call->getOperand(0) != func || paramIndex < 0 ||
                    UInt(paramIndex) >= call->getArgCount())
                {
                    continue;
                }
                if (!_setMetalAccelerationStructureType(
                        builder,
                        call->getArg(UInt(paramIndex)),
                        tagMask,
                        assignedTypes,
                        sink))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

static UInt _getSharedMetalTagMask(
    IRInst* schemaOperation,
    IRType* payloadType,
    UInt topologyTagMask,
    UInt capabilityTagMask)
{
    UInt result = topologyTagMask | capabilityTagMask;
    for (auto decoration : schemaOperation->getDecorations())
    {
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (!group || !_isStructuralHitGroupForPayload(group, payloadType))
            continue;

        auto closestHit = _getMetalStageRequirements(getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::ClosestHit));
        auto anyHit = _getMetalStageRequirements(getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::AnyHit));
        auto intersection = _getMetalStageRequirements(getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::Intersection));
        auto all = _combineMetalStageRequirements(
            _combineMetalStageRequirements(closestHit, anyHit),
            intersection);
        auto hitAttributesKind =
            StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue());
        if (all.triangleBarycentricCoord || all.triangleFrontFacing ||
            (all.hitKind && hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle))
            result |= UInt(MetalStructuralRayTracingTag::TriangleData);
        if (all.curveParameter)
            result |= UInt(MetalStructuralRayTracingTag::CurveData);
        if (anyHit.worldSpaceOrigin || anyHit.worldSpaceDirection ||
            intersection.worldSpaceOrigin || intersection.worldSpaceDirection ||
            closestHit.objectSpaceRay || all.objectToWorld || all.worldToObject)
        {
            result |= UInt(MetalStructuralRayTracingTag::WorldSpaceData);
        }
    }
    return result;
}

enum class MetalDescriptorDataSection : UInt
{
    InstanceHitGroupOffsets = 0,
    HitRecords = 1,
    MissRecords = 2,
    CallableRecords = 3,
};

static IRInst* _emitMetalRecordAddress(
    IRBuilder& builder,
    IRInst* descriptorData,
    MetalDescriptorDataSection section,
    IRInst* recordIndex,
    IRIntegerValue recordStride)
{
    auto uintType = builder.getUIntType();
    auto sectionOffset = builder.emitLoad(builder.emitGetOffsetPtr(
        descriptorData,
        builder.getIntValue(uintType, IRIntegerValue(UInt(section)))));
    auto recordByteOffset = builder.emitAdd(
        uintType,
        sectionOffset,
        builder.emitMul(uintType, recordIndex, builder.getIntValue(uintType, recordStride)));
    auto bytePointerType = builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global);
    auto recordByteBase = builder.emitBitCast(bytePointerType, descriptorData);
    return builder.emitGetOffsetPtr(recordByteBase, recordByteOffset);
}

static IRInst* _emitMetalRecordValueFromDataAddress(
    IRBuilder& builder,
    IRInst* recordDataAddress,
    IRType* recordType)
{
    auto recordPointerType = builder.getPtrType(recordType, AddressSpace::Global);
    return builder.emitLoad(builder.emitBitCast(recordPointerType, recordDataAddress));
}

static IRParam* _emitMetalSystemValueParam(
    IRBuilder& builder,
    IRType* type,
    const char* name,
    const char* systemValue)
{
    auto result = builder.emitParam(type);
    builder.addNameHintDecoration(result, UnownedTerminatedStringSlice(name));
    builder.addTargetSystemValueDecoration(result, UnownedStringSlice(systemValue));
    return result;
}

static IRMatrixType* _getFloat4x3Type(IRBuilder& builder)
{
    return builder.getMatrixType(
        builder.getFloatType(),
        builder.getIntValue(builder.getIntType(), 4),
        builder.getIntValue(builder.getIntType(), 3),
        builder.getIntValue(builder.getIntType(), kMatrixLayoutMode_RowMajor));
}

static void _addStructuralStageInfo(
    IRBuilder& builder,
    IRFunc* adapter,
    StructuralRayTracingStageKind stageKind,
    IRFunc* invoke,
    IRType* stageType,
    IRStringLit* stageSourceTypeName,
    IRStringLit* stageTypeIdentity,
    IRType* contextType,
    IRType* payloadType,
    IRType* recordType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind,
    IRType* callableDataType = nullptr)
{
    addStructuralRayTracingEntryPointInfo(
        builder,
        adapter,
        {
            .stageKind = stageKind,
            .invoke = invoke,
            .stageType = stageType,
            .stageSourceTypeName = stageSourceTypeName,
            .stageTypeIdentity = stageTypeIdentity,
            .contextType = contextType,
            .payloadType = payloadType,
            .recordType = recordType,
            .hitAttributesType = hitAttributesType,
            .callableDataType = callableDataType,
            .hitAttributesKind = hitAttributesKind,
        });
}

static void _collectStageInputOperations(IRInst* parent, List<IRInst*>& operations);
static void _inlineCandidateOperationCalls(IRFunc* adapter);

struct MetalVisibleInputValues
{
    IRInst* record = nullptr;
    IRInst* hitAttributes = nullptr;
    IRInst* triangleBarycentricCoord = nullptr;
    IRInst* triangleFrontFacing = nullptr;
    IRInst* curveParameter = nullptr;
    IRInst* minDistance = nullptr;
    IRInst* distance = nullptr;
    IRInst* rayTime = nullptr;
    IRInst* rayFlags = nullptr;
    IRInst* hitKind = nullptr;
    IRInst* worldSpaceOrigin = nullptr;
    IRInst* worldSpaceDirection = nullptr;
    IRInst* primitiveIndex = nullptr;
    IRInst* geometryIndex = nullptr;
    IRInst* instanceIndex = nullptr;
    IRInst* instanceID = nullptr;
    IRInst* objectSpaceOrigin = nullptr;
    IRInst* objectSpaceDirection = nullptr;
    IRInst* objectToWorld = nullptr;
    IRInst* worldToObject = nullptr;
    IRInst* dispatchRaysIndex = nullptr;
    IRInst* dispatchRaysDimensions = nullptr;
};

struct MetalDispatchValues
{
    IRInst* index = nullptr;
    IRInst* dimensions = nullptr;
};

static void _lowerMetalVisibleInputOperations(
    IRFunc* adapter,
    const MetalVisibleInputValues& values)
{
    List<IRInst*> operations;
    _collectStageInputOperations(adapter, operations);
    for (auto operation : operations)
    {
        IRInst* replacement = nullptr;
        switch (operation->getOp())
        {
        case kIROp_StructuralRayTracingGetRecord:
            replacement = values.record;
            break;
        case kIROp_StructuralRayTracingGetHitAttributes:
            replacement = values.hitAttributes;
            break;
        case kIROp_StructuralRayTracingGetTriangleBarycentricCoord:
            replacement = values.triangleBarycentricCoord;
            break;
        case kIROp_StructuralRayTracingGetTriangleFrontFacing:
            replacement = values.triangleFrontFacing;
            break;
        case kIROp_StructuralRayTracingGetCurveParameter:
            replacement = values.curveParameter;
            break;
        case kIROp_StructuralRayTracingGetRayTMin:
            replacement = values.minDistance;
            break;
        case kIROp_StructuralRayTracingGetRayTCurrent:
            replacement = values.distance;
            break;
        case kIROp_StructuralRayTracingGetRayTime:
            replacement = values.rayTime;
            break;
        case kIROp_StructuralRayTracingGetRayFlags:
            replacement = values.rayFlags;
            break;
        case kIROp_StructuralRayTracingGetHitKind:
            replacement = values.hitKind;
            break;
        case kIROp_StructuralRayTracingGetWorldRayOrigin:
            replacement = values.worldSpaceOrigin;
            break;
        case kIROp_StructuralRayTracingGetWorldRayDirection:
            replacement = values.worldSpaceDirection;
            break;
        case kIROp_StructuralRayTracingGetPrimitiveIndex:
            replacement = values.primitiveIndex;
            break;
        case kIROp_StructuralRayTracingGetGeometryIndex:
            replacement = values.geometryIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceIndex:
            replacement = values.instanceIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceID:
            replacement = values.instanceID;
            break;
        case kIROp_StructuralRayTracingGetObjectSpaceRay:
            {
                IRBuilder builder(operation);
                builder.setInsertBefore(operation);
                IRInst* fields[] = {
                    values.objectSpaceOrigin,
                    values.minDistance,
                    values.objectSpaceDirection,
                    values.distance,
                };
                replacement = builder.emitMakeStruct(
                    cast<IRType>(operation->getDataType()),
                    SLANG_COUNT_OF(fields),
                    fields);
                break;
            }
        case kIROp_StructuralRayTracingGetObjectToWorld:
            replacement = values.objectToWorld;
            break;
        case kIROp_StructuralRayTracingGetWorldToObject:
            replacement = values.worldToObject;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysIndex:
            replacement = values.dispatchRaysIndex;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysDimensions:
            replacement = values.dispatchRaysDimensions;
            break;
        default:
            break;
        }
        if (replacement)
        {
            operation->replaceUsesWith(replacement);
            operation->removeAndDeallocate();
        }
    }
}

static void _collectMetalCallableDispatchOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectMetalCallableDispatchOperations(child, operations);
        if (child->getOp() == kIROp_MetalStructuralRayTracingCallShader)
            operations.add(child);
    }
}

static void _rebindMetalCallableDispatches(
    IRFunc* function,
    IRInst* descriptorResources,
    IRInst* records)
{
    List<IRInst*> operations;
    _collectMetalCallableDispatchOperations(function, operations);
    for (auto operation : operations)
    {
        operation->setOperand(6, descriptorResources);
        operation->setOperand(7, records);
    }
}

static IRFunc* _generateVisibleStageAdapter(
    IRModule* module,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generated,
    Dictionary<IRFunc*, IRInst*>& payloadValues,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues,
    IRInst* entryType,
    StructuralRayTracingStageKind stageKind,
    IRType* stageType,
    IRStringLit* stageSourceTypeName,
    IRStringLit* stageTypeIdentity,
    IRInst* invokeValue,
    IRType* contextType,
    IRType* payloadType,
    IRType* recordType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind,
    UnownedStringSlice physicalName,
    const MetalStageRequirements& tableRequirements,
    MetalRayDataInfo* rayDataInfo,
    IRType* descriptorResourcesPointerType,
    IRMetalVisibleFunctionTable* visibleFunctionTableType)
{
    auto invoke = as<IRFunc>(invokeValue);
    if (!invoke && stageKind != StructuralRayTracingStageKind::ClosestHit)
        return nullptr;
    KeyValuePair<IRInst*, IRInst*> generatedKey(entryType, rayDataInfo->type);
    if (auto existing = generated.tryGetValue(generatedKey))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    auto rayDataPointerType = builder.getPtrType(rayDataInfo->type, AddressSpace::ThreadLocal);
    adapter->setFullType(visibleFunctionTableType->getFunctionType());

    // These functions are looked up by the host and inserted into a schema-specific VFT. Export
    // the shared reflection name exactly; a name hint alone would let Metal emission append a
    // collision-order suffix and break the reflected ABI.
    builder.addNameHintDecoration(adapter, physicalName);
    builder.addExportDecoration(adapter, physicalName);
    builder.addKeepAliveDecoration(adapter);
    IRInst* visibleDecorationOperands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(stageKind)),
        visibleFunctionTableType,
    };
    builder.addDecoration(
        adapter,
        kIROp_MetalVisibleFunctionDecoration,
        visibleDecorationOperands,
        SLANG_COUNT_OF(visibleDecorationOperands));
    if (invoke)
    {
        _addStructuralStageInfo(
            builder,
            adapter,
            stageKind,
            invoke,
            stageType,
            stageSourceTypeName,
            stageTypeIdentity,
            contextType,
            payloadType,
            recordType,
            hitAttributesType,
            hitAttributesKind);
    }

    builder.setInsertInto(adapter);
    builder.emitBlock();
    auto rayData = builder.emitParam(rayDataPointerType);
    builder.addNameHintDecoration(rayData, UnownedTerminatedStringSlice("rayData"));

    auto emitNamedParam = [&](IRType* type, const char* name)
    {
        auto param = builder.emitParam(type);
        builder.addNameHintDecoration(param, UnownedTerminatedStringSlice(name));
        return param;
    };

    MetalVisibleInputValues values;
    if (tableRequirements.distance || tableRequirements.objectSpaceRay)
        values.distance = emitNamedParam(builder.getFloatType(), "distance");
    if (tableRequirements.hitKind)
        values.hitKind = emitNamedParam(builder.getUIntType(), "hitKind");
    if (tableRequirements.triangleBarycentricCoord)
        values.triangleBarycentricCoord =
            emitNamedParam(builder.getVectorType(builder.getFloatType(), 2), "barycentricCoord");
    if (tableRequirements.triangleFrontFacing)
        values.triangleFrontFacing = emitNamedParam(builder.getBoolType(), "frontFacing");
    if (tableRequirements.curveParameter)
        values.curveParameter = emitNamedParam(builder.getFloatType(), "curveParameter");
    if (tableRequirements.worldSpaceOrigin)
        values.worldSpaceOrigin =
            emitNamedParam(builder.getVectorType(builder.getFloatType(), 3), "worldSpaceOrigin");
    if (tableRequirements.worldSpaceDirection)
        values.worldSpaceDirection =
            emitNamedParam(builder.getVectorType(builder.getFloatType(), 3), "worldSpaceDirection");
    if (tableRequirements.primitiveIndex)
        values.primitiveIndex = emitNamedParam(builder.getUIntType(), "primitiveIndex");
    if (tableRequirements.geometryIndex)
        values.geometryIndex = emitNamedParam(builder.getUIntType(), "geometryIndex");
    if (tableRequirements.instanceIndex)
        values.instanceIndex = emitNamedParam(builder.getUIntType(), "instanceIndex");
    if (tableRequirements.instanceID)
        values.instanceID = emitNamedParam(builder.getUIntType(), "instanceID");
    if (tableRequirements.objectSpaceRay)
    {
        values.objectSpaceOrigin =
            emitNamedParam(builder.getVectorType(builder.getFloatType(), 3), "objectSpaceOrigin");
        values.objectSpaceDirection = emitNamedParam(
            builder.getVectorType(builder.getFloatType(), 3),
            "objectSpaceDirection");
    }
    if (tableRequirements.objectToWorld)
        values.objectToWorld = emitNamedParam(_getFloat4x3Type(builder), "objectToWorld");
    if (tableRequirements.worldToObject)
        values.worldToObject = emitNamedParam(_getFloat4x3Type(builder), "worldToObject");
    IRInst* recordDataAddress = nullptr;
    if (tableRequirements.record)
    {
        recordDataAddress = emitNamedParam(
            builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global),
            "recordData");
    }
    IRInst* descriptorResources = nullptr;
    if (tableRequirements.callableDispatch)
        descriptorResources = emitNamedParam(descriptorResourcesPointerType, "descriptorResources");

    // A dense Metal VFT cannot leave a `NoClosestHit` slot uninitialized when another group in the
    // payload partition enables post-traversal closest-hit dispatch. Its no-op accepts the exact
    // table-wide signature but intentionally has no source-stage metadata or input operations.
    if (!invoke)
    {
        builder.emitReturn();
        generated.add(generatedKey, adapter);
        return adapter;
    }

    auto payload = builder.emitFieldAddress(rayData, rayDataInfo->payloadKey);
    payloadValues[adapter] = payload;
    if (rayDataInfo->minDistanceKey)
    {
        values.minDistance =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->minDistanceKey));
    }
    if (rayDataInfo->rayFlagsKey)
    {
        values.rayFlags =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->rayFlagsKey));
    }
    if (rayDataInfo->rayTimeKey)
    {
        values.rayTime =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->rayTimeKey));
    }
    if (rayDataInfo->dispatchRaysIndexKey)
    {
        values.dispatchRaysIndex =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysIndexKey));
    }
    if (rayDataInfo->dispatchRaysDimensionsKey)
    {
        values.dispatchRaysDimensions = builder.emitLoad(
            builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysDimensionsKey));
    }
    if (values.dispatchRaysIndex || values.dispatchRaysDimensions)
    {
        dispatchValues[adapter] = {values.dispatchRaysIndex, values.dispatchRaysDimensions};
    }

    if (tableRequirements.record)
        values.record =
            _emitMetalRecordValueFromDataAddress(builder, recordDataAddress, recordType);

    if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Custom)
    {
        if (auto key = rayDataInfo->customAttributeKeys.tryGetValue(entryType))
            values.hitAttributes = builder.emitLoad(builder.emitFieldAddress(rayData, *key));
        if (rayDataInfo->customHitKindKey)
        {
            values.hitKind =
                builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->customHitKindKey));
        }
    }

    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn();

    _inlineCandidateOperationCalls(adapter);
    _lowerMetalVisibleInputOperations(adapter, values);
    if (descriptorResources)
    {
        SLANG_ASSERT(rayDataInfo->recordDataKey);
        auto records =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->recordDataKey));
        _rebindMetalCallableDispatches(adapter, descriptorResources, records);
    }
    generated.add(generatedKey, adapter);
    return adapter;
}

static IRFunc* _generateCallableStageAdapter(
    IRModule* module,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generated,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues,
    IRStructuralRayTracingCallableShaderInfoDecoration* entry,
    IRInst* programLayout,
    UnownedStringSlice physicalName,
    IRType* descriptorResourcesPointerType,
    IRMetalVisibleFunctionTable* callableFunctionTableType,
    const MetalStageRequirements& signatureRequirements)
{
    auto invoke = as<IRFunc>(entry->getCallable());
    if (!invoke)
        return nullptr;
    // The callable ABI is schema-wide. The same source callable can therefore need two physical
    // adapters when it appears in two schemas, even if canonical type construction happens to
    // deduplicate their visible-function-table types.
    KeyValuePair<IRInst*, IRInst*> generatedKey(entry->getCallableType(), programLayout);
    if (auto existing = generated.tryGetValue(generatedKey))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    auto dataType = cast<IRType>(entry->getCallableDataType());
    auto dataPointerType = builder.getPtrType(dataType, AddressSpace::ThreadLocal);
    auto descriptorDataType = builder.getPtrType(builder.getUIntType(), AddressSpace::Global);
    auto requirements = _getMetalStageRequirements(invoke);
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    adapter->setFullType(callableFunctionTableType->getFunctionType());

    builder.addNameHintDecoration(adapter, physicalName);
    builder.addExportDecoration(adapter, physicalName);
    builder.addKeepAliveDecoration(adapter);
    IRInst* visibleDecorationOperands[] = {
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(StructuralRayTracingStageKind::Callable)),
        callableFunctionTableType,
    };
    builder.addDecoration(
        adapter,
        kIROp_MetalVisibleFunctionDecoration,
        visibleDecorationOperands,
        SLANG_COUNT_OF(visibleDecorationOperands));
    _addStructuralStageInfo(
        builder,
        adapter,
        StructuralRayTracingStageKind::Callable,
        invoke,
        entry->getCallableType(),
        entry->getCallableSourceTypeName(),
        entry->getCallableTypeIdentity(),
        entry->getContextType(),
        nullptr,
        entry->getRecordType(),
        nullptr,
        StructuralRayTracingHitAttributesKind::None,
        dataType);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    auto data = builder.emitParam(dataPointerType);
    builder.addNameHintDecoration(data, UnownedTerminatedStringSlice("data"));
    IRInst* dispatchRaysIndex = nullptr;
    IRInst* dispatchRaysDimensions = nullptr;
    if (signatureRequirements.dispatchRaysIndex)
    {
        dispatchRaysIndex = builder.emitParam(uint3Type);
        builder.addNameHintDecoration(
            dispatchRaysIndex,
            UnownedTerminatedStringSlice("dispatchRaysIndex"));
    }
    if (signatureRequirements.dispatchRaysDimensions)
    {
        dispatchRaysDimensions = builder.emitParam(uint3Type);
        builder.addNameHintDecoration(
            dispatchRaysDimensions,
            UnownedTerminatedStringSlice("dispatchRaysDimensions"));
    }
    IRInst* recordDataAddress = nullptr;
    if (signatureRequirements.record)
    {
        recordDataAddress =
            builder.emitParam(builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global));
        builder.addNameHintDecoration(
            recordDataAddress,
            UnownedTerminatedStringSlice("recordData"));
    }
    auto descriptorResources = builder.emitParam(descriptorResourcesPointerType);
    builder.addNameHintDecoration(
        descriptorResources,
        UnownedTerminatedStringSlice("descriptorResources"));
    auto descriptorData = builder.emitParam(descriptorDataType);
    builder.addNameHintDecoration(descriptorData, UnownedTerminatedStringSlice("descriptorData"));

    IRInst* record = nullptr;
    if (requirements.record)
        record = _emitMetalRecordValueFromDataAddress(
            builder,
            recordDataAddress,
            cast<IRType>(entry->getRecordType()));

    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn();

    _inlineCandidateOperationCalls(adapter);
    _rebindMetalCallableDispatches(adapter, descriptorResources, descriptorData);
    List<IRInst*> operations;
    _collectStageInputOperations(adapter, operations);
    for (auto operation : operations)
    {
        IRInst* replacement = nullptr;
        if (operation->getOp() == kIROp_StructuralRayTracingGetCallableData)
            replacement = data;
        else if (operation->getOp() == kIROp_StructuralRayTracingGetRecord)
            replacement = record;
        else if (operation->getOp() == kIROp_StructuralRayTracingGetDispatchRaysIndex)
            replacement = dispatchRaysIndex;
        else if (operation->getOp() == kIROp_StructuralRayTracingGetDispatchRaysDimensions)
            replacement = dispatchRaysDimensions;
        if (!replacement)
            continue;
        operation->replaceUsesWith(replacement);
        operation->removeAndDeallocate();
    }

    if (dispatchRaysIndex || dispatchRaysDimensions)
        dispatchValues[adapter] = {dispatchRaysIndex, dispatchRaysDimensions};

    generated.add(generatedKey, adapter);
    return adapter;
}

struct MetalCandidateResultInfo
{
    IRStructType* type = nullptr;
    IRStructKey* acceptKey = nullptr;
    IRStructKey* continueSearchKey = nullptr;
    IRStructKey* distanceKey = nullptr;
};

static MetalCandidateResultInfo _createMetalCandidateResultType(
    IRModule* module,
    const char* name,
    bool includeDistance)
{
    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    MetalCandidateResultInfo result;
    result.type = builder.createStructType();
    builder.addNameHintDecoration(result.type, UnownedTerminatedStringSlice(name));

    result.acceptKey = builder.createStructKey();
    builder.addNameHintDecoration(result.acceptKey, UnownedTerminatedStringSlice("accept"));
    builder.addTargetSystemValueDecoration(result.acceptKey, toSlice("accept_intersection"));
    builder.createStructField(result.type, result.acceptKey, builder.getBoolType());

    result.continueSearchKey = builder.createStructKey();
    builder.addNameHintDecoration(
        result.continueSearchKey,
        UnownedTerminatedStringSlice("continueSearch"));
    builder.addTargetSystemValueDecoration(result.continueSearchKey, toSlice("continue_search"));
    builder.createStructField(result.type, result.continueSearchKey, builder.getBoolType());

    if (includeDistance)
    {
        result.distanceKey = builder.createStructKey();
        builder.addNameHintDecoration(result.distanceKey, UnownedTerminatedStringSlice("distance"));
        builder.addTargetSystemValueDecoration(result.distanceKey, toSlice("distance"));
        builder.createStructField(result.type, result.distanceKey, builder.getFloatType());
    }
    return result;
}

static IRInst* _emitMetalCandidateResult(
    IRBuilder& builder,
    const MetalCandidateResultInfo& resultInfo,
    IRInst* accept,
    IRInst* continueSearch,
    IRInst* distance = nullptr)
{
    List<IRInst*> values;
    values.add(accept);
    values.add(continueSearch);
    if (resultInfo.distanceKey)
    {
        SLANG_ASSERT(distance);
        values.add(distance);
    }
    return builder.emitMakeStruct(resultInfo.type, values.getCount(), values.getBuffer());
}

static IRInst* _emitMetalCandidateResult(
    IRBuilder& builder,
    const MetalCandidateResultInfo& resultInfo,
    bool accept,
    bool continueSearch,
    IRInst* distance = nullptr)
{
    return _emitMetalCandidateResult(
        builder,
        resultInfo,
        builder.getBoolValue(accept),
        builder.getBoolValue(continueSearch),
        distance);
}

static String _getMetalCandidateName(IRStringLit* groupSourceTypeName, UnownedStringSlice suffix)
{
    StringBuilder logicalName;
    logicalName << groupSourceTypeName->getStringSlice();
    logicalName << ".candidate" << suffix;
    auto name = logicalName.produceString();
    return getStructuralRayTracingEntryPointName(name.getUnownedSlice());
}

static void _collectCallsAndAnyHitTerminations(
    IRInst* parent,
    List<IRCall*>& calls,
    bool& hasCandidateOperation)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectCallsAndAnyHitTerminations(child, calls, hasCandidateOperation);
        if (auto call = as<IRCall>(child))
            calls.add(call);
        else if (
            as<IRStructuralRayTracingStageInputOperation>(child) ||
            child->getOp() == kIROp_StructuralRayTracingCallShader ||
            child->getOp() == kIROp_MetalStructuralRayTracingCallShader)
            hasCandidateOperation = true;
    }
}

static bool _functionCanReach(IRFunc* function, IRFunc* target, HashSet<IRFunc*>& activeFunctions)
{
    if (!activeFunctions.add(function))
        return false;

    List<IRCall*> calls;
    bool hasCandidateOperation = false;
    _collectCallsAndAnyHitTerminations(function, calls, hasCandidateOperation);
    for (auto call : calls)
    {
        auto callee = as<IRFunc>(call->getCallee());
        if (callee && (callee == target || _functionCanReach(callee, target, activeFunctions)))
        {
            activeFunctions.remove(function);
            return true;
        }
    }
    activeFunctions.remove(function);
    return false;
}

static void _inlineCandidateOperationCalls(IRFunc* adapter)
{
    List<IRFunc*> reachableFunctions;
    HashSet<IRFunc*> reachableFunctionSet;
    reachableFunctions.add(adapter);
    reachableFunctionSet.add(adapter);
    for (Index i = 0; i < reachableFunctions.getCount(); ++i)
    {
        List<IRCall*> calls;
        bool hasCandidateOperation = false;
        _collectCallsAndAnyHitTerminations(reachableFunctions[i], calls, hasCandidateOperation);
        for (auto call : calls)
        {
            if (auto callee = as<IRFunc>(call->getCallee()))
            {
                if (reachableFunctionSet.add(callee))
                    reachableFunctions.add(callee);
            }
        }
    }

    HashSet<IRFunc*> terminatingFunctions;
    for (auto func : reachableFunctions)
    {
        List<IRCall*> calls;
        bool hasCandidateOperation = false;
        _collectCallsAndAnyHitTerminations(func, calls, hasCandidateOperation);
        if (hasCandidateOperation)
            terminatingFunctions.add(func);
    }

    bool changed;
    do
    {
        changed = false;
        for (auto func : reachableFunctions)
        {
            if (terminatingFunctions.contains(func))
                continue;
            List<IRCall*> calls;
            bool hasCandidateOperation = false;
            _collectCallsAndAnyHitTerminations(func, calls, hasCandidateOperation);
            for (auto call : calls)
            {
                if (auto callee = as<IRFunc>(call->getCallee()))
                {
                    if (terminatingFunctions.contains(callee))
                    {
                        terminatingFunctions.add(func);
                        changed = true;
                        break;
                    }
                }
            }
        }
    } while (changed);

    HashSet<IRFunc*> recursiveFunctions;
    for (auto func : reachableFunctions)
    {
        HashSet<IRFunc*> activeFunctions;
        if (_functionCanReach(func, func, activeFunctions))
            recursiveFunctions.add(func);
    }

    for (;;)
    {
        List<IRCall*> calls;
        bool hasCandidateOperation = false;
        _collectCallsAndAnyHitTerminations(adapter, calls, hasCandidateOperation);
        IRCall* callToInline = nullptr;
        for (auto call : calls)
        {
            auto callee = as<IRFunc>(call->getCallee());
            if (callee && terminatingFunctions.contains(callee) &&
                !recursiveFunctions.contains(callee))
            {
                callToInline = call;
                break;
            }
        }
        if (!callToInline)
            break;
        const bool didInline = inlineCall(callToInline);
        SLANG_ASSERT(didInline);
    }
}

static void _lowerAnyHitTerminations(IRFunc* adapter, const MetalCandidateResultInfo& resultInfo)
{
    List<IRBlock*> blocks;
    for (auto block : adapter->getBlocks())
        blocks.add(block);

    for (auto block : blocks)
    {
        for (auto inst = block->getFirstOrdinaryInst(); inst; inst = inst->getNextInst())
        {
            bool accept;
            bool continueSearch;
            if (inst->getOp() == kIROp_StructuralRayTracingIgnoreHit)
            {
                accept = false;
                continueSearch = true;
            }
            else if (inst->getOp() == kIROp_StructuralRayTracingAcceptHitAndEndSearch)
            {
                accept = true;
                continueSearch = false;
            }
            else
                continue;

            IRBuilder builder(inst);
            builder.setInsertBefore(inst);
            builder.emitReturn(
                _emitMetalCandidateResult(builder, resultInfo, accept, continueSearch));
            for (auto oldInst = inst; oldInst;)
            {
                auto next = oldInst->getNextInst();
                oldInst->removeAndDeallocate();
                oldInst = next;
            }
            break;
        }
    }
}

struct MetalCandidateInputValues
{
    IRInst* record = nullptr;
    IRInst* triangleBarycentricCoord = nullptr;
    IRInst* triangleFrontFacing = nullptr;
    IRInst* curveParameter = nullptr;
    IRInst* minDistance = nullptr;
    IRInst* distance = nullptr;
    IRInst* rayTime = nullptr;
    IRInst* rayFlags = nullptr;
    IRInst* hitKind = nullptr;
    IRInst* worldSpaceOrigin = nullptr;
    IRInst* worldSpaceDirection = nullptr;
    IRInst* primitiveIndex = nullptr;
    IRInst* geometryIndex = nullptr;
    IRInst* instanceIndex = nullptr;
    IRInst* instanceID = nullptr;
    IRInst* objectSpaceOrigin = nullptr;
    IRInst* objectSpaceDirection = nullptr;
    IRInst* objectToWorld = nullptr;
    IRInst* worldToObject = nullptr;
    IRInst* dispatchRaysIndex = nullptr;
    IRInst* dispatchRaysDimensions = nullptr;
};

static void _collectStageInputOperations(IRInst* parent, List<IRInst*>& operations);

static void _lowerMetalCandidateInputOperations(
    IRFunc* function,
    const MetalCandidateInputValues& values)
{
    List<IRInst*> operations;
    _collectStageInputOperations(function, operations);
    for (auto operation : operations)
    {
        IRInst* replacement = nullptr;
        switch (operation->getOp())
        {
        case kIROp_StructuralRayTracingGetRecord:
            replacement = values.record;
            break;
        case kIROp_StructuralRayTracingGetTriangleBarycentricCoord:
            replacement = values.triangleBarycentricCoord;
            break;
        case kIROp_StructuralRayTracingGetTriangleFrontFacing:
            replacement = values.triangleFrontFacing;
            break;
        case kIROp_StructuralRayTracingGetCurveParameter:
            replacement = values.curveParameter;
            break;
        case kIROp_StructuralRayTracingGetRayTMin:
            replacement = values.minDistance;
            break;
        case kIROp_StructuralRayTracingGetRayTCurrent:
            replacement = values.distance;
            break;
        case kIROp_StructuralRayTracingGetRayTime:
            replacement = values.rayTime;
            break;
        case kIROp_StructuralRayTracingGetRayFlags:
            replacement = values.rayFlags;
            break;
        case kIROp_StructuralRayTracingGetHitKind:
            replacement = values.hitKind;
            break;
        case kIROp_StructuralRayTracingGetWorldRayOrigin:
            replacement = values.worldSpaceOrigin;
            break;
        case kIROp_StructuralRayTracingGetWorldRayDirection:
            replacement = values.worldSpaceDirection;
            break;
        case kIROp_StructuralRayTracingGetPrimitiveIndex:
            replacement = values.primitiveIndex;
            break;
        case kIROp_StructuralRayTracingGetGeometryIndex:
            replacement = values.geometryIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceIndex:
            replacement = values.instanceIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceID:
            replacement = values.instanceID;
            break;
        case kIROp_StructuralRayTracingGetObjectSpaceRay:
            {
                IRBuilder builder(operation);
                builder.setInsertBefore(operation);
                IRInst* fields[] = {
                    values.objectSpaceOrigin,
                    values.minDistance,
                    values.objectSpaceDirection,
                    values.distance,
                };
                replacement = builder.emitMakeStruct(
                    cast<IRType>(operation->getDataType()),
                    SLANG_COUNT_OF(fields),
                    fields);
                break;
            }
        case kIROp_StructuralRayTracingGetObjectToWorld:
            replacement = values.objectToWorld;
            break;
        case kIROp_StructuralRayTracingGetWorldToObject:
            replacement = values.worldToObject;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysIndex:
            replacement = values.dispatchRaysIndex;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysDimensions:
            replacement = values.dispatchRaysDimensions;
            break;
        default:
            break;
        }
        if (replacement)
        {
            operation->replaceUsesWith(replacement);
            operation->removeAndDeallocate();
        }
    }
}

static IRFunc* _generateBuiltInAnyHitCandidateAdapter(
    IRModule* module,
    Dictionary<KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*>, IRFunc*>& generated,
    Dictionary<IRFunc*, IRInst*>& payloadValues,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues,
    const MetalCandidateResultInfo& resultInfo,
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    const MetalStageRequirements& signatureRequirements,
    UInt tagMask,
    MetalRayDataInfo* rayDataInfo)
{
    auto invoke =
        getStructuralRayTracingHitGroupStageInvoke(group, StructuralRayTracingStageKind::AnyHit);
    if (!invoke)
        return nullptr;
    auto groupType = group->getGroupType();
    KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*> generatedKey(
        KeyValuePair<IRInst*, UInt>(groupType, tagMask),
        rayDataInfo->type);
    if (auto existing = generated.tryGetValue(generatedKey))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    auto hitAttributesKind =
        StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue());
    auto requirements = _getMetalStageRequirements(invoke);
    List<IRType*> parameterTypes;
    if (signatureRequirements.distance || signatureRequirements.objectSpaceRay)
        parameterTypes.add(builder.getFloatType());
    if (signatureRequirements.triangleBarycentricCoord)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 2));
    if (signatureRequirements.triangleFrontFacing ||
        (signatureRequirements.hitKind &&
         hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle))
    {
        parameterTypes.add(builder.getBoolType());
    }
    if (signatureRequirements.curveParameter)
        parameterTypes.add(builder.getFloatType());
    if (signatureRequirements.worldSpaceOrigin)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (signatureRequirements.worldSpaceDirection)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (signatureRequirements.primitiveIndex)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.geometryIndex)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.instanceIndex)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.instanceID)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.objectSpaceRay)
    {
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    }
    if (signatureRequirements.objectToWorld)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (signatureRequirements.worldToObject)
        parameterTypes.add(_getFloat4x3Type(builder));
    parameterTypes.add(builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global));
    auto rayDataPointerType = builder.getPtrType(rayDataInfo->type, AddressSpace::ThreadLocal);
    parameterTypes.add(rayDataPointerType);
    adapter->setFullType(builder.getFuncType(parameterTypes, resultInfo.type));

    auto name = _getMetalCandidateName(group->getGroupSourceTypeName(), toSlice(".arm"));
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addForceInlineDecoration(adapter);
    _addStructuralStageInfo(
        builder,
        adapter,
        StructuralRayTracingStageKind::AnyHit,
        invoke,
        group->getAnyHitType(),
        group->getAnyHitSourceTypeName(),
        group->getAnyHitTypeIdentity(),
        group->getContextType(),
        group->getPayloadType(),
        group->getRecordType(),
        group->getHitAttributesType(),
        hitAttributesKind);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    MetalCandidateInputValues inputs;
    if (signatureRequirements.distance || signatureRequirements.objectSpaceRay)
        inputs.distance =
            _emitMetalSystemValueParam(builder, builder.getFloatType(), "distance", "distance");
    if (signatureRequirements.triangleBarycentricCoord)
    {
        inputs.triangleBarycentricCoord = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 2),
            "barycentricCoord",
            "barycentric_coord");
    }
    if (signatureRequirements.triangleFrontFacing ||
        (signatureRequirements.hitKind &&
         hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle))
    {
        inputs.triangleFrontFacing = _emitMetalSystemValueParam(
            builder,
            builder.getBoolType(),
            "frontFacing",
            "front_facing");
    }
    if (signatureRequirements.curveParameter)
    {
        inputs.curveParameter = _emitMetalSystemValueParam(
            builder,
            builder.getFloatType(),
            "curveParameter",
            "curve_parameter");
    }
    if (signatureRequirements.worldSpaceOrigin)
    {
        inputs.worldSpaceOrigin = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceOrigin",
            "world_space_origin");
    }
    if (signatureRequirements.worldSpaceDirection)
    {
        inputs.worldSpaceDirection = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceDirection",
            "world_space_direction");
    }
    if (signatureRequirements.primitiveIndex)
    {
        inputs.primitiveIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "primitiveIndex",
            "primitive_id");
    }
    if (signatureRequirements.geometryIndex)
    {
        inputs.geometryIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "geometryIndex",
            "geometry_id");
    }
    if (signatureRequirements.instanceIndex)
    {
        inputs.instanceIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "instanceIndex",
            "instance_id");
    }
    if (signatureRequirements.instanceID)
    {
        inputs.instanceID = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "instanceID",
            "user_instance_id");
    }
    if (signatureRequirements.objectSpaceRay)
    {
        inputs.objectSpaceOrigin = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "objectSpaceOrigin",
            "origin");
        inputs.objectSpaceDirection = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "objectSpaceDirection",
            "direction");
    }
    if (signatureRequirements.objectToWorld)
    {
        inputs.objectToWorld = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "objectToWorld",
            "object_to_world_transform");
    }
    if (signatureRequirements.worldToObject)
    {
        inputs.worldToObject = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "worldToObject",
            "world_to_object_transform");
    }
    auto recordDataAddress =
        builder.emitParam(builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global));
    builder.addNameHintDecoration(recordDataAddress, UnownedTerminatedStringSlice("recordData"));
    auto rayData = builder.emitParam(rayDataPointerType);
    builder.addNameHintDecoration(rayData, UnownedTerminatedStringSlice("rayData"));
    payloadValues[adapter] = builder.emitFieldAddress(rayData, rayDataInfo->payloadKey);
    if (rayDataInfo->minDistanceKey)
    {
        inputs.minDistance =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->minDistanceKey));
    }
    if (rayDataInfo->rayFlagsKey)
    {
        inputs.rayFlags =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->rayFlagsKey));
    }
    if (rayDataInfo->rayTimeKey)
    {
        inputs.rayTime =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->rayTimeKey));
    }
    if (rayDataInfo->dispatchRaysIndexKey)
    {
        inputs.dispatchRaysIndex =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysIndexKey));
    }
    if (rayDataInfo->dispatchRaysDimensionsKey)
    {
        inputs.dispatchRaysDimensions = builder.emitLoad(
            builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysDimensionsKey));
    }
    if (inputs.dispatchRaysIndex || inputs.dispatchRaysDimensions)
    {
        dispatchValues[adapter] = {inputs.dispatchRaysIndex, inputs.dispatchRaysDimensions};
    }
    if (requirements.record)
    {
        inputs.record = _emitMetalRecordValueFromDataAddress(
            builder,
            recordDataAddress,
            cast<IRType>(group->getRecordType()));
    }
    if (requirements.hitKind)
    {
        if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle)
        {
            IRInst* operands[] = {
                inputs.triangleFrontFacing,
                builder.getIntValue(builder.getUIntType(), 254),
                builder.getIntValue(builder.getUIntType(), 255),
            };
            inputs.hitKind = builder.emitIntrinsicInst(
                builder.getUIntType(),
                kIROp_Select,
                SLANG_COUNT_OF(operands),
                operands);
        }
        else
        {
            inputs.hitKind = builder.getIntValue(builder.getUIntType(), 0);
        }
    }
    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn(_emitMetalCandidateResult(builder, resultInfo, true, true));

    _inlineCandidateOperationCalls(adapter);
    _lowerMetalCandidateInputOperations(adapter, inputs);
    _lowerAnyHitTerminations(adapter, resultInfo);
    generated.add(generatedKey, adapter);
    return adapter;
}

struct MetalProceduralCandidateState
{
    IRVar* hasCandidate = nullptr;
    IRVar* currentMaxDistance = nullptr;
    IRVar* distance = nullptr;
    IRVar* hitKind = nullptr;
    IRVar* attributes = nullptr;
    IRInst* record = nullptr;
    IRInst* minDistance = nullptr;
    IRInst* rayTime = nullptr;
    IRInst* rayFlags = nullptr;
    IRInst* worldSpaceOrigin = nullptr;
    IRInst* worldSpaceDirection = nullptr;
    IRInst* primitiveIndex = nullptr;
    IRInst* geometryIndex = nullptr;
    IRInst* instanceIndex = nullptr;
    IRInst* instanceID = nullptr;
    IRInst* objectSpaceOrigin = nullptr;
    IRInst* objectSpaceDirection = nullptr;
    IRInst* objectToWorld = nullptr;
    IRInst* worldToObject = nullptr;
    IRInst* dispatchRaysIndex = nullptr;
    IRInst* dispatchRaysDimensions = nullptr;
    IRInst* opaque = nullptr;
    IRInst* committedAttributes = nullptr;
    IRInst* committedHitKind = nullptr;
};

static void _collectReportHitOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectReportHitOperations(child, operations);
        if (child->getOp() == kIROp_StructuralRayTracingReportHit ||
            child->getOp() == kIROp_StructuralRayTracingReportHitWithKind)
        {
            operations.add(child);
        }
    }
}

static IRBlock* _splitBlockAfter(IRFunc* function, IRInst* inst, IRParam*& outResultParam)
{
    IRBuilder builder(function);
    auto continuation = builder.createBlock();
    function->addBlock(continuation);
    builder.setInsertInto(continuation);
    outResultParam = builder.emitParam(builder.getBoolType());

    for (auto suffix = inst->getNextInst(); suffix;)
    {
        auto next = suffix->getNextInst();
        suffix->insertAtEnd(continuation);
        suffix = next;
    }
    return continuation;
}

static void _emitBranchWithBool(IRBuilder& builder, IRBlock* target, bool value)
{
    auto argument = builder.getBoolValue(value);
    builder.emitBranch(target, 1, &argument);
}

static void _collectStageInputOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectStageInputOperations(child, operations);
        if (as<IRStructuralRayTracingStageInputOperation>(child))
            operations.add(child);
    }
}

static void _lowerAnyHitDecisionInputs(
    IRFunc* helper,
    IRInst* record,
    IRInst* attributes,
    IRInst* distance,
    IRInst* hitKind,
    IRInst* minDistance,
    IRInst* rayTime,
    IRInst* rayFlags,
    IRInst* worldSpaceOrigin,
    IRInst* worldSpaceDirection,
    IRInst* primitiveIndex,
    IRInst* geometryIndex,
    IRInst* instanceIndex,
    IRInst* instanceID,
    IRInst* objectSpaceOrigin,
    IRInst* objectSpaceDirection,
    IRInst* objectToWorld,
    IRInst* worldToObject,
    IRInst* dispatchRaysIndex,
    IRInst* dispatchRaysDimensions)
{
    List<IRInst*> operations;
    _collectStageInputOperations(helper, operations);
    for (auto operation : operations)
    {
        IRInst* replacement = nullptr;
        switch (operation->getOp())
        {
        case kIROp_StructuralRayTracingGetRecord:
            replacement = record;
            break;
        case kIROp_StructuralRayTracingGetHitAttributes:
            replacement = attributes;
            break;
        case kIROp_StructuralRayTracingGetRayTCurrent:
            replacement = distance;
            break;
        case kIROp_StructuralRayTracingGetRayTMin:
            replacement = minDistance;
            break;
        case kIROp_StructuralRayTracingGetRayTime:
            replacement = rayTime;
            break;
        case kIROp_StructuralRayTracingGetRayFlags:
            replacement = rayFlags;
            break;
        case kIROp_StructuralRayTracingGetHitKind:
            replacement = hitKind;
            break;
        case kIROp_StructuralRayTracingGetWorldRayOrigin:
            replacement = worldSpaceOrigin;
            break;
        case kIROp_StructuralRayTracingGetWorldRayDirection:
            replacement = worldSpaceDirection;
            break;
        case kIROp_StructuralRayTracingGetPrimitiveIndex:
            replacement = primitiveIndex;
            break;
        case kIROp_StructuralRayTracingGetGeometryIndex:
            replacement = geometryIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceIndex:
            replacement = instanceIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceID:
            replacement = instanceID;
            break;
        case kIROp_StructuralRayTracingGetObjectSpaceRay:
            {
                IRBuilder builder(operation);
                builder.setInsertBefore(operation);
                IRInst* fields[] = {
                    objectSpaceOrigin,
                    minDistance,
                    objectSpaceDirection,
                    distance,
                };
                replacement = builder.emitMakeStruct(
                    cast<IRType>(operation->getDataType()),
                    SLANG_COUNT_OF(fields),
                    fields);
                break;
            }
        case kIROp_StructuralRayTracingGetObjectToWorld:
            replacement = objectToWorld;
            break;
        case kIROp_StructuralRayTracingGetWorldToObject:
            replacement = worldToObject;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysIndex:
            replacement = dispatchRaysIndex;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysDimensions:
            replacement = dispatchRaysDimensions;
            break;
        default:
            break;
        }
        if (replacement)
        {
            operation->replaceUsesWith(replacement);
            operation->removeAndDeallocate();
        }
    }
}

static IRFunc* _generateAnyHitDecisionHelper(
    IRModule* module,
    const MetalCandidateResultInfo& resultInfo,
    IRStructuralRayTracingHitGroupInfoDecoration* group)
{
    auto invoke =
        getStructuralRayTracingHitGroupStageInvoke(group, StructuralRayTracingStageKind::AnyHit);
    if (!invoke)
        return nullptr;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto helper = builder.createFunc();
    builder.addForceInlineDecoration(helper);
    auto attributesType = cast<IRType>(group->getHitAttributesType());
    auto requirements = _getMetalStageRequirements(invoke);
    List<IRType*> parameterTypes;
    parameterTypes.add(attributesType);
    if (requirements.record)
        parameterTypes.add(cast<IRType>(group->getRecordType()));
    parameterTypes.add(builder.getFloatType());
    parameterTypes.add(builder.getUIntType());
    if (requirements.minDistance)
        parameterTypes.add(builder.getFloatType());
    if (requirements.rayTime)
        parameterTypes.add(builder.getFloatType());
    if (requirements.rayFlags)
        parameterTypes.add(builder.getUIntType());
    if (requirements.worldSpaceOrigin)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (requirements.worldSpaceDirection)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (requirements.primitiveIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.geometryIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.instanceIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.instanceID)
        parameterTypes.add(builder.getUIntType());
    if (requirements.objectSpaceRay)
    {
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    }
    if (requirements.objectToWorld)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (requirements.worldToObject)
        parameterTypes.add(_getFloat4x3Type(builder));
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    if (requirements.dispatchRaysIndex)
        parameterTypes.add(uint3Type);
    if (requirements.dispatchRaysDimensions)
        parameterTypes.add(uint3Type);
    parameterTypes.add(builder.getBoolType());
    helper->setFullType(builder.getFuncType(parameterTypes, resultInfo.type));

    auto name = _getMetalCandidateName(group->getGroupSourceTypeName(), toSlice(".anyHit"));
    builder.addNameHintDecoration(helper, name.getUnownedSlice());
    _addStructuralStageInfo(
        builder,
        helper,
        StructuralRayTracingStageKind::AnyHit,
        invoke,
        group->getAnyHitType(),
        group->getAnyHitSourceTypeName(),
        group->getAnyHitTypeIdentity(),
        group->getContextType(),
        group->getPayloadType(),
        group->getRecordType(),
        group->getHitAttributesType(),
        StructuralRayTracingHitAttributesKind::Custom);

    builder.setInsertInto(helper);
    builder.emitBlock();
    auto attributes = builder.emitParam(parameterTypes[0]);
    IRInst* record = nullptr;
    if (requirements.record)
        record = builder.emitParam(cast<IRType>(group->getRecordType()));
    auto distance = builder.emitParam(builder.getFloatType());
    auto hitKind = builder.emitParam(builder.getUIntType());
    IRInst* minDistance = nullptr;
    IRInst* rayTime = nullptr;
    IRInst* rayFlags = nullptr;
    if (requirements.minDistance)
        minDistance = builder.emitParam(builder.getFloatType());
    if (requirements.rayTime)
        rayTime = builder.emitParam(builder.getFloatType());
    if (requirements.rayFlags)
        rayFlags = builder.emitParam(builder.getUIntType());
    IRInst* worldSpaceOrigin = nullptr;
    IRInst* worldSpaceDirection = nullptr;
    if (requirements.worldSpaceOrigin)
    {
        worldSpaceOrigin = builder.emitParam(builder.getVectorType(builder.getFloatType(), 3));
        builder.addNameHintDecoration(
            worldSpaceOrigin,
            UnownedTerminatedStringSlice("worldSpaceOrigin"));
    }
    if (requirements.worldSpaceDirection)
    {
        worldSpaceDirection = builder.emitParam(builder.getVectorType(builder.getFloatType(), 3));
        builder.addNameHintDecoration(
            worldSpaceDirection,
            UnownedTerminatedStringSlice("worldSpaceDirection"));
    }
    IRInst* primitiveIndex = nullptr;
    IRInst* geometryIndex = nullptr;
    IRInst* instanceIndex = nullptr;
    IRInst* instanceID = nullptr;
    if (requirements.primitiveIndex)
        primitiveIndex = builder.emitParam(builder.getUIntType());
    if (requirements.geometryIndex)
        geometryIndex = builder.emitParam(builder.getUIntType());
    if (requirements.instanceIndex)
        instanceIndex = builder.emitParam(builder.getUIntType());
    if (requirements.instanceID)
        instanceID = builder.emitParam(builder.getUIntType());
    IRInst* objectSpaceOrigin = nullptr;
    IRInst* objectSpaceDirection = nullptr;
    IRInst* objectToWorld = nullptr;
    IRInst* worldToObject = nullptr;
    if (requirements.objectSpaceRay)
    {
        objectSpaceOrigin = builder.emitParam(builder.getVectorType(builder.getFloatType(), 3));
        objectSpaceDirection = builder.emitParam(builder.getVectorType(builder.getFloatType(), 3));
    }
    if (requirements.objectToWorld)
        objectToWorld = builder.emitParam(_getFloat4x3Type(builder));
    if (requirements.worldToObject)
        worldToObject = builder.emitParam(_getFloat4x3Type(builder));
    IRInst* dispatchRaysIndex = nullptr;
    IRInst* dispatchRaysDimensions = nullptr;
    if (requirements.dispatchRaysIndex)
        dispatchRaysIndex = builder.emitParam(uint3Type);
    if (requirements.dispatchRaysDimensions)
        dispatchRaysDimensions = builder.emitParam(uint3Type);
    auto opaque = builder.emitParam(builder.getBoolType());
    builder.addNameHintDecoration(attributes, UnownedTerminatedStringSlice("attributes"));
    builder.addNameHintDecoration(distance, UnownedTerminatedStringSlice("distance"));
    builder.addNameHintDecoration(hitKind, UnownedTerminatedStringSlice("hitKind"));
    builder.addNameHintDecoration(opaque, UnownedTerminatedStringSlice("opaque"));
    auto opaqueBlock = builder.createBlock();
    auto sourceBlock = builder.createBlock();
    helper->addBlock(opaqueBlock);
    helper->addBlock(sourceBlock);
    builder.emitIfElse(opaque, opaqueBlock, sourceBlock, sourceBlock);

    builder.setInsertInto(opaqueBlock);
    builder.emitReturn(_emitMetalCandidateResult(builder, resultInfo, true, true));

    builder.setInsertInto(sourceBlock);

    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn(_emitMetalCandidateResult(builder, resultInfo, true, true));

    _inlineCandidateOperationCalls(helper);
    _lowerAnyHitDecisionInputs(
        helper,
        record,
        attributes,
        distance,
        hitKind,
        minDistance,
        rayTime,
        rayFlags,
        worldSpaceOrigin,
        worldSpaceDirection,
        primitiveIndex,
        geometryIndex,
        instanceIndex,
        instanceID,
        objectSpaceOrigin,
        objectSpaceDirection,
        objectToWorld,
        worldToObject,
        dispatchRaysIndex,
        dispatchRaysDimensions);
    _lowerAnyHitTerminations(helper, resultInfo);
    return helper;
}

static void _lowerProceduralReportHitOperations(
    IRFunc* adapter,
    const MetalProceduralCandidateState& state,
    IRFunc* anyHitDecision,
    const MetalStageRequirements& anyHitRequirements,
    const MetalCandidateResultInfo& filterResultInfo,
    const MetalCandidateResultInfo& proceduralResultInfo)
{
    List<IRInst*> operations;
    _collectReportHitOperations(adapter, operations);

    for (auto operation : operations)
    {
        const bool hasHitKind = operation->getOp() == kIROp_StructuralRayTracingReportHitWithKind;
        auto distance = operation->getOperand(2);
        auto hitKind = hasHitKind ? operation->getOperand(3) : nullptr;
        auto attributes = operation->getOperand(hasHitKind ? 4 : 3);
        auto originalBlock = cast<IRBlock>(operation->getParent());

        IRParam* reportResult = nullptr;
        auto continuation = _splitBlockAfter(adapter, operation, reportResult);
        operation->replaceUsesWith(reportResult);
        operation->removeAndDeallocate();

        IRBuilder builder(adapter);
        auto effectiveHitKind = hitKind ? hitKind : builder.getIntValue(builder.getUIntType(), 0);
        auto acceptedBlock = builder.createBlock();
        auto rejectedBlock = builder.createBlock();
        adapter->addBlock(acceptedBlock);
        adapter->addBlock(rejectedBlock);

        builder.setInsertInto(originalBlock);
        auto aboveMin = builder.emitGeq(distance, state.minDistance);
        auto belowMax = builder.emitGeq(builder.emitLoad(state.currentMaxDistance), distance);
        auto inRange = builder.emitAnd(builder.getBoolType(), aboveMin, belowMax);
        builder.emitIfElse(inRange, acceptedBlock, rejectedBlock, continuation);

        builder.setInsertInto(acceptedBlock);
        IRInst* continueSearch = nullptr;
        if (anyHitDecision)
        {
            List<IRInst*> arguments;
            arguments.add(attributes);
            if (state.record)
                arguments.add(state.record);
            arguments.add(distance);
            arguments.add(effectiveHitKind);
            if (anyHitRequirements.minDistance)
                arguments.add(state.minDistance);
            if (anyHitRequirements.rayTime)
                arguments.add(state.rayTime);
            if (anyHitRequirements.rayFlags)
                arguments.add(state.rayFlags);
            if (state.worldSpaceOrigin)
                arguments.add(state.worldSpaceOrigin);
            if (state.worldSpaceDirection)
                arguments.add(state.worldSpaceDirection);
            if (anyHitRequirements.primitiveIndex)
                arguments.add(state.primitiveIndex);
            if (anyHitRequirements.geometryIndex)
                arguments.add(state.geometryIndex);
            if (anyHitRequirements.instanceIndex)
                arguments.add(state.instanceIndex);
            if (anyHitRequirements.instanceID)
                arguments.add(state.instanceID);
            if (anyHitRequirements.objectSpaceRay)
            {
                arguments.add(state.objectSpaceOrigin);
                arguments.add(state.objectSpaceDirection);
            }
            if (anyHitRequirements.objectToWorld)
                arguments.add(state.objectToWorld);
            if (anyHitRequirements.worldToObject)
                arguments.add(state.worldToObject);
            if (anyHitRequirements.dispatchRaysIndex)
                arguments.add(state.dispatchRaysIndex);
            if (anyHitRequirements.dispatchRaysDimensions)
                arguments.add(state.dispatchRaysDimensions);
            arguments.add(state.opaque);
            auto decision = builder.emitCallInst(
                filterResultInfo.type,
                anyHitDecision,
                arguments.getCount(),
                arguments.getBuffer());
            auto filterAccepted = builder.emitFieldExtract(decision, filterResultInfo.acceptKey);
            continueSearch = builder.emitFieldExtract(decision, filterResultInfo.continueSearchKey);

            auto filteredAcceptedBlock = builder.createBlock();
            adapter->addBlock(filteredAcceptedBlock);
            builder.emitIfElse(filterAccepted, filteredAcceptedBlock, rejectedBlock, continuation);
            builder.setInsertInto(filteredAcceptedBlock);
        }

        builder.emitStore(state.hasCandidate, builder.getBoolValue(true));
        builder.emitStore(state.currentMaxDistance, distance);
        builder.emitStore(state.distance, distance);
        builder.emitStore(state.hitKind, effectiveHitKind);
        builder.emitStore(state.attributes, attributes);
        if (state.committedAttributes)
            builder.emitStore(state.committedAttributes, attributes);
        if (state.committedHitKind)
            builder.emitStore(state.committedHitKind, effectiveHitKind);
        if (continueSearch)
        {
            auto continuingBlock = builder.createBlock();
            auto endingBlock = builder.createBlock();
            adapter->addBlock(continuingBlock);
            adapter->addBlock(endingBlock);
            builder.emitIfElse(continueSearch, continuingBlock, endingBlock, continuation);

            builder.setInsertInto(continuingBlock);
            _emitBranchWithBool(builder, continuation, true);

            builder.setInsertInto(endingBlock);
            builder.emitReturn(
                _emitMetalCandidateResult(builder, proceduralResultInfo, true, false, distance));
        }
        else
        {
            _emitBranchWithBool(builder, continuation, true);
        }

        builder.setInsertInto(rejectedBlock);
        _emitBranchWithBool(builder, continuation, false);
    }

    // Splitting a block for a later report-hit operation can append its new continuation after
    // blocks that it dominates. Restore dominance order because cloning a multi-block function
    // relies on block parameters being encountered before uses in dominated blocks.
    sortBlocksInFunc(adapter);
}

static void _lowerProceduralIntersectionInputs(
    IRFunc* adapter,
    const MetalProceduralCandidateState& state,
    IRInst* primitiveIndex,
    IRInst* geometryIndex,
    IRInst* instanceIndex,
    IRInst* instanceID)
{
    List<IRInst*> operations;
    _collectStageInputOperations(adapter, operations);
    for (auto operation : operations)
    {
        IRBuilder builder(operation);
        builder.setInsertBefore(operation);
        IRInst* replacement = nullptr;
        switch (operation->getOp())
        {
        case kIROp_StructuralRayTracingGetRecord:
            replacement = state.record;
            break;
        case kIROp_StructuralRayTracingGetRayTMin:
            replacement = state.minDistance;
            break;
        case kIROp_StructuralRayTracingGetRayTime:
            replacement = state.rayTime;
            break;
        case kIROp_StructuralRayTracingGetRayFlags:
            replacement = state.rayFlags;
            break;
        case kIROp_StructuralRayTracingGetObjectSpaceRay:
            {
                IRInst* values[] = {
                    state.objectSpaceOrigin,
                    state.minDistance,
                    state.objectSpaceDirection,
                    builder.emitLoad(state.currentMaxDistance),
                };
                replacement = builder.emitMakeStruct(
                    cast<IRType>(operation->getDataType()),
                    SLANG_COUNT_OF(values),
                    values);
                break;
            }
        case kIROp_StructuralRayTracingGetPrimitiveIndex:
            replacement = primitiveIndex;
            break;
        case kIROp_StructuralRayTracingGetGeometryIndex:
            replacement = geometryIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceIndex:
            replacement = instanceIndex;
            break;
        case kIROp_StructuralRayTracingGetInstanceID:
            replacement = instanceID;
            break;
        case kIROp_StructuralRayTracingGetWorldRayOrigin:
            replacement = state.worldSpaceOrigin;
            break;
        case kIROp_StructuralRayTracingGetWorldRayDirection:
            replacement = state.worldSpaceDirection;
            break;
        case kIROp_StructuralRayTracingGetObjectToWorld:
            replacement = state.objectToWorld;
            break;
        case kIROp_StructuralRayTracingGetWorldToObject:
            replacement = state.worldToObject;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysIndex:
            replacement = state.dispatchRaysIndex;
            break;
        case kIROp_StructuralRayTracingGetDispatchRaysDimensions:
            replacement = state.dispatchRaysDimensions;
            break;
        default:
            break;
        }
        if (replacement)
        {
            operation->replaceUsesWith(replacement);
            operation->removeAndDeallocate();
        }
    }
}

static IRFunc* _generateBoundingBoxCandidateAdapter(
    IRModule* module,
    Dictionary<KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*>, IRFunc*>& generated,
    HashSet<IRFunc*>& generatedHelpers,
    Dictionary<IRFunc*, IRInst*>& payloadValues,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues,
    const MetalCandidateResultInfo& filterResultInfo,
    const MetalCandidateResultInfo& proceduralResultInfo,
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    const MetalStageRequirements& signatureRequirements,
    bool signatureHasAnyHit,
    UInt tagMask,
    MetalRayDataInfo* rayDataInfo)
{
    auto invoke = getStructuralRayTracingHitGroupStageInvoke(
        group,
        StructuralRayTracingStageKind::Intersection);
    if (!invoke)
        return nullptr;
    auto groupType = group->getGroupType();
    KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*> generatedKey(
        KeyValuePair<IRInst*, UInt>(groupType, tagMask),
        rayDataInfo->type);
    if (auto existing = generated.tryGetValue(generatedKey))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    auto intersectionRequirements = _getMetalStageRequirements(invoke);
    auto anyHitRequirements = _getMetalStageRequirements(
        getStructuralRayTracingHitGroupStageInvoke(group, StructuralRayTracingStageKind::AnyHit));
    auto requirements =
        _combineMetalStageRequirements(intersectionRequirements, anyHitRequirements);
    List<IRType*> parameterTypes;
    parameterTypes.add(builder.getFloatType());
    parameterTypes.add(builder.getFloatType());
    if (signatureRequirements.objectSpaceRay)
    {
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    }
    if (signatureRequirements.objectToWorld)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (signatureRequirements.worldToObject)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (signatureRequirements.primitiveIndex)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.geometryIndex)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.instanceIndex)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.instanceID)
        parameterTypes.add(builder.getUIntType());
    if (signatureRequirements.worldSpaceOrigin)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (signatureRequirements.worldSpaceDirection)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (signatureHasAnyHit)
        parameterTypes.add(builder.getBoolType());
    parameterTypes.add(builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global));
    auto rayDataPointerType = builder.getPtrType(rayDataInfo->type, AddressSpace::ThreadLocal);
    parameterTypes.add(rayDataPointerType);
    adapter->setFullType(builder.getFuncType(parameterTypes, proceduralResultInfo.type));

    auto name = _getMetalCandidateName(group->getGroupSourceTypeName(), toSlice(".arm"));
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addForceInlineDecoration(adapter);
    _addStructuralStageInfo(
        builder,
        adapter,
        StructuralRayTracingStageKind::Intersection,
        invoke,
        group->getIntersectionType(),
        group->getIntersectionSourceTypeName(),
        group->getIntersectionTypeIdentity(),
        group->getContextType(),
        group->getPayloadType(),
        group->getRecordType(),
        group->getHitAttributesType(),
        StructuralRayTracingHitAttributesKind::Custom);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    auto minDistance = builder.emitParam(builder.getFloatType());
    builder.addNameHintDecoration(minDistance, UnownedTerminatedStringSlice("minDistance"));
    builder.addTargetSystemValueDecoration(minDistance, toSlice("min_distance"));
    auto maxDistance = builder.emitParam(builder.getFloatType());
    builder.addNameHintDecoration(maxDistance, UnownedTerminatedStringSlice("maxDistance"));
    builder.addTargetSystemValueDecoration(maxDistance, toSlice("max_distance"));
    IRInst* objectSpaceOrigin = nullptr;
    IRInst* objectSpaceDirection = nullptr;
    IRInst* primitiveIndex = nullptr;
    IRInst* geometryIndex = nullptr;
    IRInst* instanceIndex = nullptr;
    IRInst* instanceID = nullptr;
    if (signatureRequirements.objectSpaceRay)
    {
        objectSpaceOrigin = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "objectSpaceOrigin",
            "origin");
        objectSpaceDirection = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "objectSpaceDirection",
            "direction");
    }
    IRInst* objectToWorld = nullptr;
    IRInst* worldToObject = nullptr;
    if (signatureRequirements.objectToWorld)
    {
        objectToWorld = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "objectToWorld",
            "object_to_world_transform");
    }
    if (signatureRequirements.worldToObject)
    {
        worldToObject = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "worldToObject",
            "world_to_object_transform");
    }
    if (signatureRequirements.primitiveIndex)
    {
        primitiveIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "primitiveIndex",
            "primitive_id");
    }
    if (signatureRequirements.geometryIndex)
    {
        geometryIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "geometryIndex",
            "geometry_id");
    }
    if (signatureRequirements.instanceIndex)
    {
        instanceIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "instanceIndex",
            "instance_id");
    }
    if (signatureRequirements.instanceID)
    {
        instanceID = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "instanceID",
            "user_instance_id");
    }
    IRInst* worldSpaceOrigin = nullptr;
    IRInst* worldSpaceDirection = nullptr;
    IRInst* opaque = nullptr;
    if (signatureRequirements.worldSpaceOrigin)
    {
        worldSpaceOrigin = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceOrigin",
            "world_space_origin");
    }
    if (signatureRequirements.worldSpaceDirection)
    {
        worldSpaceDirection = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceDirection",
            "world_space_direction");
    }
    if (signatureHasAnyHit)
    {
        opaque = _emitMetalSystemValueParam(builder, builder.getBoolType(), "opaque", "opaque");
    }
    auto recordDataAddress =
        builder.emitParam(builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global));
    builder.addNameHintDecoration(recordDataAddress, UnownedTerminatedStringSlice("recordData"));
    auto rayData = builder.emitParam(rayDataPointerType);
    builder.addNameHintDecoration(rayData, UnownedTerminatedStringSlice("rayData"));
    payloadValues[adapter] = builder.emitFieldAddress(rayData, rayDataInfo->payloadKey);

    MetalProceduralCandidateState state;
    state.minDistance = minDistance;
    if (rayDataInfo->rayTimeKey)
    {
        state.rayTime =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->rayTimeKey));
    }
    if (rayDataInfo->rayFlagsKey)
    {
        state.rayFlags =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->rayFlagsKey));
    }
    state.primitiveIndex = primitiveIndex;
    state.geometryIndex = geometryIndex;
    state.instanceIndex = instanceIndex;
    state.instanceID = instanceID;
    state.hasCandidate = builder.emitVar(builder.getBoolType());
    state.currentMaxDistance = builder.emitVar(builder.getFloatType());
    state.distance = builder.emitVar(builder.getFloatType());
    state.hitKind = builder.emitVar(builder.getUIntType());
    state.attributes = builder.emitVar(cast<IRType>(group->getHitAttributesType()));
    if (requirements.record)
    {
        state.record = _emitMetalRecordValueFromDataAddress(
            builder,
            recordDataAddress,
            cast<IRType>(group->getRecordType()));
    }
    state.worldSpaceOrigin = worldSpaceOrigin;
    state.worldSpaceDirection = worldSpaceDirection;
    state.objectSpaceOrigin = objectSpaceOrigin;
    state.objectSpaceDirection = objectSpaceDirection;
    state.objectToWorld = objectToWorld;
    state.worldToObject = worldToObject;
    if (rayDataInfo->dispatchRaysIndexKey)
    {
        state.dispatchRaysIndex =
            builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysIndexKey));
    }
    if (rayDataInfo->dispatchRaysDimensionsKey)
    {
        state.dispatchRaysDimensions = builder.emitLoad(
            builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysDimensionsKey));
    }
    if (state.dispatchRaysIndex || state.dispatchRaysDimensions)
    {
        dispatchValues[adapter] = {state.dispatchRaysIndex, state.dispatchRaysDimensions};
    }
    state.opaque = opaque;
    if (auto key = rayDataInfo->customAttributeKeys.tryGetValue(groupType))
        state.committedAttributes = builder.emitFieldAddress(rayData, *key);
    if (rayDataInfo->customHitKindKey)
        state.committedHitKind = builder.emitFieldAddress(rayData, rayDataInfo->customHitKindKey);
    builder.addNameHintDecoration(state.hasCandidate, UnownedTerminatedStringSlice("hasCandidate"));
    builder.addNameHintDecoration(
        state.currentMaxDistance,
        UnownedTerminatedStringSlice("currentMaxDistance"));
    builder.addNameHintDecoration(
        state.distance,
        UnownedTerminatedStringSlice("candidateDistance"));
    builder.addNameHintDecoration(state.hitKind, UnownedTerminatedStringSlice("candidateHitKind"));
    builder.addNameHintDecoration(
        state.attributes,
        UnownedTerminatedStringSlice("candidateAttributes"));
    builder.emitStore(state.hasCandidate, builder.getBoolValue(false));
    builder.emitStore(state.currentMaxDistance, maxDistance);
    builder.emitStore(state.distance, builder.getFloatValue(builder.getFloatType(), 0.0));
    builder.emitStore(state.hitKind, builder.getIntValue(builder.getUIntType(), 0));
    builder.emitStore(
        state.attributes,
        builder.emitDefaultConstruct(cast<IRType>(group->getHitAttributesType())));

    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn(_emitMetalCandidateResult(
        builder,
        proceduralResultInfo,
        builder.emitLoad(state.hasCandidate),
        builder.getBoolValue(true),
        builder.emitLoad(state.distance)));

    _inlineCandidateOperationCalls(adapter);
    _lowerProceduralIntersectionInputs(
        adapter,
        state,
        primitiveIndex,
        geometryIndex,
        instanceIndex,
        instanceID);
    auto anyHitDecision = _generateAnyHitDecisionHelper(module, filterResultInfo, group);
    if (anyHitDecision)
        generatedHelpers.add(anyHitDecision);
    _lowerProceduralReportHitOperations(
        adapter,
        state,
        anyHitDecision,
        anyHitRequirements,
        filterResultInfo,
        proceduralResultInfo);
    generated.add(generatedKey, adapter);
    return adapter;
}

struct MetalCandidateDispatcherArm
{
    IRStructuralRayTracingHitGroupInfoDecoration* group = nullptr;
    IRFunc* helper = nullptr;
};

struct MetalCandidateDispatcherKey
{
    IRInst* schemaIdentity = nullptr;
    IRInst* rayDataType = nullptr;
    UInt tagMask = 0;
    IRIntegerValue maxLevels = 0;
    MetalStructuralRayTracingGeometryKind geometryKind =
        MetalStructuralRayTracingGeometryKind::Unknown;

    bool operator==(const MetalCandidateDispatcherKey& other) const
    {
        return schemaIdentity == other.schemaIdentity && rayDataType == other.rayDataType &&
               tagMask == other.tagMask && maxLevels == other.maxLevels &&
               geometryKind == other.geometryKind;
    }

    HashCode getHashCode() const
    {
        auto result =
            combineHash(Slang::getHashCode(schemaIdentity), Slang::getHashCode(rayDataType));
        result = combineHash(result, Slang::getHashCode(tagMask));
        result = combineHash(result, Slang::getHashCode(maxLevels));
        return combineHash(result, Slang::getHashCode(UInt(geometryKind)));
    }
};

static StructuralRayTracingMetalCandidateKind _getStructuralMetalCandidateKind(
    MetalStructuralRayTracingGeometryKind geometryKind)
{
    switch (geometryKind)
    {
    case MetalStructuralRayTracingGeometryKind::Triangle:
        return StructuralRayTracingMetalCandidateKind::Triangle;
    case MetalStructuralRayTracingGeometryKind::BoundingBox:
        return StructuralRayTracingMetalCandidateKind::BoundingBox;
    case MetalStructuralRayTracingGeometryKind::Curve:
        return StructuralRayTracingMetalCandidateKind::Curve;
    default:
        SLANG_UNEXPECTED("invalid Metal candidate dispatcher geometry");
    }
}

static void _copyMetalCandidateParameterDecorations(
    IRBuilder& builder,
    IRParam* destination,
    IRParam* source)
{
    if (auto nameHint = source->findDecoration<IRNameHintDecoration>())
        builder.addNameHintDecoration(destination, nameHint->getName());
    if (auto semantic = source->findDecoration<IRTargetSystemValueDecoration>())
        builder.addTargetSystemValueDecoration(destination, semantic->getSemantic());
}

// Creates the one native intersection function used by a payload partition and primitive kind.
// The host selects only this fixed IFT entry. The dispatcher then resolves the runtime SBT record,
// reads its function index, and invokes the matching source-stage arm with that record's data.
static IRFunc* _generateMetalCandidateDispatcher(
    IRModule* module,
    Dictionary<MetalCandidateDispatcherKey, IRFunc*>& generated,
    Dictionary<IRFunc*, IRParam*>& candidateRayDataParams,
    const MetalCandidateResultInfo& resultInfo,
    const List<MetalCandidateDispatcherArm>& arms,
    MetalStructuralRayTracingGeometryKind geometryKind,
    UInt tagMask,
    IRIntegerValue maxLevels,
    IRIntegerValue hitRecordStride,
    MetalRayDataInfo* rayDataInfo,
    IRInst* schemaIdentity,
    UnownedStringSlice physicalName)
{
    if (arms.getCount() == 0)
        return nullptr;

    // The exact export name and candidate arms are schema-owned. Keep that semantic identity in
    // the dedup key even though each partition currently also receives a fresh nominal ray-data
    // type; this prevents a future carrier canonicalization from cross-reusing physical symbols.
    MetalCandidateDispatcherKey key =
        {schemaIdentity, rayDataInfo->type, tagMask, maxLevels, geometryKind};
    if (auto existing = generated.tryGetValue(key))
        return *existing;

    auto signature = cast<IRFuncType>(arms[0].helper->getDataType());
    SLANG_RELEASE_ASSERT(signature->getParamCount() >= 2);
    auto helperRecordParameterIndex = signature->getParamCount() - 2;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto dispatcher = builder.createFunc();
    List<IRType*> parameterTypes;
    for (UInt i = 0; i < signature->getParamCount(); ++i)
    {
        if (i != helperRecordParameterIndex)
            parameterTypes.add(signature->getParamType(i));
    }
    dispatcher->setFullType(builder.getFuncType(parameterTypes, resultInfo.type));

    builder.addNameHintDecoration(dispatcher, physicalName);
    builder.addExportDecoration(dispatcher, physicalName);
    builder.addKeepAliveDecoration(dispatcher);
    IRInst* intersectionOperands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(geometryKind)),
        builder.getIntValue(builder.getIntType(), IRIntegerValue(tagMask)),
        builder.getIntValue(builder.getIntType(), maxLevels),
    };
    builder.addDecoration(
        dispatcher,
        kIROp_MetalIntersectionFunctionDecoration,
        intersectionOperands,
        SLANG_COUNT_OF(intersectionOperands));

    builder.setInsertInto(dispatcher);
    auto entryBlock = builder.emitBlock();
    List<IRInst*> dispatcherParameters;
    List<IRParam*> sourceParameters;
    for (auto parameter : arms[0].helper->getParams())
        sourceParameters.add(parameter);
    IRParam* rayData = nullptr;
    IRParam* geometryIndex = nullptr;
    IRParam* instanceIndex = nullptr;
    UInt dispatcherParameterIndex = 0;
    for (UInt helperParameterIndex = 0; helperParameterIndex < signature->getParamCount();
         ++helperParameterIndex)
    {
        if (helperParameterIndex == helperRecordParameterIndex)
            continue;
        auto sourceParameter = sourceParameters[helperParameterIndex];
        auto parameter = builder.emitParam(parameterTypes[dispatcherParameterIndex++]);
        _copyMetalCandidateParameterDecorations(builder, parameter, sourceParameter);
        dispatcherParameters.add(parameter);
        if (auto semantic = parameter->findDecoration<IRTargetSystemValueDecoration>())
        {
            if (semantic->getSemantic() == toSlice("geometry_id"))
                geometryIndex = parameter;
            else if (semantic->getSemantic() == toSlice("instance_id"))
                instanceIndex = parameter;
        }
        if (helperParameterIndex + 1 == signature->getParamCount())
            rayData = parameter;
    }
    SLANG_RELEASE_ASSERT(rayData && geometryIndex);
    candidateRayDataParams[dispatcher] = rayData;

    SLANG_RELEASE_ASSERT(
        rayDataInfo->recordDataKey && rayDataInfo->sbtOffsetKey && rayDataInfo->sbtStrideKey);
    auto descriptorData =
        builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->recordDataKey));
    auto sbtOffset = builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->sbtOffsetKey));
    auto sbtStride = builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->sbtStrideKey));
    auto uintType = builder.getUIntType();
    auto hitRecordIndex =
        builder.emitAdd(uintType, builder.emitMul(uintType, geometryIndex, sbtStride), sbtOffset);
    if ((tagMask & UInt(MetalStructuralRayTracingTag::Instancing)) != 0)
    {
        SLANG_RELEASE_ASSERT(instanceIndex);
        auto instanceTableByteOffset = builder.emitLoad(
            builder.emitGetOffsetPtr(descriptorData, builder.getIntValue(uintType, 0)));
        auto instanceTableWordOffset =
            builder.emitShr(uintType, instanceTableByteOffset, builder.getIntValue(uintType, 2));
        auto instanceTableIndex = builder.emitAdd(uintType, instanceTableWordOffset, instanceIndex);
        auto instanceContribution =
            builder.emitLoad(builder.emitGetOffsetPtr(descriptorData, instanceTableIndex));
        hitRecordIndex = builder.emitAdd(uintType, instanceContribution, hitRecordIndex);
    }
    auto hitRecordAddress = _emitMetalRecordAddress(
        builder,
        descriptorData,
        MetalDescriptorDataSection::HitRecords,
        hitRecordIndex,
        hitRecordStride);
    auto functionIndex = builder.emitLoad(
        builder.emitBitCast(builder.getPtrType(uintType, AddressSpace::Global), hitRecordAddress));
    auto recordDataAddress =
        builder.emitGetOffsetPtr(hitRecordAddress, builder.getIntValue(uintType, 16));

    auto defaultBlock = builder.createBlock();
    auto breakBlock = builder.createBlock();
    dispatcher->addBlock(defaultBlock);
    dispatcher->addBlock(breakBlock);
    List<IRInst*> switchCases;
    for (auto& arm : arms)
    {
        SLANG_RELEASE_ASSERT(arm.helper->getDataType() == signature);
        auto caseBlock = builder.createBlock();
        dispatcher->addBlock(caseBlock);
        switchCases.add(arm.group->getFunctionIndex());
        switchCases.add(caseBlock);
        builder.setInsertInto(caseBlock);
        List<IRInst*> arguments;
        UInt wrapperIndex = 0;
        for (UInt helperIndex = 0; helperIndex < signature->getParamCount(); ++helperIndex)
        {
            if (helperIndex == helperRecordParameterIndex)
                arguments.add(recordDataAddress);
            else
                arguments.add(dispatcherParameters[wrapperIndex++]);
        }
        auto candidateResult = builder.emitCallInst(
            resultInfo.type,
            arm.helper,
            arguments.getCount(),
            arguments.getBuffer());
        builder.emitReturn(candidateResult);
    }

    builder.setInsertInto(defaultBlock);
    bool defaultAccept = geometryKind != MetalStructuralRayTracingGeometryKind::BoundingBox;
    builder.emitReturn(_emitMetalCandidateResult(
        builder,
        resultInfo,
        defaultAccept,
        true,
        builder.getFloatValue(builder.getFloatType(), 0.0)));
    builder.setInsertInto(breakBlock);
    builder.emitUnreachable();
    builder.setInsertInto(entryBlock);
    builder.emitSwitch(
        functionIndex,
        breakBlock,
        defaultBlock,
        switchCases.getCount(),
        switchCases.getBuffer());

    generated.add(key, dispatcher);
    return dispatcher;
}

static void _collectReturns(IRInst* parent, List<IRReturn*>& returns)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectReturns(child, returns);
        if (auto returnInst = as<IRReturn>(child))
            returns.add(returnInst);
    }
}

static void _convertCandidateParameterToRayData(IRFunc* adapter, IRParam* rayDataParam)
{
    auto rayDataPointerType = cast<IRPtrTypeBase>(rayDataParam->getDataType());
    auto rayDataType = rayDataPointerType->getValueType();

    auto firstBlock = adapter->getFirstBlock();
    SLANG_ASSERT(firstBlock);
    auto firstOrdinaryInst = firstBlock->getFirstOrdinaryInst();
    SLANG_ASSERT(firstOrdinaryInst);

    IRBuilder builder(adapter);
    builder.setInsertBefore(firstOrdinaryInst);
    auto rayDataStorage = builder.emitVar(rayDataType);
    builder.addNameHintDecoration(rayDataStorage, UnownedTerminatedStringSlice("rayDataStorage"));
    rayDataParam->replaceUsesWith(rayDataStorage);
    builder.emitStore(rayDataStorage, builder.emitLoad(rayDataParam));

    List<IRReturn*> returns;
    _collectReturns(adapter, returns);
    for (auto returnInst : returns)
    {
        builder.setInsertBefore(returnInst);
        builder.emitStore(rayDataParam, builder.emitLoad(rayDataStorage));
    }

    rayDataParam->setFullType(builder.getRefParamType(rayDataType, AddressSpace::Generic));
    builder.addTargetSystemValueDecoration(rayDataParam, toSlice("payload"));
    fixUpFuncType(adapter);
}

static void _getStructFields(IRStructType* type, List<IRStructField*>& fields)
{
    for (auto field : type->getFields())
        fields.add(field);
}

struct MetalTraceDescriptorInfo
{
    IRStructField* descriptorResourcesField = nullptr;
    IRPtrType* descriptorResourcesPointerType = nullptr;
    IRStructField* intersectionFunctionsField = nullptr;
    IRStructField* missFunctionsField = nullptr;
    IRStructField* closestHitFunctionsField = nullptr;
    IRStructField* callableFunctionsField = nullptr;
    IRStructField* recordsField = nullptr;
    IRType* intersectionFunctionTableType = nullptr;
    IRType* missFunctionTableType = nullptr;
    IRType* closestHitFunctionTableType = nullptr;
    IRType* callableFunctionTableType = nullptr;
};

struct MetalPayloadPartitionDescriptorInfo
{
    Index payloadIndex = -1;
    IRType* payloadType = nullptr;
    // These are schema-wide facts for this payload partition. They are collected from every
    // structural trace before any table or adapter is materialized, so neither generated MSL nor
    // post-emit metadata depends on which trace operation happens to be visited first.
    UInt tagMask = 0;
    IRIntegerValue maxLevels = 0;
    MetalStageRequirements missRequirements;
    MetalStageRequirements closestHitRequirements;
    RefPtr<MetalRayDataInfo> rayDataInfo;
    IRStructField* intersectionFunctionsField = nullptr;
    IRStructField* missFunctionsField = nullptr;
    IRStructField* closestHitFunctionsField = nullptr;
    IRType* intersectionFunctionTableType = nullptr;
    IRType* missFunctionTableType = nullptr;
    IRType* closestHitFunctionTableType = nullptr;
};

// Stores the one physical Metal descriptor shape synthesized for a concrete program schema.
//
// Hit and miss functions exchange the payload through their visible-function signature, so two
// payload types cannot share a table even when they belong to the same logical SBT. The schema
// therefore owns an ordered list of payload partitions, while callable functions and records stay
// schema-wide.
class MetalProgramDescriptorInfo : public RefObject
{
public:
    IRInst* programLayout = nullptr;
    IRStringLit* programLayoutSourceTypeName = nullptr;
    IRInst* descriptor = nullptr;
    IRInst* representativeSchemaOperation = nullptr;
    IRStructType* sourceDescriptorType = nullptr;
    IRStructType* physicalDescriptorType = nullptr;
    IRStructField* sourceDescriptorResourcesField = nullptr;
    List<IRStructField*> sourceResourceFields;
    IRStructField* descriptorResourcesField = nullptr;
    IRPtrType* descriptorResourcesPointerType = nullptr;
    IRStructField* callableFunctionsField = nullptr;
    IRStructField* recordsField = nullptr;
    IRType* callableFunctionTableType = nullptr;
    IRType* callableDataType = nullptr;
    MetalStageRequirements callableRequirements;
    bool arePayloadEntriesMaterialized = false;
    bool areCallableEntriesMaterialized = false;
    // The record buffer has one schema-wide offset for each section. Consequently every payload
    // partition must use these same hit and miss strides when resolving a physical record index.
    IRIntegerValue hitRecordStride = 16;
    IRIntegerValue missRecordStride = 16;
    IRIntegerValue callableRecordStride = 16;
    List<IRInst*> operations;
    List<IRInst*> descriptorValues;
    HashSet<IRInst*> descriptorValueSet;
    List<IRStructField*> storageFields;
    HashSet<IRStructField*> storageFieldSet;
    List<IRInst*> directStorageValues;
    HashSet<IRInst*> directStorageValueSet;
    Dictionary<IRTypeLayout*, IRTypeLayout*> physicalDescriptorLayouts;
    List<MetalPayloadPartitionDescriptorInfo> payloadPartitions;
    HashSet<IRType*> payloadTypes;

    MetalPayloadPartitionDescriptorInfo* findPayloadPartition(IRType* payloadType)
    {
        for (auto& partition : payloadPartitions)
        {
            if (partition.payloadType == payloadType)
                return &partition;
        }
        return nullptr;
    }

    void addPayloadType(IRType* payloadType)
    {
        if (!payloadTypes.add(payloadType))
            return;
        MetalPayloadPartitionDescriptorInfo partition;
        partition.payloadIndex = payloadPartitions.getCount();
        partition.payloadType = payloadType;
        payloadPartitions.add(partition);
    }

    void addDescriptorValue(IRInst* value)
    {
        if (descriptorValueSet.add(value))
            descriptorValues.add(value);
    }

    void addOperation(IRInst* operation) { operations.add(operation); }

    void addStorageField(IRStructField* field)
    {
        if (storageFieldSet.add(field))
            storageFields.add(field);
    }

    void addDirectStorageValue(IRInst* value)
    {
        if (directStorageValueSet.add(value))
            directStorageValues.add(value);
    }
};

static IRStructFieldLayoutAttr* _findStructFieldLayout(IRStructTypeLayout* layout, IRInst* fieldKey)
{
    for (auto fieldLayout : layout->getFieldLayoutAttrs())
    {
        if (fieldLayout->getFieldKey() == fieldKey)
            return fieldLayout;
    }
    return nullptr;
}

static void _copyMetalTypeLayoutAttributes(IRTypeLayout* source, IRTypeLayout::Builder& destination)
{
    for (auto sizeAttr : source->getSizeAttrs())
        destination.addResourceUsage(sizeAttr);
    for (auto alignmentAttr : source->getAlignmentAttrs())
        destination.addAlignment(alignmentAttr);
}

static IRVarLayout* _cloneMetalVarLayout(
    IRBuilder& builder,
    IRVarLayout* source,
    IRTypeLayout* typeLayout)
{
    IRVarLayout::Builder result(&builder, typeLayout);
    result.cloneEverythingButOffsetsFrom(source);
    for (auto offsetAttr : source->getOffsetAttrs())
    {
        auto offset = result.findOrAddResourceInfo(offsetAttr->getResourceKind());
        offset->offset = offsetAttr->getOffset();
        offset->space = offsetAttr->getSpace();
    }
    return result.build();
}

static void _getMetalPhysicalDescriptorFields(
    MetalProgramDescriptorInfo* info,
    List<IRStructField*>& fields)
{
    for (auto& partition : info->payloadPartitions)
    {
        fields.add(partition.intersectionFunctionsField);
        fields.add(partition.missFunctionsField);
        fields.add(partition.closestHitFunctionsField);
    }
    fields.add(info->callableFunctionsField);
    fields.add(info->recordsField);
}

// Build the target layout that corresponds to one source descriptor layout. This follows the
// descriptor's exact type-layout path rather than replacing every layout with the standard
// module's shared field keys: specialization erases the phantom `Schema` parameter from the source
// struct, so those keys alone cannot distinguish two schemas in one linked module.
static IRTypeLayout* _getMetalPhysicalDescriptorLayout(
    IRBuilder& builder,
    MetalProgramDescriptorInfo* info,
    IRTypeLayout* sourceTypeLayout)
{
    if (auto existing = info->physicalDescriptorLayouts.tryGetValue(sourceTypeLayout))
        return *existing;

    auto sourceDescriptorLayout = as<IRStructTypeLayout>(sourceTypeLayout);
    SLANG_RELEASE_ASSERT(sourceDescriptorLayout);
    auto sourceResourcesFieldLayout = _findStructFieldLayout(
        sourceDescriptorLayout,
        info->sourceDescriptorResourcesField->getKey());
    SLANG_RELEASE_ASSERT(sourceResourcesFieldLayout);
    auto sourceParameterGroupLayout =
        as<IRParameterGroupTypeLayout>(sourceResourcesFieldLayout->getLayout()->getTypeLayout());
    SLANG_RELEASE_ASSERT(sourceParameterGroupLayout);
    auto sourceResourcesLayout =
        as<IRStructTypeLayout>(sourceParameterGroupLayout->getElementVarLayout()->getTypeLayout());
    SLANG_RELEASE_ASSERT(sourceResourcesLayout);

    List<IRStructField*> physicalFields;
    _getMetalPhysicalDescriptorFields(info, physicalFields);
    auto sourceTableFieldLayout =
        _findStructFieldLayout(sourceResourcesLayout, info->sourceResourceFields[0]->getKey());
    SLANG_RELEASE_ASSERT(sourceTableFieldLayout);
    auto sourceTableVarLayout = sourceTableFieldLayout->getLayout();

    IRStructTypeLayout::Builder resourcesLayoutBuilder(&builder);
    for (auto sizeAttr : sourceResourcesLayout->getSizeAttrs())
    {
        if (sizeAttr->getResourceKind() == LayoutResourceKind::MetalArgumentBufferElement)
        {
            resourcesLayoutBuilder.addResourceUsage(
                LayoutResourceKind::MetalArgumentBufferElement,
                LayoutSize(physicalFields.getCount()));
        }
        else
        {
            resourcesLayoutBuilder.addResourceUsage(sizeAttr);
        }
    }
    for (auto alignmentAttr : sourceResourcesLayout->getAlignmentAttrs())
        resourcesLayoutBuilder.addAlignment(alignmentAttr);
    for (Index i = 0; i < physicalFields.getCount(); ++i)
    {
        auto fieldLayout = _cloneMetalVarLayout(
            builder,
            sourceTableVarLayout,
            sourceTableVarLayout->getTypeLayout());
        auto argumentBufferOffset =
            fieldLayout->findOffsetAttr(LayoutResourceKind::MetalArgumentBufferElement);
        SLANG_RELEASE_ASSERT(argumentBufferOffset);
        IRVarLayout::Builder indexedFieldLayoutBuilder(&builder, fieldLayout->getTypeLayout());
        indexedFieldLayoutBuilder.cloneEverythingButOffsetsFrom(fieldLayout);
        for (auto offsetAttr : fieldLayout->getOffsetAttrs())
        {
            auto offset =
                indexedFieldLayoutBuilder.findOrAddResourceInfo(offsetAttr->getResourceKind());
            offset->offset =
                offsetAttr->getResourceKind() == LayoutResourceKind::MetalArgumentBufferElement
                    ? LayoutOffset(i)
                    : offsetAttr->getOffset();
            offset->space = offsetAttr->getSpace();
        }
        resourcesLayoutBuilder.addField(
            physicalFields[i]->getKey(),
            indexedFieldLayoutBuilder.build());
    }
    auto physicalResourcesLayout = resourcesLayoutBuilder.build();

    IRParameterGroupTypeLayout::Builder parameterGroupLayoutBuilder(&builder);
    _copyMetalTypeLayoutAttributes(sourceParameterGroupLayout, parameterGroupLayoutBuilder);
    parameterGroupLayoutBuilder.setContainerVarLayout(
        sourceParameterGroupLayout->getContainerVarLayout());
    parameterGroupLayoutBuilder.setElementVarLayout(_cloneMetalVarLayout(
        builder,
        sourceParameterGroupLayout->getElementVarLayout(),
        physicalResourcesLayout));
    parameterGroupLayoutBuilder.setOffsetElementTypeLayout(physicalResourcesLayout);
    auto physicalParameterGroupLayout = parameterGroupLayoutBuilder.build();

    IRStructTypeLayout::Builder descriptorLayoutBuilder(&builder);
    _copyMetalTypeLayoutAttributes(sourceDescriptorLayout, descriptorLayoutBuilder);
    descriptorLayoutBuilder.addField(
        info->descriptorResourcesField->getKey(),
        _cloneMetalVarLayout(
            builder,
            sourceResourcesFieldLayout->getLayout(),
            physicalParameterGroupLayout));
    auto physicalDescriptorLayout = descriptorLayoutBuilder.build();
    info->physicalDescriptorLayouts.add(sourceTypeLayout, physicalDescriptorLayout);
    return physicalDescriptorLayout;
}

static IRStructField* _findMetalDescriptorStorageField(
    IRBuilder& builder,
    IRInst* aggregate,
    IRInst* fieldKey)
{
    auto aggregateType = cast<IRType>(aggregate->getDataType());
    auto structType = as<IRStructType>(tryGetPointedToType(&builder, aggregateType));
    if (!structType)
        structType = as<IRStructType>(aggregateType);
    return structType ? findStructField(structType, cast<IRStructKey>(fieldKey)) : nullptr;
}

static void _retagMetalDescriptorValue(
    IRBuilder& builder,
    MetalProgramDescriptorInfo* info,
    IRInst* value,
    Dictionary<IRInst*, IRType*>& physicalTypesByValue);

static void _retagMetalDescriptorPointer(
    IRBuilder& builder,
    MetalProgramDescriptorInfo* info,
    IRInst* pointer,
    Dictionary<IRInst*, IRType*>& physicalTypesByValue)
{
    auto pointerType = as<IRPtrTypeBase>(pointer->getDataType());
    SLANG_RELEASE_ASSERT(pointerType);
    auto physicalPointerType =
        builder.getPtrTypeWithAddressSpace(info->physicalDescriptorType, pointerType);
    if (auto previousType = physicalTypesByValue.tryGetValue(pointer))
    {
        SLANG_RELEASE_ASSERT(*previousType == physicalPointerType);
        return;
    }
    physicalTypesByValue.add(pointer, physicalPointerType);
    pointer->setFullType(physicalPointerType);

    if (auto fieldAddress = as<IRFieldAddress>(pointer))
    {
        auto storageField = _findMetalDescriptorStorageField(
            builder,
            fieldAddress->getBase(),
            fieldAddress->getField());
        SLANG_RELEASE_ASSERT(storageField);
        auto oldFieldType = storageField->getFieldType();
        SLANG_RELEASE_ASSERT(
            oldFieldType == info->sourceDescriptorType ||
            oldFieldType == info->physicalDescriptorType);
        storageField->setFieldType(info->physicalDescriptorType);
        info->addStorageField(storageField);
        return;
    }
    if (as<IRVar>(pointer))
    {
        for (auto use = pointer->firstUse; use; use = use->nextUse)
        {
            if (auto store = as<IRStore>(use->getUser()))
                _retagMetalDescriptorValue(builder, info, store->getVal(), physicalTypesByValue);
        }
        return;
    }
    SLANG_RELEASE_ASSERT(as<IRParam>(pointer) || as<IRGlobalParam>(pointer));
}

// Give every descriptor value selected by a structural operation the target-specific nominal type
// keyed by that operation's `programLayout`. The source generic's Schema argument is phantom and
// has already been erased by specialization; preserving the ordinary shared source struct here
// would make two schemas overwrite each other's table types.
static void _retagMetalDescriptorValue(
    IRBuilder& builder,
    MetalProgramDescriptorInfo* info,
    IRInst* value,
    Dictionary<IRInst*, IRType*>& physicalTypesByValue)
{
    if (auto previousType = physicalTypesByValue.tryGetValue(value))
    {
        SLANG_RELEASE_ASSERT(*previousType == info->physicalDescriptorType);
        return;
    }
    auto sourceType = value->getDataType();
    physicalTypesByValue.add(value, info->physicalDescriptorType);
    value->setFullType(info->physicalDescriptorType);

    if (auto load = as<IRLoad>(value))
    {
        _retagMetalDescriptorPointer(builder, info, load->getPtr(), physicalTypesByValue);
        return;
    }
    if (auto fieldExtract = as<IRFieldExtract>(value))
    {
        auto storageField = _findMetalDescriptorStorageField(
            builder,
            fieldExtract->getBase(),
            fieldExtract->getField());
        SLANG_RELEASE_ASSERT(storageField);
        SLANG_RELEASE_ASSERT(
            storageField->getFieldType() == sourceType ||
            storageField->getFieldType() == info->physicalDescriptorType);
        storageField->setFieldType(info->physicalDescriptorType);
        info->addStorageField(storageField);
        return;
    }
    if (as<IRGlobalParam>(value))
    {
        info->addDirectStorageValue(value);
        return;
    }
    if (auto param = as<IRParam>(value))
    {
        auto block = as<IRBlock>(param->getParent());
        auto func = block ? as<IRFunc>(block->getParent()) : nullptr;
        SLANG_RELEASE_ASSERT(func && block == func->getFirstBlock());
        Index parameterIndex = 0;
        for (auto candidate : func->getParams())
        {
            if (candidate == param)
                break;
            ++parameterIndex;
        }
        SLANG_RELEASE_ASSERT(parameterIndex < Index(func->getParamCount()));
        fixUpFuncType(func);

        List<IRCall*> callSites;
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            auto call = as<IRCall>(use->getUser());
            if (call && call->getCallee() == func)
                callSites.add(call);
        }
        for (auto call : callSites)
        {
            _retagMetalDescriptorValue(
                builder,
                info,
                call->getArg(UInt(parameterIndex)),
                physicalTypesByValue);
        }
        return;
    }

    // Descriptor construction is unavailable to user code, so after specialization every
    // structural operation must receive it from a parameter, a global parameter, or a load/extract
    // rooted in one of those storage locations.
    SLANG_RELEASE_ASSERT(!"unexpected structural ray-tracing descriptor producer");
}

static void _collectMetalStructTypeLayouts(IRInst* parent, List<IRStructTypeLayout*>& layouts)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectMetalStructTypeLayouts(child, layouts);
        if (auto layout = as<IRStructTypeLayout>(child))
            layouts.add(layout);
    }
}

static void _replaceMetalDescriptorStorageLayouts(
    IRModule* module,
    MetalProgramDescriptorInfo* info)
{
    IRBuilder builder(module);
    for (auto storageValue : info->directStorageValues)
    {
        auto layoutDecoration = storageValue->findDecoration<IRLayoutDecoration>();
        if (!layoutDecoration)
            continue;
        auto sourceVarLayout = cast<IRVarLayout>(layoutDecoration->getLayout());
        auto physicalTypeLayout =
            _getMetalPhysicalDescriptorLayout(builder, info, sourceVarLayout->getTypeLayout());
        layoutDecoration->setOperand(
            0,
            _cloneMetalVarLayout(builder, sourceVarLayout, physicalTypeLayout));
    }

    List<IRStructTypeLayout*> layouts;
    _collectMetalStructTypeLayouts(module->getModuleInst(), layouts);
    for (auto oldLayout : layouts)
    {
        IRStructField* matchingStorageField = nullptr;
        IRStructFieldLayoutAttr* matchingFieldLayout = nullptr;
        for (auto storageField : info->storageFields)
        {
            if (auto fieldLayout = _findStructFieldLayout(oldLayout, storageField->getKey()))
            {
                matchingStorageField = storageField;
                matchingFieldLayout = fieldLayout;
                break;
            }
        }
        if (!matchingStorageField)
            continue;

        IRStructTypeLayout::Builder newLayoutBuilder(&builder);
        _copyMetalTypeLayoutAttributes(oldLayout, newLayoutBuilder);
        for (auto fieldLayout : oldLayout->getFieldLayoutAttrs())
        {
            auto varLayout = fieldLayout->getLayout();
            if (fieldLayout == matchingFieldLayout)
            {
                varLayout = _cloneMetalVarLayout(
                    builder,
                    varLayout,
                    _getMetalPhysicalDescriptorLayout(builder, info, varLayout->getTypeLayout()));
            }
            newLayoutBuilder.addField(fieldLayout->getFieldKey(), varLayout);
        }
        oldLayout->replaceUsesWith(newLayoutBuilder.build());
    }
}

static IRStructField* _createMetalDescriptorResourceField(
    IRBuilder& builder,
    IRStructType* resourcesType,
    IRType* placeholderType,
    StructuralRayTracingDescriptorResourceKind kind,
    Index payloadIndex,
    Index payloadCount)
{
    auto key = builder.createStructKey();
    auto name =
        getStructuralRayTracingMetalDescriptorResourceName(kind, payloadIndex, payloadCount);
    builder.addNameHintDecoration(key, name.getUnownedSlice());
    return builder.createStructField(resourcesType, key, placeholderType);
}

// Replace the fixed source placeholder with the target-specific `3 * payloadCount + 2` resource
// struct. Payload partitions have already been collected in shader declaration order.
static bool _synthesizeMetalProgramDescriptor(
    IRModule* module,
    MetalProgramDescriptorInfo* info,
    Dictionary<IRInst*, IRType*>& physicalTypesByValue)
{
    auto descriptorType = as<IRStructType>(info->descriptor->getDataType());
    if (!descriptorType)
        return false;

    List<IRStructField*> descriptorFields;
    _getStructFields(descriptorType, descriptorFields);
    if (descriptorFields.getCount() != 1)
        return false;

    auto sourceParameterBlock =
        as<IRUniformParameterGroupType>(descriptorFields[0]->getFieldType());
    auto sourceResourcesType =
        sourceParameterBlock ? as<IRStructType>(sourceParameterBlock->getElementType()) : nullptr;
    if (!sourceResourcesType)
        return false;

    List<IRStructField*> sourceFields;
    _getStructFields(sourceResourcesType, sourceFields);
    if (sourceFields.getCount() != 5)
        return false;
    info->sourceDescriptorType = descriptorType;
    info->sourceDescriptorResourcesField = descriptorFields[0];
    for (auto field : sourceFields)
        info->sourceResourceFields.add(field);

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto physicalResourcesType = builder.createStructType();
    if (auto nameHint = sourceResourcesType->findDecoration<IRNameHintDecoration>())
        builder.addNameHintDecoration(physicalResourcesType, nameHint->getName());

    auto tablePlaceholderType = sourceFields[0]->getFieldType();
    auto payloadCount = info->payloadPartitions.getCount();
    for (Index i = 0; i < info->payloadPartitions.getCount(); ++i)
    {
        auto& partition = info->payloadPartitions[i];
        partition.intersectionFunctionsField = _createMetalDescriptorResourceField(
            builder,
            physicalResourcesType,
            tablePlaceholderType,
            StructuralRayTracingDescriptorResourceKind::IntersectionFunctionTable,
            i,
            payloadCount);
        partition.missFunctionsField = _createMetalDescriptorResourceField(
            builder,
            physicalResourcesType,
            tablePlaceholderType,
            StructuralRayTracingDescriptorResourceKind::MissVisibleFunctionTable,
            i,
            payloadCount);
        partition.closestHitFunctionsField = _createMetalDescriptorResourceField(
            builder,
            physicalResourcesType,
            tablePlaceholderType,
            StructuralRayTracingDescriptorResourceKind::ClosestHitVisibleFunctionTable,
            i,
            payloadCount);
    }
    info->callableFunctionsField = _createMetalDescriptorResourceField(
        builder,
        physicalResourcesType,
        sourceFields[3]->getFieldType(),
        StructuralRayTracingDescriptorResourceKind::CallableVisibleFunctionTable,
        -1,
        payloadCount);
    info->recordsField = _createMetalDescriptorResourceField(
        builder,
        physicalResourcesType,
        sourceFields[4]->getFieldType(),
        StructuralRayTracingDescriptorResourceKind::Records,
        -1,
        payloadCount);
    List<IRInst*> parameterBlockOperands;
    parameterBlockOperands.add(physicalResourcesType);
    for (UInt i = 1; i < sourceParameterBlock->getOperandCount(); ++i)
        parameterBlockOperands.add(sourceParameterBlock->getOperand(i));
    auto physicalParameterBlock = builder.getType(
        sourceParameterBlock->getOp(),
        parameterBlockOperands.getCount(),
        parameterBlockOperands.getBuffer());
    info->physicalDescriptorType = builder.createStructType();
    if (auto nameHint = descriptorType->findDecoration<IRNameHintDecoration>())
        builder.addNameHintDecoration(info->physicalDescriptorType, nameHint->getName());
    info->descriptorResourcesField = builder.createStructField(
        info->physicalDescriptorType,
        descriptorFields[0]->getKey(),
        physicalParameterBlock);

    info->descriptorResourcesPointerType =
        builder.getPtrType(builder.getUIntType(), AddressSpace::Uniform);
    for (auto descriptorValue : info->descriptorValues)
        _retagMetalDescriptorValue(builder, info, descriptorValue, physicalTypesByValue);
    _replaceMetalDescriptorStorageLayouts(module, info);
    return true;
}

static IRFuncType* _getMetalVisibleFunctionSignature(
    IRBuilder& builder,
    IRType* rayDataType,
    const MetalStageRequirements& requirements,
    IRType* descriptorResourcesPointerType)
{
    List<IRType*> parameterTypes;
    parameterTypes.add(builder.getPtrType(rayDataType, AddressSpace::ThreadLocal));
    if (requirements.distance || requirements.objectSpaceRay)
        parameterTypes.add(builder.getFloatType());
    if (requirements.hitKind)
        parameterTypes.add(builder.getUIntType());
    if (requirements.triangleBarycentricCoord)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 2));
    if (requirements.triangleFrontFacing)
        parameterTypes.add(builder.getBoolType());
    if (requirements.curveParameter)
        parameterTypes.add(builder.getFloatType());
    if (requirements.worldSpaceOrigin)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (requirements.worldSpaceDirection)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (requirements.primitiveIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.geometryIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.instanceIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.instanceID)
        parameterTypes.add(builder.getUIntType());
    if (requirements.objectSpaceRay)
    {
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    }
    if (requirements.objectToWorld)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (requirements.worldToObject)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (requirements.record)
        parameterTypes.add(builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global));
    if (requirements.callableDispatch)
        parameterTypes.add(descriptorResourcesPointerType);
    return builder.getFuncType(parameterTypes, builder.getVoidType());
}

static bool _prepareTraceDescriptor(
    IRBuilder& builder,
    MetalProgramDescriptorInfo* programInfo,
    MetalPayloadPartitionDescriptorInfo* partition,
    MetalRayDataInfo* rayDataInfo,
    MetalTraceDescriptorInfo& outInfo)
{
    if (!partition || partition->rayDataInfo.Ptr() != rayDataInfo)
        return false;

    auto descriptorResourcesPointerType = programInfo->descriptorResourcesPointerType;

    auto intType = builder.getIntType();
    auto tagMask = builder.getIntValue(intType, IRIntegerValue(partition->tagMask));
    auto maxLevels = builder.getIntValue(intType, partition->maxLevels);
    IRInst* intersectionTableOperands[] = {tagMask, maxLevels};
    auto intersectionFunctionTableType = builder.getType(
        kIROp_MetalIntersectionFunctionTable,
        SLANG_COUNT_OF(intersectionTableOperands),
        intersectionTableOperands);

    IRInst* missTableTypeOperands[] = {
        _getMetalVisibleFunctionSignature(
            builder,
            rayDataInfo->type,
            partition->missRequirements,
            descriptorResourcesPointerType),
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(StructuralRayTracingStageKind::Miss)),
    };
    auto missFunctionTableType = builder.getType(
        kIROp_MetalVisibleFunctionTable,
        SLANG_COUNT_OF(missTableTypeOperands),
        missTableTypeOperands);
    IRInst* closestHitTableTypeOperands[] = {
        _getMetalVisibleFunctionSignature(
            builder,
            rayDataInfo->type,
            partition->closestHitRequirements,
            descriptorResourcesPointerType),
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(StructuralRayTracingStageKind::ClosestHit)),
    };
    auto closestHitFunctionTableType = builder.getType(
        kIROp_MetalVisibleFunctionTable,
        SLANG_COUNT_OF(closestHitTableTypeOperands),
        closestHitTableTypeOperands);

    if (partition->intersectionFunctionTableType)
    {
        SLANG_RELEASE_ASSERT(
            partition->intersectionFunctionTableType == intersectionFunctionTableType &&
            partition->missFunctionTableType == missFunctionTableType &&
            partition->closestHitFunctionTableType == closestHitFunctionTableType);
    }
    else
    {
        partition->intersectionFunctionTableType = intersectionFunctionTableType;
        partition->missFunctionTableType = missFunctionTableType;
        partition->closestHitFunctionTableType = closestHitFunctionTableType;
        partition->intersectionFunctionsField->setFieldType(intersectionFunctionTableType);
        partition->missFunctionsField->setFieldType(missFunctionTableType);
        partition->closestHitFunctionsField->setFieldType(closestHitFunctionTableType);
    }

    outInfo.descriptorResourcesField = programInfo->descriptorResourcesField;
    outInfo.descriptorResourcesPointerType = descriptorResourcesPointerType;
    outInfo.intersectionFunctionsField = partition->intersectionFunctionsField;
    outInfo.missFunctionsField = partition->missFunctionsField;
    outInfo.closestHitFunctionsField = partition->closestHitFunctionsField;
    outInfo.callableFunctionsField = programInfo->callableFunctionsField;
    outInfo.recordsField = programInfo->recordsField;
    outInfo.intersectionFunctionTableType = intersectionFunctionTableType;
    outInfo.missFunctionTableType = missFunctionTableType;
    outInfo.closestHitFunctionTableType = closestHitFunctionTableType;
    return true;
}

static bool _prepareCallableDescriptor(
    IRBuilder& builder,
    IRType* dataType,
    MetalProgramDescriptorInfo* programInfo,
    const MetalStageRequirements& requirements,
    MetalTraceDescriptorInfo& outInfo)
{
    List<IRType*> parameterTypes;
    parameterTypes.add(builder.getPtrType(dataType, AddressSpace::ThreadLocal));
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    if (requirements.dispatchRaysIndex)
        parameterTypes.add(uint3Type);
    if (requirements.dispatchRaysDimensions)
        parameterTypes.add(uint3Type);
    if (requirements.record)
        parameterTypes.add(builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global));
    parameterTypes.add(builder.getPtrType(builder.getUIntType(), AddressSpace::Uniform));
    parameterTypes.add(builder.getPtrType(builder.getUIntType(), AddressSpace::Global));
    auto signature = builder.getFuncType(
        parameterTypes.getCount(),
        parameterTypes.getBuffer(),
        builder.getVoidType());
    IRInst* callableTableTypeOperands[] = {
        signature,
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(StructuralRayTracingStageKind::Callable)),
    };
    auto callableFunctionTableType = builder.getType(
        kIROp_MetalVisibleFunctionTable,
        SLANG_COUNT_OF(callableTableTypeOperands),
        callableTableTypeOperands);
    if (programInfo->callableFunctionTableType)
    {
        SLANG_RELEASE_ASSERT(programInfo->callableFunctionTableType == callableFunctionTableType);
    }
    else
    {
        programInfo->callableFunctionTableType = callableFunctionTableType;
        programInfo->callableFunctionsField->setFieldType(callableFunctionTableType);
    }

    outInfo.descriptorResourcesField = programInfo->descriptorResourcesField;
    outInfo.descriptorResourcesPointerType = programInfo->descriptorResourcesPointerType;
    outInfo.callableFunctionsField = programInfo->callableFunctionsField;
    outInfo.recordsField = programInfo->recordsField;
    outInfo.callableFunctionTableType = callableFunctionTableType;
    return true;
}

static IRInst* _loadDescriptorResource(
    IRBuilder& builder,
    IRInst* descriptor,
    const MetalTraceDescriptorInfo& descriptorInfo,
    IRStructField* resourceField)
{
    auto resources =
        builder.emitFieldExtract(descriptor, descriptorInfo.descriptorResourcesField->getKey());
    auto resourceAddress = builder.emitFieldAddress(resources, resourceField->getKey());
    return builder.emitLoad(resourceAddress);
}

static IRInst* _getDescriptorResources(
    IRBuilder& builder,
    IRInst* descriptor,
    const MetalTraceDescriptorInfo& descriptorInfo)
{
    return builder.emitFieldExtract(descriptor, descriptorInfo.descriptorResourcesField->getKey());
}

static MetalStructuralRayTracingGeometryKind _getGeometryKind(
    IRInst* schemaOperation,
    IRType* payloadType)
{
    auto result = MetalStructuralRayTracingGeometryKind::Unknown;
    for (auto decoration : schemaOperation->getDecorations())
    {
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (!group || !_isStructuralHitGroupForPayload(group, payloadType))
            continue;

        MetalStructuralRayTracingGeometryKind candidate;
        switch (StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue()))
        {
        case StructuralRayTracingHitAttributesKind::Triangle:
            candidate = MetalStructuralRayTracingGeometryKind::Triangle;
            break;
        case StructuralRayTracingHitAttributesKind::Curve:
            candidate = MetalStructuralRayTracingGeometryKind::Curve;
            break;
        case StructuralRayTracingHitAttributesKind::Custom:
            candidate = MetalStructuralRayTracingGeometryKind::BoundingBox;
            break;
        default:
            return MetalStructuralRayTracingGeometryKind::Unknown;
        }

        if (result == MetalStructuralRayTracingGeometryKind::Unknown)
            result = candidate;
        else if (result != candidate)
            return MetalStructuralRayTracingGeometryKind::Unknown;
    }
    return result;
}

static void _getRayTraversalDescValues(
    IRBuilder& builder,
    IRInst* desc,
    IRInst*& outOrigin,
    IRInst*& outDirection,
    IRInst*& outMinDistance,
    IRInst*& outMaxDistance,
    IRInst*& outTime,
    IRInst*& outRayFlags,
    IRInst*& outInstanceMask,
    IRInst*& outSbtOffset,
    IRInst*& outSbtStride,
    IRInst*& outMissIndex)
{
    auto descType = cast<IRStructType>(desc->getDataType());
    List<IRStructField*> descFields;
    _getStructFields(descType, descFields);
    SLANG_ASSERT(descFields.getCount() == 7);

    auto ray = builder.emitFieldExtract(desc, descFields[0]->getKey());
    auto rayType = cast<IRStructType>(ray->getDataType());
    List<IRStructField*> rayFields;
    _getStructFields(rayType, rayFields);
    SLANG_ASSERT(rayFields.getCount() == 4);

    outOrigin = builder.emitFieldExtract(ray, rayFields[0]->getKey());
    outMinDistance = builder.emitFieldExtract(ray, rayFields[1]->getKey());
    outDirection = builder.emitFieldExtract(ray, rayFields[2]->getKey());
    outMaxDistance = builder.emitFieldExtract(ray, rayFields[3]->getKey());
    outTime = builder.emitFieldExtract(desc, descFields[1]->getKey());
    outRayFlags = builder.emitFieldExtract(desc, descFields[2]->getKey());
    outInstanceMask = builder.emitFieldExtract(desc, descFields[3]->getKey());
    outSbtOffset = builder.emitFieldExtract(desc, descFields[4]->getKey());
    outSbtStride = builder.emitFieldExtract(desc, descFields[5]->getKey());
    outMissIndex = builder.emitFieldExtract(desc, descFields[6]->getKey());
}

// Materializes one schema payload partition and, for the payload used by an actual trace, lowers
// that trace operation as well. Keeping both paths in this routine guarantees that an eagerly
// emitted table has exactly the signature and adapter set that a later trace through it consumes.
static bool _materializeMetalPayloadPartition(
    IRModule* module,
    IRInst* schemaOperation,
    IRType* payloadType,
    bool lowerTrace,
    MetalProgramDescriptorInfo* programInfo,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generatedMissAdapters,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generatedClosestHitAdapters,
    Dictionary<KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*>, IRFunc*>&
        generatedCandidateHelpers,
    Dictionary<MetalCandidateDispatcherKey, IRFunc*>& generatedCandidateDispatchers,
    HashSet<IRFunc*>& candidateAdapterSet,
    HashSet<IRFunc*>& candidateHelperSet,
    Dictionary<IRFunc*, IRInst*>& payloadValues,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues,
    Dictionary<IRFunc*, IRParam*>& candidateRayDataParams,
    MetalRayDataInfo* rayDataInfo,
    const MetalCandidateResultInfo& filterResultInfo,
    const MetalCandidateResultInfo& proceduralResultInfo)
{
    auto trace = as<IRStructuralRayTracingTrace>(schemaOperation);
    SLANG_RELEASE_ASSERT(!lowerTrace || trace);
    IRBuilder builder(module);
    auto payloadPartition = programInfo->findPayloadPartition(payloadType);
    SLANG_RELEASE_ASSERT(payloadPartition);
    auto tagMask = payloadPartition->tagMask;
    auto maxLevels = payloadPartition->maxLevels;
    const auto& missRequirements = payloadPartition->missRequirements;
    const auto& closestHitRequirements = payloadPartition->closestHitRequirements;
    MetalTraceDescriptorInfo descriptorInfo;
    if (!_prepareTraceDescriptor(
            builder,
            programInfo,
            payloadPartition,
            rayDataInfo,
            descriptorInfo))
        return false;

    bool hasMissFunctions = false;
    bool hasClosestHitFunctions = false;
    bool hasIntersectionFunctions = false;
    List<IRStructuralRayTracingHitGroupInfoDecoration*> triangleCandidateGroups;
    List<IRStructuralRayTracingHitGroupInfoDecoration*> curveCandidateGroups;
    List<IRStructuralRayTracingHitGroupInfoDecoration*> boundingBoxCandidateGroups;
    MetalStageRequirements triangleCandidateRequirements;
    MetalStageRequirements curveCandidateRequirements;
    MetalStageRequirements boundingBoxCandidateRequirements;
    bool boundingBoxHasAnyHit = false;
    for (auto decoration : schemaOperation->getDecorations())
    {
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (!group || !_isStructuralHitGroupForPayload(group, payloadType))
            continue;
        auto closestHitInvoke = getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::ClosestHit);
        auto anyHitInvoke = getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::AnyHit);
        auto intersectionInvoke = getStructuralRayTracingHitGroupStageInvoke(
            group,
            StructuralRayTracingStageKind::Intersection);
        hasClosestHitFunctions |= closestHitInvoke != nullptr;
        auto attributesKind =
            StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue());
        if (attributesKind == StructuralRayTracingHitAttributesKind::Triangle)
        {
            if (!anyHitInvoke)
                continue;
            triangleCandidateGroups.add(group);
            triangleCandidateRequirements = _combineMetalStageRequirements(
                triangleCandidateRequirements,
                _getMetalStageRequirements(anyHitInvoke));
        }
        else if (attributesKind == StructuralRayTracingHitAttributesKind::Curve)
        {
            if (!anyHitInvoke)
                continue;
            curveCandidateGroups.add(group);
            curveCandidateRequirements = _combineMetalStageRequirements(
                curveCandidateRequirements,
                _getMetalStageRequirements(anyHitInvoke));
        }
        else if (attributesKind == StructuralRayTracingHitAttributesKind::Custom)
        {
            if (!intersectionInvoke)
                continue;
            boundingBoxCandidateGroups.add(group);
            boundingBoxCandidateRequirements = _combineMetalStageRequirements(
                boundingBoxCandidateRequirements,
                _combineMetalStageRequirements(
                    _getMetalStageRequirements(intersectionInvoke),
                    _getMetalStageRequirements(anyHitInvoke)));
            boundingBoxHasAnyHit |= anyHitInvoke != nullptr;
        }
    }
    const bool candidateUsesInstancing =
        (tagMask & UInt(MetalStructuralRayTracingTag::Instancing)) != 0;
    auto prepareCandidateSignature = [&](MetalStageRequirements& requirements)
    {
        requirements.record = true;
        requirements.geometryIndex = true;
        requirements.instanceIndex |= candidateUsesInstancing;
    };
    prepareCandidateSignature(triangleCandidateRequirements);
    prepareCandidateSignature(curveCandidateRequirements);
    prepareCandidateSignature(boundingBoxCandidateRequirements);

    for (auto decoration : schemaOperation->getDecorations())
    {
        if (auto entry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
        {
            if (!_isStructuralMissShaderForPayload(entry, payloadType))
                continue;
            auto physicalName = getStructuralRayTracingMetalMissFunctionName(
                programInfo->programLayoutSourceTypeName->getStringSlice(),
                payloadPartition->payloadIndex,
                Index(entry->getFunctionIndex()->getValue()),
                entry->getMissSourceTypeName()->getStringSlice());
            if (_generateVisibleStageAdapter(
                    module,
                    generatedMissAdapters,
                    payloadValues,
                    dispatchValues,
                    entry->getMissType(),
                    StructuralRayTracingStageKind::Miss,
                    entry->getMissType(),
                    entry->getMissSourceTypeName(),
                    entry->getMissTypeIdentity(),
                    entry->getMiss(),
                    entry->getContextType(),
                    entry->getPayloadType(),
                    entry->getRecordType(),
                    nullptr,
                    StructuralRayTracingHitAttributesKind::None,
                    physicalName.getUnownedSlice(),
                    missRequirements,
                    rayDataInfo,
                    descriptorInfo.descriptorResourcesPointerType,
                    cast<IRMetalVisibleFunctionTable>(descriptorInfo.missFunctionTableType)))
            {
                hasMissFunctions = true;
            }
        }
        else if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
        {
            if (!_isStructuralHitGroupForPayload(group, payloadType))
                continue;
            // No table is required when every group uses `NoClosestHit`. Once any real stage
            // enables the dense table, however, every group must contribute a physical entry.
            if (!hasClosestHitFunctions)
                continue;
            auto hitAttributesKind =
                StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue());
            auto closestHitInvoke = getStructuralRayTracingHitGroupStageInvoke(
                group,
                StructuralRayTracingStageKind::ClosestHit);
            const bool hasSourceClosestHit = closestHitInvoke != nullptr;
            auto physicalName =
                hasSourceClosestHit
                    ? getStructuralRayTracingMetalClosestHitFunctionName(
                          programInfo->programLayoutSourceTypeName->getStringSlice(),
                          payloadPartition->payloadIndex,
                          Index(group->getFunctionIndex()->getValue()),
                          group->getGroupSourceTypeName()->getStringSlice(),
                          group->getClosestHitSourceTypeName()->getStringSlice())
                    : getStructuralRayTracingMetalNoOpClosestHitFunctionName(
                          programInfo->programLayoutSourceTypeName->getStringSlice(),
                          payloadPartition->payloadIndex,
                          Index(group->getFunctionIndex()->getValue()),
                          group->getGroupSourceTypeName()->getStringSlice());
            if (_generateVisibleStageAdapter(
                    module,
                    generatedClosestHitAdapters,
                    payloadValues,
                    dispatchValues,
                    group->getGroupType(),
                    StructuralRayTracingStageKind::ClosestHit,
                    group->getClosestHitType(),
                    group->getClosestHitSourceTypeName(),
                    group->getClosestHitTypeIdentity(),
                    closestHitInvoke,
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getRecordType(),
                    group->getHitAttributesType(),
                    hitAttributesKind,
                    physicalName.getUnownedSlice(),
                    closestHitRequirements,
                    rayDataInfo,
                    descriptorInfo.descriptorResourcesPointerType,
                    cast<IRMetalVisibleFunctionTable>(descriptorInfo.closestHitFunctionTableType)))
            {
                hasClosestHitFunctions = true;
            }
        }
    }

    auto generateBuiltInDispatcher =
        [&](const List<IRStructuralRayTracingHitGroupInfoDecoration*>& groups,
            const MetalStageRequirements& signatureRequirements,
            MetalStructuralRayTracingGeometryKind geometryKind)
    {
        List<MetalCandidateDispatcherArm> arms;
        for (auto group : groups)
        {
            if (auto helper = _generateBuiltInAnyHitCandidateAdapter(
                    module,
                    generatedCandidateHelpers,
                    payloadValues,
                    dispatchValues,
                    filterResultInfo,
                    group,
                    signatureRequirements,
                    tagMask,
                    rayDataInfo))
            {
                arms.add({group, helper});
                candidateHelperSet.add(helper);
            }
        }
        auto physicalName = getStructuralRayTracingMetalCandidateDispatcherName(
            programInfo->programLayoutSourceTypeName->getStringSlice(),
            payloadPartition->payloadIndex,
            _getStructuralMetalCandidateKind(geometryKind));
        if (auto dispatcher = _generateMetalCandidateDispatcher(
                module,
                generatedCandidateDispatchers,
                candidateRayDataParams,
                filterResultInfo,
                arms,
                geometryKind,
                tagMask,
                maxLevels,
                programInfo->hitRecordStride,
                rayDataInfo,
                programInfo->programLayout,
                physicalName.getUnownedSlice()))
        {
            hasIntersectionFunctions = true;
            candidateAdapterSet.add(dispatcher);
        }
    };
    generateBuiltInDispatcher(
        triangleCandidateGroups,
        triangleCandidateRequirements,
        MetalStructuralRayTracingGeometryKind::Triangle);
    generateBuiltInDispatcher(
        curveCandidateGroups,
        curveCandidateRequirements,
        MetalStructuralRayTracingGeometryKind::Curve);

    List<MetalCandidateDispatcherArm> boundingBoxArms;
    for (auto group : boundingBoxCandidateGroups)
    {
        if (auto helper = _generateBoundingBoxCandidateAdapter(
                module,
                generatedCandidateHelpers,
                candidateHelperSet,
                payloadValues,
                dispatchValues,
                filterResultInfo,
                proceduralResultInfo,
                group,
                boundingBoxCandidateRequirements,
                boundingBoxHasAnyHit,
                tagMask,
                rayDataInfo))
        {
            boundingBoxArms.add({group, helper});
            candidateHelperSet.add(helper);
        }
    }
    auto boundingBoxPhysicalName = getStructuralRayTracingMetalCandidateDispatcherName(
        programInfo->programLayoutSourceTypeName->getStringSlice(),
        payloadPartition->payloadIndex,
        StructuralRayTracingMetalCandidateKind::BoundingBox);
    if (auto dispatcher = _generateMetalCandidateDispatcher(
            module,
            generatedCandidateDispatchers,
            candidateRayDataParams,
            proceduralResultInfo,
            boundingBoxArms,
            MetalStructuralRayTracingGeometryKind::BoundingBox,
            tagMask,
            maxLevels,
            programInfo->hitRecordStride,
            rayDataInfo,
            programInfo->programLayout,
            boundingBoxPhysicalName.getUnownedSlice()))
    {
        hasIntersectionFunctions = true;
        candidateAdapterSet.add(dispatcher);
    }

    if (!lowerTrace)
        return true;

    SLANG_RELEASE_ASSERT(trace->getPayloadType() == payloadType);

    builder.setInsertBefore(trace);
    auto intersectionFunctions = _loadDescriptorResource(
        builder,
        trace->getDescriptor(),
        descriptorInfo,
        descriptorInfo.intersectionFunctionsField);
    auto missFunctions = _loadDescriptorResource(
        builder,
        trace->getDescriptor(),
        descriptorInfo,
        descriptorInfo.missFunctionsField);
    auto closestHitFunctions = _loadDescriptorResource(
        builder,
        trace->getDescriptor(),
        descriptorInfo,
        descriptorInfo.closestHitFunctionsField);
    auto records = _loadDescriptorResource(
        builder,
        trace->getDescriptor(),
        descriptorInfo,
        descriptorInfo.recordsField);
    auto descriptorResources =
        _getDescriptorResources(builder, trace->getDescriptor(), descriptorInfo);

    IRInst* origin;
    IRInst* direction;
    IRInst* minDistance;
    IRInst* maxDistance;
    IRInst* time;
    IRInst* rayFlags;
    IRInst* instanceMask;
    IRInst* sbtOffset;
    IRInst* sbtStride;
    IRInst* missIndex;
    _getRayTraversalDescValues(
        builder,
        trace->getDesc(),
        origin,
        direction,
        minDistance,
        maxDistance,
        time,
        rayFlags,
        instanceMask,
        sbtOffset,
        sbtStride,
        missIndex);

    auto rayData = builder.emitVar(rayDataInfo->type);
    builder.addNameHintDecoration(rayData, UnownedTerminatedStringSlice("rayData"));
    auto rayDataPayload = builder.emitFieldAddress(rayData, rayDataInfo->payloadKey);
    builder.emitStore(rayDataPayload, builder.emitLoad(trace->getPayload()));
    if (rayDataInfo->recordDataKey)
    {
        builder.emitStore(builder.emitFieldAddress(rayData, rayDataInfo->recordDataKey), records);
    }
    if (rayDataInfo->sbtOffsetKey)
    {
        SLANG_ASSERT(rayDataInfo->sbtStrideKey);
        builder.emitStore(builder.emitFieldAddress(rayData, rayDataInfo->sbtOffsetKey), sbtOffset);
        builder.emitStore(builder.emitFieldAddress(rayData, rayDataInfo->sbtStrideKey), sbtStride);
    }
    if (rayDataInfo->minDistanceKey)
    {
        builder.emitStore(
            builder.emitFieldAddress(rayData, rayDataInfo->minDistanceKey),
            minDistance);
    }
    if (rayDataInfo->rayFlagsKey)
    {
        builder.emitStore(builder.emitFieldAddress(rayData, rayDataInfo->rayFlagsKey), rayFlags);
    }
    if (rayDataInfo->rayTimeKey)
    {
        builder.emitStore(builder.emitFieldAddress(rayData, rayDataInfo->rayTimeKey), time);
    }
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    if (rayDataInfo->dispatchRaysIndexKey)
    {
        auto dispatchRaysIndex = builder.emitIntrinsicInst(
            uint3Type,
            kIROp_MetalStructuralRayTracingDispatchRaysIndex,
            0,
            nullptr);
        builder.emitStore(
            builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysIndexKey),
            dispatchRaysIndex);
    }
    if (rayDataInfo->dispatchRaysDimensionsKey)
    {
        auto dispatchRaysDimensions = builder.emitIntrinsicInst(
            uint3Type,
            kIROp_MetalStructuralRayTracingDispatchRaysDimensions,
            0,
            nullptr);
        builder.emitStore(
            builder.emitFieldAddress(rayData, rayDataInfo->dispatchRaysDimensionsKey),
            dispatchRaysDimensions);
    }

    auto intType = builder.getIntType();
    IRInst* operands[] = {
        builder.getIntValue(intType, IRIntegerValue(tagMask)),
        builder.getIntValue(intType, maxLevels),
        builder.getIntValue(
            intType,
            IRIntegerValue(_getMetalStageRequirementMask(missRequirements))),
        builder.getIntValue(
            intType,
            IRIntegerValue(_getMetalStageRequirementMask(closestHitRequirements))),
        builder.getIntValue(
            intType,
            IRIntegerValue(_getGeometryKind(schemaOperation, payloadType))),
        builder.getBoolValue(hasIntersectionFunctions),
        builder.getBoolValue(hasMissFunctions),
        builder.getBoolValue(hasClosestHitFunctions),
        origin,
        direction,
        minDistance,
        maxDistance,
        time,
        rayFlags,
        instanceMask,
        sbtOffset,
        sbtStride,
        missIndex,
        trace->getAccelerationStructure(),
        intersectionFunctions,
        missFunctions,
        closestHitFunctions,
        descriptorResources,
        records,
        builder.getIntValue(intType, programInfo->hitRecordStride),
        builder.getIntValue(intType, programInfo->missRecordStride),
        rayData,
        builder.getBoolValue(false),
        builder.getBoolValue(false),
        builder.emitDefaultConstruct(
            builder.getPtrType(builder.getUIntType(), AddressSpace::ThreadLocal)),
    };
    builder.emitIntrinsicInst(
        builder.getVoidType(),
        kIROp_MetalStructuralRayTracingTrace,
        SLANG_COUNT_OF(operands),
        operands);
    builder.emitStore(trace->getPayload(), builder.emitLoad(rayDataPayload));
    trace->removeAndDeallocate();
    return true;
}

// Emits every callable adapter declared by an activated schema, regardless of which callable
// index a reachable call happens to select. Reflection exposes the complete callable table, so a
// host must never receive an entry that has no corresponding Metal symbol.
static bool _materializeMetalCallableTable(
    IRModule* module,
    IRInst* schemaOperation,
    MetalProgramDescriptorInfo* programInfo,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generatedCallableAdapters,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues)
{
    if (!programInfo->callableDataType)
        return true;

    IRBuilder builder(module);
    MetalTraceDescriptorInfo descriptorInfo;
    if (!_prepareCallableDescriptor(
            builder,
            programInfo->callableDataType,
            programInfo,
            programInfo->callableRequirements,
            descriptorInfo))
        return false;

    for (auto decoration : schemaOperation->getDecorations())
    {
        auto entry = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
        if (!entry)
            continue;
        SLANG_RELEASE_ASSERT(entry->getCallableDataType() == programInfo->callableDataType);
        auto physicalName = getStructuralRayTracingMetalCallableFunctionName(
            programInfo->programLayoutSourceTypeName->getStringSlice(),
            Index(entry->getFunctionIndex()->getValue()),
            entry->getCallableSourceTypeName()->getStringSlice());
        _generateCallableStageAdapter(
            module,
            generatedCallableAdapters,
            dispatchValues,
            entry,
            programInfo->programLayout,
            physicalName.getUnownedSlice(),
            descriptorInfo.descriptorResourcesPointerType,
            cast<IRMetalVisibleFunctionTable>(descriptorInfo.callableFunctionTableType),
            programInfo->callableRequirements);
    }
    return true;
}

static bool _lowerCallableDispatch(
    IRModule* module,
    IRStructuralRayTracingCallShader* callOperation,
    MetalProgramDescriptorInfo* programInfo,
    DiagnosticSink* sink)
{
    IRBuilder builder(module);
    SLANG_UNUSED(sink);
    SLANG_RELEASE_ASSERT(
        programInfo->callableDataType &&
        programInfo->callableDataType == callOperation->getCallableDataType() &&
        programInfo->areCallableEntriesMaterialized);

    MetalTraceDescriptorInfo descriptorInfo;
    if (!_prepareCallableDescriptor(
            builder,
            programInfo->callableDataType,
            programInfo,
            programInfo->callableRequirements,
            descriptorInfo))
        return false;

    builder.setInsertBefore(callOperation);
    auto descriptorResources =
        _getDescriptorResources(builder, callOperation->getDescriptor(), descriptorInfo);
    auto records = _loadDescriptorResource(
        builder,
        callOperation->getDescriptor(),
        descriptorInfo,
        descriptorInfo.recordsField);
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    auto zeroDispatchValue = builder.emitDefaultConstruct(uint3Type);
    auto dispatchRaysIndex = programInfo->callableRequirements.dispatchRaysIndex
                                 ? builder.emitIntrinsicInst(
                                       uint3Type,
                                       kIROp_MetalStructuralRayTracingDispatchRaysIndex,
                                       0,
                                       nullptr)
                                 : zeroDispatchValue;
    auto dispatchRaysDimensions = programInfo->callableRequirements.dispatchRaysDimensions
                                      ? builder.emitIntrinsicInst(
                                            uint3Type,
                                            kIROp_MetalStructuralRayTracingDispatchRaysDimensions,
                                            0,
                                            nullptr)
                                      : zeroDispatchValue;
    IRInst* operands[] = {
        callOperation->getCallableIndex(),
        callOperation->getData(),
        dispatchRaysIndex,
        dispatchRaysDimensions,
        builder.getBoolValue(programInfo->callableRequirements.dispatchRaysIndex),
        builder.getBoolValue(programInfo->callableRequirements.dispatchRaysDimensions),
        descriptorResources,
        records,
        builder.getIntValue(builder.getIntType(), programInfo->callableRecordStride),
        builder.getBoolValue(programInfo->callableRequirements.record),
        descriptorResources->getDataType(),
        descriptorInfo.callableFunctionsField,
        builder.getBoolValue(false),
        builder.emitDefaultConstruct(
            builder.getPtrType(builder.getUIntType(), AddressSpace::ThreadLocal)),
    };
    builder.emitIntrinsicInst(
        builder.getVoidType(),
        kIROp_MetalStructuralRayTracingCallShader,
        SLANG_COUNT_OF(operands),
        operands);
    callOperation->removeAndDeallocate();
    return true;
}

static void _makeStructuralRayGenerationEntryPointPhysicalCompute(
    IRBuilder& builder,
    IRFunc* entryPoint)
{
    auto decoration = entryPoint->findDecoration<IREntryPointDecoration>();
    if (!decoration || decoration->getProfile().getStage() != Stage::RayGeneration)
        return;

    decoration->setOperand(
        0,
        builder.getIntValue(builder.getIntType(), Profile(Stage::Compute).raw));
}

struct MetalRayGenerationSystemValueThreader
{
    MetalRayGenerationSystemValueThreader(
        IRModule* module,
        IRType* type,
        const char* parameterName,
        const char* metalSystemValue)
        : module(module)
        , type(type)
        , parameterName(parameterName)
        , metalSystemValue(metalSystemValue)
    {
    }

    IRFunc* findEnclosingFunc(IRInst* inst)
    {
        for (auto parent = inst; parent; parent = parent->getParent())
        {
            if (auto func = as<IRFunc>(parent))
                return func;
        }
        return nullptr;
    }

    IRInst* findOrCreateParameter(IRInst* inst)
    {
        auto func = findEnclosingFunc(inst);
        SLANG_ASSERT(func);
        return findOrCreateParameter(func);
    }

    IRInst* findOrCreateParameter(IRFunc* func)
    {
        if (auto found = parameters.tryGetValue(func))
            return *found;

        auto firstBlock = func->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        IRBuilder builder(module);
        auto parameter = builder.createParam(type);
        builder.addNameHintDecoration(parameter, UnownedTerminatedStringSlice(parameterName));
        parameter->insertBefore(firstBlock->getFirstOrdinaryInst());
        parameters.add(func, parameter);

        if (func->findDecoration<IREntryPointDecoration>())
        {
            builder.addTargetSystemValueDecoration(
                parameter,
                UnownedTerminatedStringSlice(metalSystemValue));
        }

        fixUpFuncType(func);

        List<IRCall*> callUses;
        for (auto use = func->firstUse; use; use = use->nextUse)
        {
            if (auto call = as<IRCall>(use->getUser()))
            {
                if (call->getCallee() == func)
                    callUses.add(call);
            }
        }

        for (auto call : callUses)
        {
            List<IRInst*> args;
            for (UInt i = 0; i < call->getArgCount(); ++i)
                args.add(call->getArg(i));
            args.add(findOrCreateParameter(call));

            builder.setInsertBefore(call);
            auto newCall = builder.emitCallInst(
                call->getDataType(),
                call->getCallee(),
                args.getCount(),
                args.getBuffer());
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }
        return parameter;
    }

    void registerValue(IRFunc* func, IRInst* value)
    {
        if (value)
            parameters[func] = value;
    }

    void lower(IRInst* operation)
    {
        operation->replaceUsesWith(findOrCreateParameter(operation));
        operation->removeAndDeallocate();
    }

    IRModule* module;
    IRType* type;
    const char* parameterName;
    const char* metalSystemValue;
    Dictionary<IRFunc*, IRInst*> parameters;
};

static void _collectMetalRayGenerationSystemValueOperations(
    IRInst* parent,
    List<IRInst*>& dispatchIndexOperations,
    List<IRInst*>& dispatchDimensionsOperations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectMetalRayGenerationSystemValueOperations(
            child,
            dispatchIndexOperations,
            dispatchDimensionsOperations);
        if (child->getOp() == kIROp_MetalStructuralRayTracingDispatchRaysIndex)
        {
            dispatchIndexOperations.add(child);
            continue;
        }
        if (child->getOp() == kIROp_MetalStructuralRayTracingDispatchRaysDimensions)
        {
            dispatchDimensionsOperations.add(child);
            continue;
        }
    }
}

static bool _isCalledFromMetalDispatchRoot(
    IRFunc* function,
    const HashSet<IRFunc*>& roots,
    HashSet<IRFunc*>& visited)
{
    if (roots.contains(function))
        return true;
    if (!visited.add(function))
        return false;

    for (auto use = function->firstUse; use; use = use->nextUse)
    {
        auto call = as<IRCall>(use->getUser());
        if (!call || call->getCallee() != function)
            continue;
        auto caller = _findEnclosingFunc(call);
        if (caller && _isCalledFromMetalDispatchRoot(caller, roots, visited))
            return true;
    }
    return false;
}

static void _lowerMetalRayGenerationSystemValues(
    IRModule* module,
    const Dictionary<IRInst*, HashSet<IRFunc*>>& referencingEntryPoints,
    const HashSet<IRFunc*>& physicalRayGenerationEntryPoints,
    const Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues)
{
    List<IRInst*> dispatchIndexOperations;
    List<IRInst*> dispatchDimensionsOperations;
    _collectMetalRayGenerationSystemValueOperations(
        module->getModuleInst(),
        dispatchIndexOperations,
        dispatchDimensionsOperations);

    IRBuilder builder(module);
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    MetalRayGenerationSystemValueThreader dispatchIndexThreader(
        module,
        uint3Type,
        "dispatchRaysIndex",
        "thread_position_in_grid");
    MetalRayGenerationSystemValueThreader dispatchDimensionsThreader(
        module,
        uint3Type,
        "dispatchRaysDimensions",
        "threads_per_grid");

    HashSet<IRFunc*> roots = physicalRayGenerationEntryPoints;
    for (const auto& [func, values] : dispatchValues)
    {
        roots.add(func);
        dispatchIndexThreader.registerValue(func, values.index);
        dispatchDimensionsThreader.registerValue(func, values.dimensions);
    }

    auto isUsedByStructuralRayTracing = [&](IRInst* operation)
    {
        auto func = dispatchIndexThreader.findEnclosingFunc(operation);
        if (!func)
            return false;
        HashSet<IRFunc*> visited;
        if (_isCalledFromMetalDispatchRoot(func, roots, visited))
            return true;
        auto referencing = func ? referencingEntryPoints.tryGetValue(func) : nullptr;
        if (!referencing)
            return false;
        for (auto entryPoint : *referencing)
        {
            if (physicalRayGenerationEntryPoints.contains(entryPoint))
                return true;
        }
        return false;
    };

    for (auto operation : dispatchIndexOperations)
    {
        if (isUsedByStructuralRayTracing(operation))
            dispatchIndexThreader.lower(operation);
    }
    for (auto operation : dispatchDimensionsOperations)
    {
        if (isUsedByStructuralRayTracing(operation))
            dispatchDimensionsThreader.lower(operation);
    }
}

static MetalProgramDescriptorInfo* _findOrAddMetalProgramDescriptorInfo(
    IRInst* operation,
    Dictionary<IRInst*, RefPtr<MetalProgramDescriptorInfo>>& infosByProgramLayout,
    List<RefPtr<MetalProgramDescriptorInfo>>& orderedInfos)
{
    IRInst* programLayout = nullptr;
    IRStringLit* programLayoutSourceTypeName = nullptr;
    IRInst* descriptor = nullptr;
    if (auto trace = as<IRStructuralRayTracingTrace>(operation))
    {
        programLayout = trace->getProgramLayout();
        programLayoutSourceTypeName = trace->getProgramLayoutSourceTypeName();
        descriptor = trace->getDescriptor();
    }
    else
    {
        auto call = cast<IRStructuralRayTracingCallShader>(operation);
        programLayout = call->getProgramLayout();
        programLayoutSourceTypeName = call->getProgramLayoutSourceTypeName();
        descriptor = call->getDescriptor();
    }

    if (auto found = infosByProgramLayout.tryGetValue(programLayout))
    {
        // One schema has one specialized `TraceProgramDescriptor<Schema>` representation. If two
        // operations disagree here, specialization produced a malformed semantic shape; silently
        // synthesizing two physical SBT descriptors would hide that producer defect.
        SLANG_RELEASE_ASSERT((*found)->descriptor->getDataType() == descriptor->getDataType());
        SLANG_RELEASE_ASSERT(
            (*found)->programLayoutSourceTypeName->getStringSlice() ==
            programLayoutSourceTypeName->getStringSlice());
        (*found)->addDescriptorValue(descriptor);
        return found->Ptr();
    }

    RefPtr<MetalProgramDescriptorInfo> info = new MetalProgramDescriptorInfo();
    info->programLayout = programLayout;
    info->programLayoutSourceTypeName = programLayoutSourceTypeName;
    info->descriptor = descriptor;
    info->addDescriptorValue(descriptor);
    infosByProgramLayout.add(programLayout, info);
    orderedInfos.add(info);
    return info;
}

// Record payload partitions in the same deterministic order exposed by schema reflection: first
// appearance in `HitGroups`, followed by first appearance in `MissShaders`.
//
// IR decorations are deliberately inserted at the front of their owner, so iterating them directly
// reverses source-list order. Collect each role and visit it backwards to recover the list order
// encoded by AST-to-IR lowering.
static void _collectMetalPayloadPartitions(
    IRInst* schemaOperation,
    MetalProgramDescriptorInfo* info)
{
    List<IRStructuralRayTracingHitGroupInfoDecoration*> hitGroups;
    List<IRStructuralRayTracingMissShaderInfoDecoration*> missShaders;
    for (auto decoration : schemaOperation->getDecorations())
    {
        if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            hitGroups.add(group);
        else if (auto entry = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            missShaders.add(entry);
    }
    for (Index i = hitGroups.getCount(); i > 0; --i)
        info->addPayloadType(cast<IRType>(hitGroups[i - 1]->getPayloadType()));
    for (Index i = missShaders.getCount(); i > 0; --i)
        info->addPayloadType(cast<IRType>(missShaders[i - 1]->getPayloadType()));
}

// Collects the ABI shared by the schema's single callable table. Schema validation has already
// rejected heterogeneous CallableData types, so every operation carrying this schema must agree
// with the first linked callable entry.
static void _collectMetalCallableTableInfo(IRInst* operation, MetalProgramDescriptorInfo* info)
{
    for (auto decoration : operation->getDecorations())
    {
        auto entry = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration);
        if (!entry)
            continue;
        auto dataType = cast<IRType>(entry->getCallableDataType());
        if (info->callableDataType)
            SLANG_RELEASE_ASSERT(info->callableDataType == dataType);
        else
            info->callableDataType = dataType;
        info->callableRequirements = _combineMetalStageRequirements(
            info->callableRequirements,
            _getMetalStageRequirements(entry->getCallable()));
    }
}

// Returns the fixed byte stride for records whose largest application-data type is `recordType`.
// The first 16 bytes are reserved for the host-written function index and future ABI metadata;
// rounding the whole record to 16 bytes keeps every following data payload naturally aligned.
static IRIntegerValue _getMetalRecordStride(TargetRequest* targetRequest, IRType* recordType)
{
    IRSizeAndAlignment sizeAndAlignment;
    SLANG_RELEASE_ASSERT(
        SLANG_SUCCEEDED(getNaturalSizeAndAlignment(targetRequest, recordType, &sizeAndAlignment)) &&
        sizeAndAlignment.size != IRSizeAndAlignment::kIndeterminateSize);
    return IRIntegerValue(getStructuralRayTracingMetalRecordStride(UInt64(sizeAndAlignment.size)));
}

static void _calculateMetalRecordStrides(
    MetalProgramDescriptorInfo* info,
    TargetRequest* targetRequest)
{
    for (auto operation : info->operations)
    {
        for (auto decoration : operation->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                info->hitRecordStride = Math::Max(
                    info->hitRecordStride,
                    _getMetalRecordStride(targetRequest, cast<IRType>(group->getRecordType())));
                continue;
            }
            if (auto miss = as<IRStructuralRayTracingMissShaderInfoDecoration>(decoration))
            {
                info->missRecordStride = Math::Max(
                    info->missRecordStride,
                    _getMetalRecordStride(targetRequest, cast<IRType>(miss->getRecordType())));
                continue;
            }
            if (auto callable = as<IRStructuralRayTracingCallableShaderInfoDecoration>(decoration))
            {
                info->callableRecordStride = Math::Max(
                    info->callableRecordStride,
                    _getMetalRecordStride(targetRequest, cast<IRType>(callable->getRecordType())));
            }
        }
    }
}

// Converts Slang's compiler-internal tag bits to Apple's public
// `MTLIntersectionFunctionSignature` bit positions. The two enums intentionally have different
// layouts, so a cast here would make reflected opaque-function signatures silently incorrect.
static UInt _getMetalIntersectionFunctionSignature(UInt tagMask, IRIntegerValue maxLevels)
{
    UInt result = UInt(slang::MetalIntersectionFunctionSignature::None);
#define SLANG_ADD_METAL_INTERSECTION_SIGNATURE(INTERNAL_TAG, PUBLIC_TAG)       \
    if ((tagMask & UInt(MetalStructuralRayTracingTag::INTERNAL_TAG)) != 0)     \
    {                                                                          \
        result |= UInt(slang::MetalIntersectionFunctionSignature::PUBLIC_TAG); \
    }
    SLANG_ADD_METAL_INTERSECTION_SIGNATURE(Instancing, Instancing)
    SLANG_ADD_METAL_INTERSECTION_SIGNATURE(TriangleData, TriangleData)
    SLANG_ADD_METAL_INTERSECTION_SIGNATURE(WorldSpaceData, WorldSpaceData)
    SLANG_ADD_METAL_INTERSECTION_SIGNATURE(InstanceMotion, InstanceMotion)
    SLANG_ADD_METAL_INTERSECTION_SIGNATURE(PrimitiveMotion, PrimitiveMotion)
    SLANG_ADD_METAL_INTERSECTION_SIGNATURE(ExtendedLimits, ExtendedLimits)
    SLANG_ADD_METAL_INTERSECTION_SIGNATURE(CurveData, CurveData)
#undef SLANG_ADD_METAL_INTERSECTION_SIGNATURE
    if (maxLevels > 0)
        result |= UInt(slang::MetalIntersectionFunctionSignature::MaxLevels);
    return result;
}

// Unions every trace's target requirements into its schema payload partition before any physical
// table type is created. Consider two entry points that use the same schema and payload but appear
// in the opposite link order. Both must produce one identical IFT type and one identical metadata
// record; allowing the first operation to freeze the type would make code generation
// order-sensitive.
static void _collectMetalPayloadPartitionRequirements(
    MetalProgramDescriptorInfo* info,
    const Dictionary<IRInst*, MetalTraceContextRequirements>& traceContextRequirements,
    UInt capabilityTagMask)
{
    for (auto operation : info->operations)
    {
        auto traceRequirements = traceContextRequirements.tryGetValue(operation);
        SLANG_RELEASE_ASSERT(traceRequirements);
        for (auto& partition : info->payloadPartitions)
        {
            partition.tagMask |= _getSharedMetalTagMask(
                operation,
                partition.payloadType,
                traceRequirements->tagMask,
                capabilityTagMask);
            partition.maxLevels = Math::Max(partition.maxLevels, traceRequirements->maxLevels);
            partition.missRequirements = _combineMetalStageRequirements(
                partition.missRequirements,
                _getMetalStageRequirements(
                    operation,
                    StructuralRayTracingStageKind::Miss,
                    partition.payloadType));
            partition.closestHitRequirements = _combineMetalStageRequirements(
                partition.closestHitRequirements,
                _getMetalStageRequirements(
                    operation,
                    StructuralRayTracingStageKind::ClosestHit,
                    partition.payloadType));
        }
    }
}

// Records the finalized IFT signature on the schema's nominal physical descriptor. The generic
// post-emit metadata collector reads this decoration after all target lowering; it does not know
// how to infer Metal tags itself.
static void _addMetalPayloadMetadataDecorations(IRModule* module, MetalProgramDescriptorInfo* info)
{
    IRBuilder builder(module);
    for (auto& partition : info->payloadPartitions)
    {
        IRInst* operands[] = {
            info->programLayoutSourceTypeName,
            builder.getIntValue(builder.getIntType(), partition.payloadIndex),
            builder.getIntValue(
                builder.getIntType(),
                IRIntegerValue(_getMetalIntersectionFunctionSignature(
                    partition.tagMask,
                    partition.maxLevels))),
        };
        builder.addDecoration(
            info->physicalDescriptorType,
            kIROp_StructuralRayTracingMetalPayloadMetadataDecoration,
            operands,
            SLANG_COUNT_OF(operands));
    }
}

static bool _prepareMetalProgramDescriptors(
    IRModule* module,
    const List<IRInst*>& operations,
    TargetRequest* targetRequest,
    const Dictionary<IRInst*, MetalTraceContextRequirements>& traceContextRequirements,
    UInt capabilityTagMask,
    Dictionary<IRInst*, RefPtr<MetalProgramDescriptorInfo>>& outInfos)
{
    List<RefPtr<MetalProgramDescriptorInfo>> orderedInfos;
    Dictionary<IRInst*, IRType*> physicalTypesByValue;
    for (auto operation : operations)
    {
        auto info = _findOrAddMetalProgramDescriptorInfo(operation, outInfos, orderedInfos);
        info->addOperation(operation);
        _collectMetalCallableTableInfo(operation, info);
        if (!info->representativeSchemaOperation)
            info->representativeSchemaOperation = operation;
        _collectMetalPayloadPartitions(operation, info);
    }

    for (auto info : orderedInfos)
    {
        _calculateMetalRecordStrides(info, targetRequest);
        _collectMetalPayloadPartitionRequirements(
            info,
            traceContextRequirements,
            capabilityTagMask);
        if (!_synthesizeMetalProgramDescriptor(module, info, physicalTypesByValue))
            return false;
        _addMetalPayloadMetadataDecorations(module, info);
        for (Index i = 0; i < info->payloadPartitions.getCount(); ++i)
        {
            SLANG_RELEASE_ASSERT(info->representativeSchemaOperation);
            auto& partition = info->payloadPartitions[i];
            partition.rayDataInfo = _createMetalRayDataInfo(
                module,
                info->representativeSchemaOperation,
                partition.payloadType,
                i,
                info->payloadPartitions.getCount() > 1,
                targetRequest);
        }
    }
    return true;
}

void prepareMetalStructuralRayTracing(
    IRModule* module,
    List<IRFunc*>& entryPoints,
    TargetRequest* targetRequest,
    DiagnosticSink* sink)
{
    List<IRInst*> operations;
    _collectStructuralProgramOperations(module->getModuleInst(), operations);
    bool hasStructuralEntryPoint = false;
    bool hasInvalidStructuralEntryPoint = false;
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (func && func->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        {
            hasStructuralEntryPoint = true;
            hasInvalidStructuralEntryPoint |= !validateStructuralRayTracingEntryPoint(func, sink);
        }
    }
    if (operations.getCount() == 0 && !hasStructuralEntryPoint)
        return;
    if (hasInvalidStructuralEntryPoint)
        return;

    // Schema validation must happen before descriptor physicalization. Invalid operations are
    // removed here so that they cannot contribute payload partitions or leave behind a partially
    // rewritten `TraceProgramDescriptor` type.
    List<IRInst*> validOperations;
    for (auto operation : operations)
    {
        if (!validateStructuralRayTracingSchemaOperation(operation, sink))
        {
            operation->removeAndDeallocate();
            continue;
        }
        validOperations.add(operation);
    }

    UInt capabilityTagMask = 0;
    if (targetRequest->getTargetCaps().implies(CapabilityAtom::metal_raytracing_extended_limits))
    {
        capabilityTagMask |= UInt(MetalStructuralRayTracingTag::ExtendedLimits);
    }

    // Resolve target-dependent trace-context requirements before constructing any schema
    // descriptor. Invalid traces cannot be allowed to freeze a payload table's signature, and a
    // later operation must consume exactly the same normalized fact collected here.
    Dictionary<IRInst*, MetalTraceContextRequirements> traceContextRequirements;
    List<IRInst*> targetValidOperations;
    for (auto operation : validOperations)
    {
        MetalTraceContextRequirements requirements;
        if (!_tryGetMetalTraceContextRequirements(operation, targetRequest, sink, requirements))
        {
            operation->removeAndDeallocate();
            continue;
        }
        traceContextRequirements.add(operation, requirements);
        targetValidOperations.add(operation);
    }
    validOperations = _Move(targetValidOperations);

    Dictionary<IRInst*, RefPtr<MetalProgramDescriptorInfo>> programDescriptorInfos;
    SLANG_RELEASE_ASSERT(_prepareMetalProgramDescriptors(
        module,
        validOperations,
        targetRequest,
        traceContextRequirements,
        capabilityTagMask,
        programDescriptorInfos));

    Dictionary<IRInst*, HashSet<IRFunc*>> referencingEntryPoints;
    buildEntryPointReferenceGraph(referencingEntryPoints, module);

    IRBuilder builder(module);
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*> generatedMissAdapters;
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*> generatedClosestHitAdapters;
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*> generatedCallableAdapters;
    Dictionary<KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*>, IRFunc*>
        generatedCandidateHelpers;
    Dictionary<MetalCandidateDispatcherKey, IRFunc*> generatedCandidateDispatchers;
    HashSet<IRFunc*> candidateAdapterSet;
    HashSet<IRFunc*> candidateHelperSet;
    Dictionary<IRFunc*, IRInst*> payloadValues;
    Dictionary<IRFunc*, MetalDispatchValues> dispatchValues;
    Dictionary<IRFunc*, IRParam*> candidateRayDataParams;
    Dictionary<IRInst*, IRType*> accelerationStructureTypes;
    HashSet<IRFunc*> physicalRayGenerationEntryPoints;
    auto filterResultInfo =
        _createMetalCandidateResultType(module, "StructuralRayTracingFilterResult", false);
    auto proceduralResultInfo =
        _createMetalCandidateResultType(module, "StructuralRayTracingIntersectionResult", true);
    // Materializing a stage adapter can clone a structural trace or callable dispatch from the
    // stage body. For example, inlining a helper called by one callable can expose a dispatch to a
    // second callable inside the generated visible function. Grow this work list whenever the
    // current round is exhausted so no structural marker reaches type legalization.
    for (Index operationIndex = 0;; ++operationIndex)
    {
        if (operationIndex == validOperations.getCount())
        {
            // All original operations have been removed or lowered at this point, so any raw
            // structural operations still in the module were introduced by adapter synthesis.
            // Such an operation is a clone of source IR that contributed to descriptor
            // physicalization; its schema must therefore already have one canonical descriptor
            // layout. A schema invented here would make layout depend on synthesis order.
            List<IRInst*> generatedOperations;
            _collectStructuralProgramOperations(module->getModuleInst(), generatedOperations);
            for (auto generatedOperation : generatedOperations)
            {
                if (!validateStructuralRayTracingSchemaOperation(generatedOperation, sink))
                {
                    generatedOperation->removeAndDeallocate();
                    continue;
                }

                MetalTraceContextRequirements requirements;
                if (!_tryGetMetalTraceContextRequirements(
                        generatedOperation,
                        targetRequest,
                        sink,
                        requirements))
                {
                    generatedOperation->removeAndDeallocate();
                    continue;
                }

                auto programLayout = _getStructuralRayTracingProgramLayout(generatedOperation);
                SLANG_RELEASE_ASSERT(programDescriptorInfos.containsKey(programLayout));
                traceContextRequirements.add(generatedOperation, requirements);
                validOperations.add(generatedOperation);
            }

            if (operationIndex == validOperations.getCount())
                break;
        }

        auto operation = validOperations[operationIndex];
        auto programLayout =
            as<IRStructuralRayTracingTrace>(operation)
                ? cast<IRStructuralRayTracingTrace>(operation)->getProgramLayout()
                : cast<IRStructuralRayTracingCallShader>(operation)->getProgramLayout();
        auto foundProgramInfo = programDescriptorInfos.tryGetValue(programLayout);
        SLANG_RELEASE_ASSERT(foundProgramInfo);
        auto programInfo = foundProgramInfo->Ptr();

        if (!programInfo->areCallableEntriesMaterialized)
        {
            if (!_materializeMetalCallableTable(
                    module,
                    operation,
                    programInfo,
                    generatedCallableAdapters,
                    dispatchValues))
            {
                operation->removeAndDeallocate();
                continue;
            }
            programInfo->areCallableEntriesMaterialized = true;
        }

        bool didMaterializePayloadEntries = true;
        if (!programInfo->arePayloadEntriesMaterialized)
        {
            // A descriptor represents its complete schema even when the only reachable operation
            // is `callShader`. Materialize every hit/miss payload partition from the canonical
            // schema metadata before considering which runtime operation activated it.
            for (auto& schemaPartition : programInfo->payloadPartitions)
            {
                didMaterializePayloadEntries &= _materializeMetalPayloadPartition(
                    module,
                    programInfo->representativeSchemaOperation,
                    schemaPartition.payloadType,
                    false,
                    programInfo,
                    generatedMissAdapters,
                    generatedClosestHitAdapters,
                    generatedCandidateHelpers,
                    generatedCandidateDispatchers,
                    candidateAdapterSet,
                    candidateHelperSet,
                    payloadValues,
                    dispatchValues,
                    candidateRayDataParams,
                    schemaPartition.rayDataInfo,
                    filterResultInfo,
                    proceduralResultInfo);
                if (!didMaterializePayloadEntries)
                    break;
            }
            programInfo->arePayloadEntriesMaterialized = didMaterializePayloadEntries;
        }
        if (!didMaterializePayloadEntries)
        {
            operation->removeAndDeallocate();
            continue;
        }

        auto enclosingFunc = _findEnclosingFunc(operation);
        if (enclosingFunc)
        {
            if (auto referencing = getReferencingEntryPoints(referencingEntryPoints, enclosingFunc))
            {
                for (auto entryPoint : *referencing)
                {
                    _makeStructuralRayGenerationEntryPointPhysicalCompute(builder, entryPoint);
                    if (auto decoration = entryPoint->findDecoration<IREntryPointDecoration>())
                    {
                        if (decoration->getProfile().getStage() == Stage::Compute)
                            physicalRayGenerationEntryPoints.add(entryPoint);
                    }
                }
            }
        }

        if (auto callOperation = as<IRStructuralRayTracingCallShader>(operation))
        {
            _lowerCallableDispatch(module, callOperation, programInfo, sink);
            continue;
        }

        auto trace = cast<IRStructuralRayTracingTrace>(operation);
        auto traceRequirements = traceContextRequirements.tryGetValue(operation);
        SLANG_RELEASE_ASSERT(traceRequirements);
        IRBuilder operationBuilder(trace);
        if (!_setMetalAccelerationStructureType(
                operationBuilder,
                trace->getAccelerationStructure(),
                traceRequirements->tagMask,
                accelerationStructureTypes,
                sink))
        {
            trace->removeAndDeallocate();
            continue;
        }

        // An empty logical SBT has no shader to dispatch after traversal and no candidate function
        // to invoke during traversal. The trace therefore has no observable shader-side effect.
        // Keep non-empty programs intact until the table/dispatch lowering consumes them.
        if (!_hasStructuralShaderEntries(trace))
        {
            SLANG_ASSERT(trace->getDataType()->getOp() == kIROp_VoidType);
            trace->removeAndDeallocate();
        }
        else
        {
            auto partition = programInfo->findPayloadPartition(trace->getPayloadType());
            SLANG_RELEASE_ASSERT(partition && partition->rayDataInfo);
            if (!_materializeMetalPayloadPartition(
                    module,
                    trace,
                    trace->getPayloadType(),
                    true,
                    programInfo,
                    generatedMissAdapters,
                    generatedClosestHitAdapters,
                    generatedCandidateHelpers,
                    generatedCandidateDispatchers,
                    candidateAdapterSet,
                    candidateHelperSet,
                    payloadValues,
                    dispatchValues,
                    candidateRayDataParams,
                    partition->rayDataInfo,
                    filterResultInfo,
                    proceduralResultInfo))
            {
                trace->removeAndDeallocate();
            }
        }
    }

    _lowerMetalRayGenerationSystemValues(
        module,
        referencingEntryPoints,
        physicalRayGenerationEntryPoints,
        dispatchValues);

    lowerMetalStructuralRayTracingStageInputOperations(module, payloadValues);
    for (auto child = module->getModuleInst()->getFirstChild(); child; child = child->getNextInst())
    {
        if (auto info = child->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
            info->removeAndDeallocate();
    }
    for (auto adapter : candidateAdapterSet)
    {
        auto rayDataParam = candidateRayDataParams.tryGetValue(adapter);
        SLANG_ASSERT(rayDataParam);
        _convertCandidateParameterToRayData(adapter, *rayDataParam);
        if (auto readNone = adapter->findDecoration<IRReadNoneDecoration>())
            readNone->removeAndDeallocate();
    }
    for (auto helper : candidateHelperSet)
    {
        if (auto readNone = helper->findDecoration<IRReadNoneDecoration>())
            readNone->removeAndDeallocate();
    }

    // Keep this parameter while the pass grows into adapter synthesis. It also documents that the
    // physical entry points being rewritten are the linked target program's selected entry points.
    SLANG_UNUSED(entryPoints);
}

static IRInst* _findKernelContextValue(IRInst* operation)
{
    auto func = _findEnclosingFunc(operation);
    if (!func)
        return nullptr;

    for (auto param : func->getParams())
    {
        if (param->findDecoration<IRExplicitGlobalContextDecoration>())
            return param;
    }
    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (inst->findDecoration<IRExplicitGlobalContextDecoration>())
                return inst;
        }
    }
    return nullptr;
}

static IRParam* _findKernelContextParameter(IRFunc* func)
{
    for (auto param : func->getParams())
    {
        if (param->findDecoration<IRExplicitGlobalContextDecoration>())
            return param;
    }
    return nullptr;
}

static void _eraseVisibleFunctionKernelContextType(
    IRModule* module,
    IRFunc* func,
    IRParam* contextParam)
{
    auto typedContextPointer = cast<IRType>(contextParam->getDataType());
    IRBuilder builder(module);
    builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
    auto typedContext = builder.emitBitCast(typedContextPointer, contextParam);
    contextParam->replaceUsesWith(typedContext);
    typedContext->setOperand(0, contextParam);
    contextParam->setFullType(
        builder.getPtrType(builder.getUInt8Type(), AddressSpace::ThreadLocal));
    fixUpFuncType(func);
}

static void _appendErasedKernelContextParameter(IRModule* module, IRFunc* func)
{
    IRBuilder builder(module);
    auto param =
        builder.createParam(builder.getPtrType(builder.getUInt8Type(), AddressSpace::ThreadLocal));
    builder.addNameHintDecoration(param, toSlice("kernelContext"));
    builder.addDecoration(param, kIROp_ExplicitGlobalContextDecoration);
    param->insertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
    fixUpFuncType(func);
}

void finalizeMetalStructuralRayTracingGlobalContext(IRModule* module, DiagnosticSink* sink)
{
    List<IRFunc*> visibleFunctions;
    List<IRFunc*> contextUsingVisibleFunctions;
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (!func)
            continue;
        auto visibleDecoration = func->findDecoration<IRMetalVisibleFunctionDecoration>();
        if (visibleDecoration)
            visibleFunctions.add(func);
        auto contextParam = _findKernelContextParameter(func);
        if (!contextParam)
            continue;

        if (func->findDecoration<IRMetalIntersectionFunctionDecoration>())
        {
            sink->diagnose(Diagnostics::StructuralRayTracingMetalCandidateGlobalParameter{
                .location = func->sourceLoc});
            continue;
        }
        if (visibleDecoration)
        {
            _eraseVisibleFunctionKernelContextType(module, func, contextParam);
            contextUsingVisibleFunctions.add(func);
        }
    }

    HashSet<IRMetalVisibleFunctionTable*> tablesWithGlobalContext;
    List<IRMetalVisibleFunctionTable*> visibleFunctionTables;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto table = as<IRMetalVisibleFunctionTable>(inst))
            visibleFunctionTables.add(table);
    }

    IRBuilder builder(module);
    for (auto table : visibleFunctionTables)
    {
        IRFuncType* contextSignature = nullptr;
        for (auto adapter : contextUsingVisibleFunctions)
        {
            auto decoration = adapter->findDecoration<IRMetalVisibleFunctionDecoration>();
            if (!decoration || decoration->getTableType() != table)
                continue;
            contextSignature = cast<IRFuncType>(adapter->getDataType());
            SLANG_RELEASE_ASSERT(
                contextSignature->getParamCount() == table->getFunctionType()->getParamCount() + 1);
            break;
        }
        if (!contextSignature)
            continue;

        // Every function stored in one visible-function table must have the same signature. If
        // one adapter acquired the explicit global context, give the other adapters in that table
        // an unused erased context parameter as well.
        for (auto adapter : visibleFunctions)
        {
            auto decoration = adapter->findDecoration<IRMetalVisibleFunctionDecoration>();
            if (!decoration || decoration->getTableType() != table)
                continue;
            auto adapterType = cast<IRFuncType>(adapter->getDataType());
            if (adapterType == table->getFunctionType())
            {
                _appendErasedKernelContextParameter(module, adapter);
                adapterType = cast<IRFuncType>(adapter->getDataType());
            }
            SLANG_RELEASE_ASSERT(adapterType == contextSignature);
        }

        IRInst* operands[] = {contextSignature, table->getStageKind()};
        auto contextTable = cast<IRMetalVisibleFunctionTable>(
            builder.getType(kIROp_MetalVisibleFunctionTable, SLANG_COUNT_OF(operands), operands));
        table->replaceUsesWith(contextTable);
        tablesWithGlobalContext.add(contextTable);
    }

    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (!func)
            continue;
        for (auto block : func->getBlocks())
        {
            for (auto child = block->getFirstOrdinaryInst(); child; child = child->getNextInst())
            {
                if (auto trace = as<IRMetalStructuralRayTracingTrace>(child))
                {
                    auto missTable =
                        as<IRMetalVisibleFunctionTable>(trace->getMissFunctions()->getDataType());
                    auto closestHitTable = as<IRMetalVisibleFunctionTable>(
                        trace->getClosestHitFunctions()->getDataType());
                    bool missUsesContext = missTable && tablesWithGlobalContext.contains(missTable);
                    bool closestHitUsesContext =
                        closestHitTable && tablesWithGlobalContext.contains(closestHitTable);
                    if (!missUsesContext && !closestHitUsesContext)
                        continue;
                    auto context = _findKernelContextValue(trace);
                    SLANG_ASSERT(context);
                    IRBuilder builder(trace);
                    trace->setOperand(27, builder.getBoolValue(missUsesContext));
                    trace->setOperand(28, builder.getBoolValue(closestHitUsesContext));
                    trace->setOperand(29, context);
                    continue;
                }
                if (auto callShader = as<IRMetalStructuralRayTracingCallShader>(child))
                {
                    auto field = cast<IRStructField>(callShader->getCallableFunctionsField());
                    auto table = as<IRMetalVisibleFunctionTable>(field->getFieldType());
                    if (!table || !tablesWithGlobalContext.contains(table))
                        continue;
                    auto context = _findKernelContextValue(callShader);
                    SLANG_ASSERT(context);
                    IRBuilder builder(callShader);
                    callShader->setOperand(12, builder.getBoolValue(true));
                    callShader->setOperand(13, context);
                }
            }
        }
    }
}

} // namespace Slang
