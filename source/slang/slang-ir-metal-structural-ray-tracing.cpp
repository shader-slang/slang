#include "slang-ir-metal-structural-ray-tracing.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-inline.h"
#include "slang-ir-insts.h"
#include "slang-ir-synthesize-structural-ray-tracing.h"
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

static bool _hasStructuralShaderGroups(IRInst* operation)
{
    for (auto decoration = operation->getFirstDecoration(); decoration;
         decoration = decoration->getNextDecoration())
    {
        if (as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration) ||
            as<IRStructuralRayTracingMissGroupInfoDecoration>(decoration) ||
            as<IRStructuralRayTracingCallableGroupInfoDecoration>(decoration))
        {
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

static String _getStructuralStageName(IRType* stageType, IRFunc* invoke)
{
    if (stageType)
    {
        if (auto nameHint = stageType->findDecoration<IRNameHintDecoration>())
            return String(nameHint->getName());
    }
    if (invoke)
    {
        if (auto nameHint = invoke->findDecoration<IRNameHintDecoration>())
        {
            auto name = nameHint->getName();
            Index separator = name.indexOf(toSlice(".invoke"));
            return separator >= 0 ? String(name.head(separator)) : String(name);
        }
    }
    return "structuralRayTracingStage";
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
    IRStructuralRayTracingTrace* trace,
    StructuralRayTracingStageKind stageKind)
{
    MetalStageRequirements result;
    for (auto decoration : trace->getDecorations())
    {
        IRInst* invoke = nullptr;
        if (stageKind == StructuralRayTracingStageKind::ClosestHit)
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
                invoke = group->getClosestHit();
        }
        else if (stageKind == StructuralRayTracingStageKind::Miss)
        {
            if (auto group = as<IRStructuralRayTracingMissGroupInfoDecoration>(decoration))
                invoke = group->getMiss();
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
    IRStructuralRayTracingTrace* trace)
{
    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    auto result = RefPtr<MetalRayDataInfo>(new MetalRayDataInfo());
    result->type = builder.createStructType();
    StringBuilder name;
    if (auto nameHint = trace->getProgramLayout()->findDecoration<IRNameHintDecoration>())
        name << nameHint->getName();
    else
        name << "StructuralRayTracingProgram";
    name << ".rayData";
    builder.addNameHintDecoration(result->type, name.getUnownedSlice());

    auto payloadPointerType = cast<IRPtrTypeBase>(trace->getPayload()->getDataType());
    result->payloadKey = builder.createStructKey();
    builder.addNameHintDecoration(result->payloadKey, UnownedTerminatedStringSlice("payload"));
    builder.createStructField(result->type, result->payloadKey, payloadPointerType->getValueType());

    bool needsRecordData = false;
    bool needsMinDistance = false;
    bool needsRayTime = false;
    bool needsRayFlags = false;
    bool needsDispatchRaysIndex = false;
    bool needsDispatchRaysDimensions = false;
    bool needsCustomHitKind = false;
    for (auto decoration : trace->getDecorations())
    {
        if (auto missGroup = as<IRStructuralRayTracingMissGroupInfoDecoration>(decoration))
        {
            auto requirements = _getMetalStageRequirements(missGroup->getMiss());
            needsRecordData |= requirements.record || requirements.callableDispatch;
            needsMinDistance |= requirements.minDistance || requirements.objectSpaceRay;
            needsRayTime |= requirements.rayTime;
            needsRayFlags |= requirements.rayFlags;
            needsDispatchRaysIndex |= requirements.dispatchRaysIndex;
            needsDispatchRaysDimensions |= requirements.dispatchRaysDimensions;
            continue;
        }
        if (auto callableGroup = as<IRStructuralRayTracingCallableGroupInfoDecoration>(decoration))
        {
            auto requirements = _getMetalStageRequirements(callableGroup->getCallable());
            needsRecordData |= requirements.record;
            needsDispatchRaysIndex |= requirements.dispatchRaysIndex;
            needsDispatchRaysDimensions |= requirements.dispatchRaysDimensions;
            continue;
        }

        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (!group ||
            StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue()) !=
                StructuralRayTracingHitAttributesKind::Custom)
        {
            if (group)
            {
                auto closestHitRequirements = _getMetalStageRequirements(group->getClosestHit());
                auto anyHitRequirements = _getMetalStageRequirements(group->getAnyHit());
                auto intersectionRequirements =
                    _getMetalStageRequirements(group->getIntersection());
                needsRecordData |=
                    closestHitRequirements.record || closestHitRequirements.callableDispatch;
                needsRecordData |= anyHitRequirements.record;
                needsRecordData |= intersectionRequirements.record;
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
            }
            continue;
        }

        auto requirements = _getMetalStageRequirements(group->getClosestHit());
        auto anyHitRequirements = _getMetalStageRequirements(group->getAnyHit());
        auto intersectionRequirements = _getMetalStageRequirements(group->getIntersection());
        needsRecordData |= requirements.record || requirements.callableDispatch;
        needsRecordData |= anyHitRequirements.record;
        needsRecordData |= intersectionRequirements.record;
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
    IRStructuralRayTracingTrace* trace,
    TargetRequest* targetRequest,
    DiagnosticSink* sink,
    UInt& outTagMask,
    IRIntegerValue& outMaxLevels)
{
    outTagMask = UInt(MetalStructuralRayTracingTag::Instancing);
    outMaxLevels = 0;

    auto accelerationStructureType =
        as<IRRaytracingAccelerationStructureType>(trace->getAccelerationStructure()->getDataType());
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
            .location = trace->sourceLoc});
        return false;
    }

    if (levelCount->getValue() == 1)
        outTagMask = 0;
    else
    {
        if (!_supportsMetalLib31(targetRequest))
        {
            sink->diagnose(Diagnostics::StructuralRayTracingMultilevelRequiresMetallib31{
                .location = trace->sourceLoc});
            return false;
        }
        outMaxLevels = levelCount->getValue();
    }

    return true;
}

static bool _validateMetalCurveSupport(
    IRStructuralRayTracingTrace* trace,
    TargetRequest* targetRequest,
    DiagnosticSink* sink)
{
    if (_supportsMetalLib31(targetRequest))
        return true;
    for (auto decoration : trace->getDecorations())
    {
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (group &&
            StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue()) ==
                StructuralRayTracingHitAttributesKind::Curve)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingCurveRequiresMetallib31{
                .location = trace->sourceLoc});
            return false;
        }
    }
    return true;
}

static bool _addMetalMotionTags(
    IRStructuralRayTracingTrace* trace,
    DiagnosticSink* sink,
    UInt& ioTagMask)
{
    auto motionKind =
        StructuralRayTracingMotionKind(cast<IRIntLit>(trace->getMotionKind())->getValue());
    if (motionKind == StructuralRayTracingMotionKind::Invalid ||
        UInt(motionKind) > UInt(StructuralRayTracingMotionKind::Primitive) +
                               UInt(StructuralRayTracingMotionKind::Instance))
    {
        sink->diagnose(
            Diagnostics::InvalidStructuralRayTracingMotion{.location = trace->sourceLoc});
        return false;
    }

    if ((UInt(motionKind) & UInt(StructuralRayTracingMotionKind::Primitive)) != 0)
        ioTagMask |= UInt(MetalStructuralRayTracingTag::PrimitiveMotion);
    if ((UInt(motionKind) & UInt(StructuralRayTracingMotionKind::Instance)) != 0)
    {
        if ((ioTagMask & UInt(MetalStructuralRayTracingTag::Instancing)) == 0)
        {
            sink->diagnose(Diagnostics::StructuralRayTracingInstanceMotionRequiresInstancing{
                .location = trace->sourceLoc});
            return false;
        }
        ioTagMask |= UInt(MetalStructuralRayTracingTag::InstanceMotion);
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
    IRStructuralRayTracingTrace* trace,
    UInt topologyTagMask,
    UInt capabilityTagMask)
{
    UInt result = topologyTagMask | capabilityTagMask;
    for (auto decoration : trace->getDecorations())
    {
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (!group)
            continue;

        auto closestHit = _getMetalStageRequirements(group->getClosestHit());
        auto anyHit = _getMetalStageRequirements(group->getAnyHit());
        auto intersection = _getMetalStageRequirements(group->getIntersection());
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

static IRInst* _emitMetalRecordValueFromDescriptorData(
    IRBuilder& builder,
    IRInst* descriptorData,
    MetalDescriptorDataSection section,
    IRIntegerValue slot,
    IRType* recordType);

static IRInst* _emitMetalRecordValue(
    IRBuilder& builder,
    IRInst* rayData,
    MetalRayDataInfo* rayDataInfo,
    MetalDescriptorDataSection section,
    IRIntegerValue slot,
    IRType* recordType)
{
    SLANG_ASSERT(rayDataInfo->recordDataKey);
    auto descriptorData =
        builder.emitLoad(builder.emitFieldAddress(rayData, rayDataInfo->recordDataKey));
    return _emitMetalRecordValueFromDescriptorData(
        builder,
        descriptorData,
        section,
        slot,
        recordType);
}

static IRInst* _emitMetalRecordValueFromDescriptorData(
    IRBuilder& builder,
    IRInst* descriptorData,
    MetalDescriptorDataSection section,
    IRIntegerValue slot,
    IRType* recordType)
{
    auto tableOffset = builder.emitLoad(builder.emitGetOffsetPtr(
        descriptorData,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(UInt(section)))));
    auto recordOffsetIndex = builder.emitAdd(
        builder.getUIntType(),
        tableOffset,
        builder.getIntValue(builder.getUIntType(), slot));
    auto recordByteOffset =
        builder.emitLoad(builder.emitGetOffsetPtr(descriptorData, recordOffsetIndex));
    auto bytePointerType = builder.getPtrType(builder.getUInt8Type(), AddressSpace::Global);
    auto recordByteBase = builder.emitBitCast(bytePointerType, descriptorData);
    auto recordByteAddress = builder.emitGetOffsetPtr(recordByteBase, recordByteOffset);
    auto recordPointerType = builder.getPtrType(recordType, AddressSpace::Global);
    return builder.emitLoad(builder.emitBitCast(recordPointerType, recordByteAddress));
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
    IRType* contextType,
    IRType* payloadType,
    IRType* recordType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind,
    IRType* callableDataType = nullptr)
{
    auto voidType = builder.getVoidType();
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(stageKind)),
        invoke,
        contextType ? contextType : voidType,
        payloadType ? payloadType : voidType,
        recordType ? recordType : voidType,
        hitAttributesType ? hitAttributesType : voidType,
        callableDataType ? callableDataType : voidType,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(hitAttributesKind)),
    };
    builder.addDecoration(
        adapter,
        kIROp_StructuralRayTracingEntryPointInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands));
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
    IRInst* groupType,
    StructuralRayTracingStageKind stageKind,
    IRType* stageType,
    IRInst* invokeValue,
    IRType* contextType,
    IRType* payloadType,
    IRType* recordType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind,
    IRIntLit* slotIndex,
    const MetalStageRequirements& tableRequirements,
    MetalRayDataInfo* rayDataInfo,
    IRType* descriptorResourcesPointerType)
{
    auto invoke = as<IRFunc>(invokeValue);
    if (!invoke)
        return nullptr;
    KeyValuePair<IRInst*, IRInst*> generatedKey(groupType, rayDataInfo->type);
    if (auto existing = generated.tryGetValue(generatedKey))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    List<IRType*> parameterTypes;
    auto rayDataPointerType = builder.getPtrType(rayDataInfo->type, AddressSpace::ThreadLocal);
    parameterTypes.add(rayDataPointerType);
    if (tableRequirements.distance || tableRequirements.objectSpaceRay)
        parameterTypes.add(builder.getFloatType());
    if (tableRequirements.hitKind)
        parameterTypes.add(builder.getUIntType());
    if (tableRequirements.triangleBarycentricCoord)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 2));
    if (tableRequirements.triangleFrontFacing)
        parameterTypes.add(builder.getBoolType());
    if (tableRequirements.curveParameter)
        parameterTypes.add(builder.getFloatType());
    if (tableRequirements.worldSpaceOrigin)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (tableRequirements.worldSpaceDirection)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (tableRequirements.primitiveIndex)
        parameterTypes.add(builder.getUIntType());
    if (tableRequirements.geometryIndex)
        parameterTypes.add(builder.getUIntType());
    if (tableRequirements.instanceIndex)
        parameterTypes.add(builder.getUIntType());
    if (tableRequirements.instanceID)
        parameterTypes.add(builder.getUIntType());
    if (tableRequirements.objectSpaceRay)
    {
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    }
    if (tableRequirements.objectToWorld)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (tableRequirements.worldToObject)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (tableRequirements.callableDispatch)
        parameterTypes.add(descriptorResourcesPointerType);
    adapter->setFullType(builder.getFuncType(parameterTypes, builder.getVoidType()));

    auto name = _getStructuralStageName(stageType, invoke);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addKeepAliveDecoration(adapter);
    builder.addDecoration(adapter, kIROp_MetalVisibleFunctionDecoration);
    _addStructuralStageInfo(
        builder,
        adapter,
        stageKind,
        invoke,
        contextType,
        payloadType,
        recordType,
        hitAttributesType,
        hitAttributesKind);

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
    IRInst* descriptorResources = nullptr;
    if (tableRequirements.callableDispatch)
        descriptorResources = emitNamedParam(descriptorResourcesPointerType, "descriptorResources");

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
    {
        auto section = stageKind == StructuralRayTracingStageKind::Miss
                           ? MetalDescriptorDataSection::MissRecords
                           : MetalDescriptorDataSection::HitRecords;
        values.record = _emitMetalRecordValue(
            builder,
            rayData,
            rayDataInfo,
            section,
            slotIndex->getValue(),
            recordType);
    }

    if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Custom)
    {
        if (auto key = rayDataInfo->customAttributeKeys.tryGetValue(groupType))
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
    IRStructuralRayTracingCallableGroupInfoDecoration* group,
    IRType* descriptorResourcesPointerType,
    IRType* callableFunctionTableType,
    const MetalStageRequirements& signatureRequirements)
{
    auto invoke = as<IRFunc>(group->getCallable());
    if (!invoke)
        return nullptr;
    KeyValuePair<IRInst*, IRInst*> generatedKey(group->getGroupType(), callableFunctionTableType);
    if (auto existing = generated.tryGetValue(generatedKey))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    auto dataType = cast<IRType>(group->getCallableDataType());
    auto dataPointerType = builder.getPtrType(dataType, AddressSpace::ThreadLocal);
    auto descriptorDataType = builder.getPtrType(builder.getUIntType(), AddressSpace::Global);
    auto requirements = _getMetalStageRequirements(invoke);
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    List<IRType*> parameterTypes;
    parameterTypes.add(dataPointerType);
    if (signatureRequirements.dispatchRaysIndex)
        parameterTypes.add(uint3Type);
    if (signatureRequirements.dispatchRaysDimensions)
        parameterTypes.add(uint3Type);
    parameterTypes.add(descriptorResourcesPointerType);
    parameterTypes.add(descriptorDataType);
    adapter->setFullType(builder.getFuncType(
        parameterTypes.getCount(),
        parameterTypes.getBuffer(),
        builder.getVoidType()));

    auto name = _getStructuralStageName(group->getCallableType(), invoke);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addKeepAliveDecoration(adapter);
    builder.addDecoration(adapter, kIROp_MetalVisibleFunctionDecoration);
    _addStructuralStageInfo(
        builder,
        adapter,
        StructuralRayTracingStageKind::Callable,
        invoke,
        group->getContextType(),
        nullptr,
        group->getRecordType(),
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
    auto descriptorResources = builder.emitParam(descriptorResourcesPointerType);
    builder.addNameHintDecoration(
        descriptorResources,
        UnownedTerminatedStringSlice("descriptorResources"));
    auto descriptorData = builder.emitParam(descriptorDataType);
    builder.addNameHintDecoration(descriptorData, UnownedTerminatedStringSlice("descriptorData"));

    IRInst* record = nullptr;
    if (requirements.record)
    {
        record = _emitMetalRecordValueFromDescriptorData(
            builder,
            descriptorData,
            MetalDescriptorDataSection::CallableRecords,
            group->getSlotIndex()->getValue(),
            cast<IRType>(group->getRecordType()));
    }

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

static String _getMetalCandidateName(IRType* groupType)
{
    StringBuilder name;
    if (auto nameHint = groupType->findDecoration<IRNameHintDecoration>())
        name << nameHint->getName();
    else
        name << "structuralRayTracingHitGroup";
    name << ".candidate";
    return name.produceString();
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
        SLANG_ASSERT(inlineCall(callToInline));
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
    Dictionary<IRFunc*, IRParam*>& candidateRayDataParams,
    const MetalCandidateResultInfo& resultInfo,
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    UInt tagMask,
    MetalRayDataInfo* rayDataInfo)
{
    auto invoke = as<IRFunc>(group->getAnyHit());
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
    auto geometryKind = hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle
                            ? MetalStructuralRayTracingGeometryKind::Triangle
                            : MetalStructuralRayTracingGeometryKind::Curve;
    auto requirements = _getMetalStageRequirements(invoke);
    List<IRType*> parameterTypes;
    if (requirements.distance || requirements.objectSpaceRay)
        parameterTypes.add(builder.getFloatType());
    if (requirements.triangleBarycentricCoord)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 2));
    if (requirements.triangleFrontFacing ||
        (requirements.hitKind &&
         hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle))
    {
        parameterTypes.add(builder.getBoolType());
    }
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
    auto rayDataPointerType = builder.getPtrType(rayDataInfo->type, AddressSpace::ThreadLocal);
    parameterTypes.add(rayDataPointerType);
    adapter->setFullType(builder.getFuncType(parameterTypes, resultInfo.type));

    auto name = _getMetalCandidateName(groupType);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addKeepAliveDecoration(adapter);
    IRInst* intersectionOperands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(geometryKind)),
        builder.getIntValue(builder.getIntType(), IRIntegerValue(tagMask)),
        builder.getIntValue(builder.getIntType(), 0),
    };
    builder.addDecoration(
        adapter,
        kIROp_MetalIntersectionFunctionDecoration,
        intersectionOperands,
        SLANG_COUNT_OF(intersectionOperands));
    _addStructuralStageInfo(
        builder,
        adapter,
        StructuralRayTracingStageKind::AnyHit,
        invoke,
        group->getContextType(),
        group->getPayloadType(),
        group->getRecordType(),
        group->getHitAttributesType(),
        hitAttributesKind);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    MetalCandidateInputValues inputs;
    if (requirements.distance || requirements.objectSpaceRay)
        inputs.distance =
            _emitMetalSystemValueParam(builder, builder.getFloatType(), "distance", "distance");
    if (requirements.triangleBarycentricCoord)
    {
        inputs.triangleBarycentricCoord = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 2),
            "barycentricCoord",
            "barycentric_coord");
    }
    if (requirements.triangleFrontFacing ||
        (requirements.hitKind &&
         hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle))
    {
        inputs.triangleFrontFacing = _emitMetalSystemValueParam(
            builder,
            builder.getBoolType(),
            "frontFacing",
            "front_facing");
    }
    if (requirements.curveParameter)
    {
        inputs.curveParameter = _emitMetalSystemValueParam(
            builder,
            builder.getFloatType(),
            "curveParameter",
            "curve_parameter");
    }
    if (requirements.worldSpaceOrigin)
    {
        inputs.worldSpaceOrigin = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceOrigin",
            "world_space_origin");
    }
    if (requirements.worldSpaceDirection)
    {
        inputs.worldSpaceDirection = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceDirection",
            "world_space_direction");
    }
    if (requirements.primitiveIndex)
    {
        inputs.primitiveIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "primitiveIndex",
            "primitive_id");
    }
    if (requirements.geometryIndex)
    {
        inputs.geometryIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "geometryIndex",
            "geometry_id");
    }
    if (requirements.instanceIndex)
    {
        inputs.instanceIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "instanceIndex",
            "instance_id");
    }
    if (requirements.instanceID)
    {
        inputs.instanceID = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "instanceID",
            "user_instance_id");
    }
    if (requirements.objectSpaceRay)
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
    if (requirements.objectToWorld)
    {
        inputs.objectToWorld = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "objectToWorld",
            "object_to_world_transform");
    }
    if (requirements.worldToObject)
    {
        inputs.worldToObject = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "worldToObject",
            "world_to_object_transform");
    }
    auto rayData = builder.emitParam(rayDataPointerType);
    builder.addNameHintDecoration(rayData, UnownedTerminatedStringSlice("rayData"));
    candidateRayDataParams[adapter] = rayData;
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
        inputs.record = _emitMetalRecordValue(
            builder,
            rayData,
            rayDataInfo,
            MetalDescriptorDataSection::HitRecords,
            group->getSlotIndex()->getValue(),
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
    auto invoke = as<IRFunc>(group->getAnyHit());
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

    auto name = _getMetalCandidateName(group->getGroupType());
    name.append(".anyHit");
    builder.addNameHintDecoration(helper, name.getUnownedSlice());
    _addStructuralStageInfo(
        builder,
        helper,
        StructuralRayTracingStageKind::AnyHit,
        invoke,
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
    Dictionary<IRFunc*, IRParam*>& candidateRayDataParams,
    const MetalCandidateResultInfo& filterResultInfo,
    const MetalCandidateResultInfo& proceduralResultInfo,
    IRStructuralRayTracingHitGroupInfoDecoration* group,
    UInt tagMask,
    MetalRayDataInfo* rayDataInfo)
{
    auto invoke = as<IRFunc>(group->getIntersection());
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
    auto anyHitRequirements = _getMetalStageRequirements(group->getAnyHit());
    auto requirements =
        _combineMetalStageRequirements(intersectionRequirements, anyHitRequirements);
    List<IRType*> parameterTypes;
    parameterTypes.add(builder.getFloatType());
    parameterTypes.add(builder.getFloatType());
    if (requirements.objectSpaceRay)
    {
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    }
    if (requirements.objectToWorld)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (requirements.worldToObject)
        parameterTypes.add(_getFloat4x3Type(builder));
    if (requirements.primitiveIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.geometryIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.instanceIndex)
        parameterTypes.add(builder.getUIntType());
    if (requirements.instanceID)
        parameterTypes.add(builder.getUIntType());
    if (requirements.worldSpaceOrigin)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (requirements.worldSpaceDirection)
        parameterTypes.add(builder.getVectorType(builder.getFloatType(), 3));
    if (as<IRFunc>(group->getAnyHit()))
        parameterTypes.add(builder.getBoolType());
    auto rayDataPointerType = builder.getPtrType(rayDataInfo->type, AddressSpace::ThreadLocal);
    parameterTypes.add(rayDataPointerType);
    adapter->setFullType(builder.getFuncType(parameterTypes, proceduralResultInfo.type));

    auto name = _getMetalCandidateName(groupType);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addKeepAliveDecoration(adapter);
    IRInst* intersectionOperands[] = {
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(MetalStructuralRayTracingGeometryKind::BoundingBox)),
        builder.getIntValue(builder.getIntType(), IRIntegerValue(tagMask)),
        builder.getIntValue(builder.getIntType(), 0),
    };
    builder.addDecoration(
        adapter,
        kIROp_MetalIntersectionFunctionDecoration,
        intersectionOperands,
        SLANG_COUNT_OF(intersectionOperands));
    _addStructuralStageInfo(
        builder,
        adapter,
        StructuralRayTracingStageKind::Intersection,
        invoke,
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
    if (requirements.objectSpaceRay)
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
    if (requirements.objectToWorld)
    {
        objectToWorld = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "objectToWorld",
            "object_to_world_transform");
    }
    if (requirements.worldToObject)
    {
        worldToObject = _emitMetalSystemValueParam(
            builder,
            _getFloat4x3Type(builder),
            "worldToObject",
            "world_to_object_transform");
    }
    if (requirements.primitiveIndex)
    {
        primitiveIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "primitiveIndex",
            "primitive_id");
    }
    if (requirements.geometryIndex)
    {
        geometryIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "geometryIndex",
            "geometry_id");
    }
    if (requirements.instanceIndex)
    {
        instanceIndex = _emitMetalSystemValueParam(
            builder,
            builder.getUIntType(),
            "instanceIndex",
            "instance_id");
    }
    if (requirements.instanceID)
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
    if (requirements.worldSpaceOrigin)
    {
        worldSpaceOrigin = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceOrigin",
            "world_space_origin");
    }
    if (requirements.worldSpaceDirection)
    {
        worldSpaceDirection = _emitMetalSystemValueParam(
            builder,
            builder.getVectorType(builder.getFloatType(), 3),
            "worldSpaceDirection",
            "world_space_direction");
    }
    if (as<IRFunc>(group->getAnyHit()))
    {
        opaque = _emitMetalSystemValueParam(builder, builder.getBoolType(), "opaque", "opaque");
    }
    auto rayData = builder.emitParam(rayDataPointerType);
    builder.addNameHintDecoration(rayData, UnownedTerminatedStringSlice("rayData"));
    candidateRayDataParams[adapter] = rayData;
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
        state.record = _emitMetalRecordValue(
            builder,
            rayData,
            rayDataInfo,
            MetalDescriptorDataSection::HitRecords,
            group->getSlotIndex()->getValue(),
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

static bool _getTraceDescriptorFields(
    IRInst* descriptor,
    IRStructField*& outDescriptorResourcesField,
    List<IRStructField*>& outResourceFields)
{
    auto descriptorType = as<IRStructType>(descriptor->getDataType());
    if (!descriptorType)
        return false;

    List<IRStructField*> descriptorFields;
    _getStructFields(descriptorType, descriptorFields);
    if (descriptorFields.getCount() != 1)
        return false;

    auto resourcesParameterBlock =
        as<IRUniformParameterGroupType>(descriptorFields[0]->getFieldType());
    auto resourcesType = resourcesParameterBlock
                             ? as<IRStructType>(resourcesParameterBlock->getElementType())
                             : nullptr;
    if (!resourcesType)
        return false;

    _getStructFields(resourcesType, outResourceFields);
    if (outResourceFields.getCount() != 5)
        return false;
    outDescriptorResourcesField = descriptorFields[0];
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
    if (requirements.callableDispatch)
        parameterTypes.add(descriptorResourcesPointerType);
    return builder.getFuncType(parameterTypes, builder.getVoidType());
}

static bool _prepareTraceDescriptor(
    IRBuilder& builder,
    IRStructuralRayTracingTrace* trace,
    UInt tagMaskValue,
    IRIntegerValue maxLevelsValue,
    const MetalStageRequirements& missRequirements,
    const MetalStageRequirements& closestHitRequirements,
    MetalRayDataInfo* rayDataInfo,
    MetalTraceDescriptorInfo& outInfo)
{
    IRStructField* descriptorResourcesField = nullptr;
    List<IRStructField*> resourceFields;
    if (!_getTraceDescriptorFields(
            trace->getDescriptor(),
            descriptorResourcesField,
            resourceFields))
        return false;

    auto descriptorResourcesPointerType =
        builder.getPtrType(builder.getUIntType(), AddressSpace::Uniform);

    auto intType = builder.getIntType();
    auto tagMask = builder.getIntValue(intType, IRIntegerValue(tagMaskValue));
    auto maxLevels = builder.getIntValue(intType, maxLevelsValue);
    IRInst* intersectionTableOperands[] = {tagMask, maxLevels};
    auto intersectionFunctionTableType = builder.getType(
        kIROp_MetalIntersectionFunctionTable,
        SLANG_COUNT_OF(intersectionTableOperands),
        intersectionTableOperands);

    auto missFunctionTableType = builder.getType(
        kIROp_MetalVisibleFunctionTable,
        _getMetalVisibleFunctionSignature(
            builder,
            rayDataInfo->type,
            missRequirements,
            descriptorResourcesPointerType));
    auto closestHitFunctionTableType = builder.getType(
        kIROp_MetalVisibleFunctionTable,
        _getMetalVisibleFunctionSignature(
            builder,
            rayDataInfo->type,
            closestHitRequirements,
            descriptorResourcesPointerType));

    resourceFields[0]->setFieldType(intersectionFunctionTableType);
    resourceFields[1]->setFieldType(missFunctionTableType);
    resourceFields[2]->setFieldType(closestHitFunctionTableType);

    outInfo.descriptorResourcesField = descriptorResourcesField;
    outInfo.descriptorResourcesPointerType = descriptorResourcesPointerType;
    outInfo.intersectionFunctionsField = resourceFields[0];
    outInfo.missFunctionsField = resourceFields[1];
    outInfo.closestHitFunctionsField = resourceFields[2];
    outInfo.callableFunctionsField = resourceFields[3];
    outInfo.recordsField = resourceFields[4];
    outInfo.intersectionFunctionTableType = intersectionFunctionTableType;
    outInfo.missFunctionTableType = missFunctionTableType;
    outInfo.closestHitFunctionTableType = closestHitFunctionTableType;
    return true;
}

static bool _prepareCallableDescriptor(
    IRBuilder& builder,
    IRStructuralRayTracingCallShader* callOperation,
    const MetalStageRequirements& requirements,
    MetalTraceDescriptorInfo& outInfo)
{
    IRStructField* descriptorResourcesField = nullptr;
    List<IRStructField*> resourceFields;
    if (!_getTraceDescriptorFields(
            callOperation->getDescriptor(),
            descriptorResourcesField,
            resourceFields))
    {
        return false;
    }

    auto dataType = cast<IRType>(callOperation->getCallableDataType());
    List<IRType*> parameterTypes;
    parameterTypes.add(builder.getPtrType(dataType, AddressSpace::ThreadLocal));
    auto uint3Type =
        builder.getVectorType(builder.getUIntType(), builder.getIntValue(builder.getIntType(), 3));
    if (requirements.dispatchRaysIndex)
        parameterTypes.add(uint3Type);
    if (requirements.dispatchRaysDimensions)
        parameterTypes.add(uint3Type);
    parameterTypes.add(builder.getPtrType(builder.getUIntType(), AddressSpace::Uniform));
    parameterTypes.add(builder.getPtrType(builder.getUIntType(), AddressSpace::Global));
    auto signature = builder.getFuncType(
        parameterTypes.getCount(),
        parameterTypes.getBuffer(),
        builder.getVoidType());
    auto callableFunctionTableType = builder.getType(kIROp_MetalVisibleFunctionTable, signature);
    resourceFields[3]->setFieldType(callableFunctionTableType);

    outInfo.descriptorResourcesField = descriptorResourcesField;
    outInfo.descriptorResourcesPointerType =
        builder.getPtrType(builder.getUIntType(), AddressSpace::Uniform);
    outInfo.intersectionFunctionsField = resourceFields[0];
    outInfo.missFunctionsField = resourceFields[1];
    outInfo.closestHitFunctionsField = resourceFields[2];
    outInfo.callableFunctionsField = resourceFields[3];
    outInfo.recordsField = resourceFields[4];
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

static MetalStructuralRayTracingGeometryKind _getGeometryKind(IRStructuralRayTracingTrace* trace)
{
    auto result = MetalStructuralRayTracingGeometryKind::Unknown;
    for (auto decoration : trace->getDecorations())
    {
        auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration);
        if (!group)
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

static bool _lowerNonEmptyTrace(
    IRModule* module,
    IRStructuralRayTracingTrace* trace,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generatedMissAdapters,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generatedClosestHitAdapters,
    Dictionary<KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*>, IRFunc*>&
        generatedCandidateAdapters,
    HashSet<IRFunc*>& candidateAdapterSet,
    HashSet<IRFunc*>& candidateHelperSet,
    Dictionary<IRFunc*, IRInst*>& payloadValues,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues,
    Dictionary<IRFunc*, IRParam*>& candidateRayDataParams,
    MetalRayDataInfo* rayDataInfo,
    const MetalCandidateResultInfo& filterResultInfo,
    const MetalCandidateResultInfo& proceduralResultInfo,
    UInt topologyTagMask,
    UInt capabilityTagMask,
    IRIntegerValue maxLevels)
{
    IRBuilder builder(module);
    auto tagMask = _getSharedMetalTagMask(trace, topologyTagMask, capabilityTagMask);
    auto missRequirements = _getMetalStageRequirements(trace, StructuralRayTracingStageKind::Miss);
    auto closestHitRequirements =
        _getMetalStageRequirements(trace, StructuralRayTracingStageKind::ClosestHit);
    MetalTraceDescriptorInfo descriptorInfo;
    if (!_prepareTraceDescriptor(
            builder,
            trace,
            tagMask,
            maxLevels,
            missRequirements,
            closestHitRequirements,
            rayDataInfo,
            descriptorInfo))
        return false;

    bool hasMissFunctions = false;
    bool hasClosestHitFunctions = false;
    bool hasIntersectionFunctions = false;
    for (auto decoration : trace->getDecorations())
    {
        if (auto group = as<IRStructuralRayTracingMissGroupInfoDecoration>(decoration))
        {
            if (_generateVisibleStageAdapter(
                    module,
                    generatedMissAdapters,
                    payloadValues,
                    dispatchValues,
                    group->getGroupType(),
                    StructuralRayTracingStageKind::Miss,
                    group->getMissType(),
                    group->getMiss(),
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getRecordType(),
                    nullptr,
                    StructuralRayTracingHitAttributesKind::None,
                    group->getSlotIndex(),
                    missRequirements,
                    rayDataInfo,
                    descriptorInfo.descriptorResourcesPointerType))
            {
                hasMissFunctions = true;
            }
        }
        else if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
        {
            auto hitAttributesKind =
                StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue());
            if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle ||
                hitAttributesKind == StructuralRayTracingHitAttributesKind::Curve)
            {
                if (auto candidate = _generateBuiltInAnyHitCandidateAdapter(
                        module,
                        generatedCandidateAdapters,
                        payloadValues,
                        dispatchValues,
                        candidateRayDataParams,
                        filterResultInfo,
                        group,
                        tagMask,
                        rayDataInfo))
                {
                    hasIntersectionFunctions = true;
                    candidateAdapterSet.add(candidate);
                }
            }
            else if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Custom)
            {
                if (auto candidate = _generateBoundingBoxCandidateAdapter(
                        module,
                        generatedCandidateAdapters,
                        candidateHelperSet,
                        payloadValues,
                        dispatchValues,
                        candidateRayDataParams,
                        filterResultInfo,
                        proceduralResultInfo,
                        group,
                        tagMask,
                        rayDataInfo))
                {
                    hasIntersectionFunctions = true;
                    candidateAdapterSet.add(candidate);
                }
            }
            if (_generateVisibleStageAdapter(
                    module,
                    generatedClosestHitAdapters,
                    payloadValues,
                    dispatchValues,
                    group->getGroupType(),
                    StructuralRayTracingStageKind::ClosestHit,
                    group->getClosestHitType(),
                    group->getClosestHit(),
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getRecordType(),
                    group->getHitAttributesType(),
                    hitAttributesKind,
                    group->getSlotIndex(),
                    closestHitRequirements,
                    rayDataInfo,
                    descriptorInfo.descriptorResourcesPointerType))
            {
                hasClosestHitFunctions = true;
            }
        }
    }

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
        builder.getIntValue(intType, IRIntegerValue(_getGeometryKind(trace))),
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
        rayData,
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

static bool _lowerCallableDispatch(
    IRModule* module,
    IRStructuralRayTracingCallShader* callOperation,
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*>& generatedCallableAdapters,
    Dictionary<IRFunc*, MetalDispatchValues>& dispatchValues,
    DiagnosticSink* sink)
{
    IRBuilder builder(module);
    List<IRStructuralRayTracingCallableGroupInfoDecoration*> callableGroups;
    MetalStageRequirements callableRequirements;
    bool hasCallableGroup = false;
    bool hasMismatchedData = false;
    for (auto decoration : callOperation->getDecorations())
    {
        auto group = as<IRStructuralRayTracingCallableGroupInfoDecoration>(decoration);
        if (!group)
            continue;
        hasCallableGroup = true;
        callableGroups.add(group);
        callableRequirements = _combineMetalStageRequirements(
            callableRequirements,
            _getMetalStageRequirements(group->getCallable()));
        if (group->getCallableDataType() != callOperation->getCallableDataType())
        {
            hasMismatchedData = true;
            sink->diagnose(Diagnostics::StructuralRayTracingCallableDataMismatch{
                .slot = Int64(group->getSlotIndex()->getValue()),
                .actualType = group->getCallableDataType(),
                .expectedType = callOperation->getCallableDataType(),
                .location = callOperation->sourceLoc});
            continue;
        }
    }
    if (!hasCallableGroup)
    {
        sink->diagnose(Diagnostics::StructuralRayTracingCallWithoutGroups{
            .location = callOperation->sourceLoc});
        return false;
    }
    if (hasMismatchedData)
        return false;

    MetalTraceDescriptorInfo descriptorInfo;
    if (!_prepareCallableDescriptor(builder, callOperation, callableRequirements, descriptorInfo))
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
    auto dispatchRaysIndex = callableRequirements.dispatchRaysIndex
                                 ? builder.emitIntrinsicInst(
                                       uint3Type,
                                       kIROp_MetalStructuralRayTracingDispatchRaysIndex,
                                       0,
                                       nullptr)
                                 : zeroDispatchValue;
    auto dispatchRaysDimensions = callableRequirements.dispatchRaysDimensions
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
        builder.getBoolValue(callableRequirements.dispatchRaysIndex),
        builder.getBoolValue(callableRequirements.dispatchRaysDimensions),
        descriptorResources,
        records,
        descriptorResources->getDataType(),
        descriptorInfo.callableFunctionsField,
    };
    builder.emitIntrinsicInst(
        builder.getVoidType(),
        kIROp_MetalStructuralRayTracingCallShader,
        SLANG_COUNT_OF(operands),
        operands);
    callOperation->removeFromParent();
    for (auto group : callableGroups)
    {
        _generateCallableStageAdapter(
            module,
            generatedCallableAdapters,
            dispatchValues,
            group,
            descriptorInfo.descriptorResourcesPointerType,
            descriptorInfo.callableFunctionTableType,
            callableRequirements);
    }
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
        auto call = as<IRCall>(child);
        if (!call)
            continue;
        auto callee = getResolvedInstForDecorations(call->getCallee());
        auto intrinsic = findAnyTargetIntrinsicDecoration(callee);
        UnownedStringSlice definition;
        if (intrinsic)
            definition = intrinsic->getDefinition();
        else if (auto nameHint = callee->findDecoration<IRNameHintDecoration>())
        {
            // Target specialization removes an intrinsic decoration whose capability set does
            // not include Metal, but retains the linked core declaration and its linkage name.
            // Require that exact compiler-owned symbol so a user function with the same source
            // name is not treated as a built-in.
            if (auto linkage = callee->findDecoration<IRLinkageDecoration>())
            {
                auto mangledName = linkage->getMangledName();
                if (mangledName == toSlice("_S4core17DispatchRaysIndexp0pv3u") ||
                    mangledName == toSlice("_S4core22DispatchRaysDimensionsp0pv3u"))
                {
                    definition = nameHint->getName();
                }
            }
        }
        if (definition == toSlice("DispatchRaysIndex"))
            dispatchIndexOperations.add(call);
        else if (definition == toSlice("DispatchRaysDimensions"))
            dispatchDimensionsOperations.add(call);
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

void prepareMetalStructuralRayTracing(
    IRModule* module,
    List<IRFunc*>& entryPoints,
    TargetRequest* targetRequest,
    DiagnosticSink* sink)
{
    List<IRInst*> operations;
    _collectStructuralProgramOperations(module->getModuleInst(), operations);
    bool hasStructuralEntryPoint = false;
    for (auto inst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(inst);
        if (func && func->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        {
            hasStructuralEntryPoint = true;
            break;
        }
    }
    if (operations.getCount() == 0 && !hasStructuralEntryPoint)
        return;

    UInt capabilityTagMask = 0;
    if (targetRequest->getTargetCaps().implies(CapabilityAtom::metal_raytracing_extended_limits))
    {
        capabilityTagMask |= UInt(MetalStructuralRayTracingTag::ExtendedLimits);
    }

    Dictionary<IRInst*, HashSet<IRFunc*>> referencingEntryPoints;
    buildEntryPointReferenceGraph(referencingEntryPoints, module);

    IRBuilder builder(module);
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*> generatedMissAdapters;
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*> generatedClosestHitAdapters;
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRFunc*> generatedCallableAdapters;
    Dictionary<KeyValuePair<KeyValuePair<IRInst*, UInt>, IRInst*>, IRFunc*>
        generatedCandidateAdapters;
    HashSet<IRFunc*> candidateAdapterSet;
    HashSet<IRFunc*> candidateHelperSet;
    Dictionary<IRFunc*, IRInst*> payloadValues;
    Dictionary<IRFunc*, MetalDispatchValues> dispatchValues;
    Dictionary<IRFunc*, IRParam*> candidateRayDataParams;
    Dictionary<IRInst*, RefPtr<MetalRayDataInfo>> rayDataInfos;
    Dictionary<IRInst*, IRType*> accelerationStructureTypes;
    HashSet<IRFunc*> physicalRayGenerationEntryPoints;
    auto filterResultInfo =
        _createMetalCandidateResultType(module, "StructuralRayTracingFilterResult", false);
    auto proceduralResultInfo =
        _createMetalCandidateResultType(module, "StructuralRayTracingIntersectionResult", true);
    for (auto operation : operations)
    {
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
            _lowerCallableDispatch(
                module,
                callOperation,
                generatedCallableAdapters,
                dispatchValues,
                sink);
            continue;
        }

        auto trace = cast<IRStructuralRayTracingTrace>(operation);
        UInt topologyTagMask = 0;
        IRIntegerValue maxLevels = 0;
        if (!_getMetalAccelerationStructureTopology(
                trace,
                targetRequest,
                sink,
                topologyTagMask,
                maxLevels) ||
            !_validateMetalCurveSupport(trace, targetRequest, sink))
        {
            trace->removeAndDeallocate();
            continue;
        }
        if (!_addMetalMotionTags(trace, sink, topologyTagMask))
        {
            trace->removeAndDeallocate();
            continue;
        }
        IRBuilder operationBuilder(trace);
        if (!_setMetalAccelerationStructureType(
                operationBuilder,
                trace->getAccelerationStructure(),
                topologyTagMask,
                accelerationStructureTypes,
                sink))
        {
            trace->removeAndDeallocate();
            continue;
        }

        // An empty logical SBT has no shader to dispatch after traversal and no candidate function
        // to invoke during traversal. The trace therefore has no observable shader-side effect.
        // Keep non-empty programs intact until the table/dispatch lowering consumes them.
        if (!_hasStructuralShaderGroups(trace))
        {
            SLANG_ASSERT(trace->getDataType()->getOp() == kIROp_VoidType);
            trace->removeAndDeallocate();
        }
        else
        {
            RefPtr<MetalRayDataInfo> rayDataInfo;
            if (auto existing = rayDataInfos.tryGetValue(trace->getProgramLayout()))
                rayDataInfo = *existing;
            else
            {
                rayDataInfo = _createMetalRayDataInfo(module, trace);
                rayDataInfos.add(trace->getProgramLayout(), rayDataInfo);
            }
            if (!_lowerNonEmptyTrace(
                    module,
                    trace,
                    generatedMissAdapters,
                    generatedClosestHitAdapters,
                    generatedCandidateAdapters,
                    candidateAdapterSet,
                    candidateHelperSet,
                    payloadValues,
                    dispatchValues,
                    candidateRayDataParams,
                    rayDataInfo,
                    filterResultInfo,
                    proceduralResultInfo,
                    topologyTagMask,
                    capabilityTagMask,
                    maxLevels))
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

} // namespace Slang
