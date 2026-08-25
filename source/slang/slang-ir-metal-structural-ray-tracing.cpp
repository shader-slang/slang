#include "slang-ir-metal-structural-ray-tracing.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-insts.h"
#include "slang-ir-synthesize-structural-ray-tracing.h"
#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

static void _collectStructuralTraceOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectStructuralTraceOperations(child, operations);
        if (child->getOp() == kIROp_StructuralRayTracingTrace)
            operations.add(child);
    }
}

static bool _hasStructuralShaderGroups(IRStructuralRayTracingTrace* trace)
{
    for (auto decoration = trace->getFirstDecoration(); decoration;
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

static void _addStructuralStageInfo(
    IRBuilder& builder,
    IRFunc* adapter,
    StructuralRayTracingStageKind stageKind,
    IRFunc* invoke,
    IRType* contextType,
    IRType* payloadType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind)
{
    auto voidType = builder.getVoidType();
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(stageKind)),
        invoke,
        contextType ? contextType : voidType,
        payloadType ? payloadType : voidType,
        hitAttributesType ? hitAttributesType : voidType,
        voidType,
        builder.getIntValue(builder.getIntType(), IRIntegerValue(hitAttributesKind)),
    };
    builder.addDecoration(
        adapter,
        kIROp_StructuralRayTracingEntryPointInfoDecoration,
        operands,
        SLANG_COUNT_OF(operands));
}

static IRFunc* _generateVisibleStageAdapter(
    IRModule* module,
    Dictionary<IRFunc*, IRFunc*>& generated,
    StructuralRayTracingStageKind stageKind,
    IRType* stageType,
    IRInst* invokeValue,
    IRType* contextType,
    IRType* payloadType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind,
    IRType* payloadPointerType)
{
    auto invoke = as<IRFunc>(invokeValue);
    if (!invoke)
        return nullptr;
    if (auto existing = generated.tryGetValue(invoke))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    List<IRType*> parameterTypes;
    parameterTypes.add(payloadPointerType);
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
        hitAttributesType,
        hitAttributesKind);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    auto payload = builder.emitParam(payloadPointerType);
    builder.addNameHintDecoration(payload, UnownedTerminatedStringSlice("payload"));

    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn();

    generated.add(invoke, adapter);
    return adapter;
}

static void _getStructFields(IRStructType* type, List<IRStructField*>& fields)
{
    for (auto field : type->getFields())
        fields.add(field);
}

static IRPtrType* _getMetalPayloadPointerType(IRBuilder& builder, IRInst* payload)
{
    auto payloadPointerType = cast<IRPtrTypeBase>(payload->getDataType());
    return builder.getPtrType(payloadPointerType->getValueType(), AddressSpace::ThreadLocal);
}

struct MetalTraceDescriptorInfo
{
    IRStructField* descriptorResourcesField = nullptr;
    IRStructField* intersectionFunctionsField = nullptr;
    IRStructField* missFunctionsField = nullptr;
    IRStructField* closestHitFunctionsField = nullptr;
    IRStructField* callableFunctionsField = nullptr;
    IRStructField* recordsField = nullptr;
    IRType* intersectionFunctionTableType = nullptr;
    IRType* visibleFunctionTableType = nullptr;
};

static bool _prepareTraceDescriptor(
    IRBuilder& builder,
    IRStructuralRayTracingTrace* trace,
    MetalTraceDescriptorInfo& outInfo)
{
    auto descriptorType = as<IRStructType>(trace->getDescriptor()->getDataType());
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

    List<IRStructField*> resourceFields;
    _getStructFields(resourcesType, resourceFields);
    if (resourceFields.getCount() != 5)
        return false;

    auto intType = builder.getIntType();
    auto tagMask =
        builder.getIntValue(intType, IRIntegerValue(MetalStructuralRayTracingTag::Instancing));
    auto maxLevels = builder.getIntValue(intType, 0);
    IRInst* intersectionTableOperands[] = {tagMask, maxLevels};
    auto intersectionFunctionTableType = builder.getType(
        kIROp_MetalIntersectionFunctionTable,
        SLANG_COUNT_OF(intersectionTableOperands),
        intersectionTableOperands);

    List<IRType*> visibleFunctionParameters;
    visibleFunctionParameters.add(_getMetalPayloadPointerType(builder, trace->getPayload()));
    auto visibleFunctionSignature =
        builder.getFuncType(visibleFunctionParameters, builder.getVoidType());
    auto visibleFunctionTableType =
        builder.getType(kIROp_MetalVisibleFunctionTable, visibleFunctionSignature);

    resourceFields[0]->setFieldType(intersectionFunctionTableType);
    resourceFields[1]->setFieldType(visibleFunctionTableType);
    resourceFields[2]->setFieldType(visibleFunctionTableType);

    outInfo.descriptorResourcesField = descriptorFields[0];
    outInfo.intersectionFunctionsField = resourceFields[0];
    outInfo.missFunctionsField = resourceFields[1];
    outInfo.closestHitFunctionsField = resourceFields[2];
    outInfo.callableFunctionsField = resourceFields[3];
    outInfo.recordsField = resourceFields[4];
    outInfo.intersectionFunctionTableType = intersectionFunctionTableType;
    outInfo.visibleFunctionTableType = visibleFunctionTableType;
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
    IRInst*& outRayFlags,
    IRInst*& outInstanceMask,
    IRInst*& outSbtOffset,
    IRInst*& outSbtStride,
    IRInst*& outMissIndex)
{
    auto descType = cast<IRStructType>(desc->getDataType());
    List<IRStructField*> descFields;
    _getStructFields(descType, descFields);
    SLANG_ASSERT(descFields.getCount() == 6);

    auto ray = builder.emitFieldExtract(desc, descFields[0]->getKey());
    auto rayType = cast<IRStructType>(ray->getDataType());
    List<IRStructField*> rayFields;
    _getStructFields(rayType, rayFields);
    SLANG_ASSERT(rayFields.getCount() == 4);

    outOrigin = builder.emitFieldExtract(ray, rayFields[0]->getKey());
    outMinDistance = builder.emitFieldExtract(ray, rayFields[1]->getKey());
    outDirection = builder.emitFieldExtract(ray, rayFields[2]->getKey());
    outMaxDistance = builder.emitFieldExtract(ray, rayFields[3]->getKey());
    outRayFlags = builder.emitFieldExtract(desc, descFields[1]->getKey());
    outInstanceMask = builder.emitFieldExtract(desc, descFields[2]->getKey());
    outSbtOffset = builder.emitFieldExtract(desc, descFields[3]->getKey());
    outSbtStride = builder.emitFieldExtract(desc, descFields[4]->getKey());
    outMissIndex = builder.emitFieldExtract(desc, descFields[5]->getKey());
}

static bool _lowerNonEmptyTrace(
    IRModule* module,
    IRStructuralRayTracingTrace* trace,
    Dictionary<IRFunc*, IRFunc*>& generatedMissAdapters,
    Dictionary<IRFunc*, IRFunc*>& generatedClosestHitAdapters)
{
    IRBuilder builder(module);
    MetalTraceDescriptorInfo descriptorInfo;
    if (!_prepareTraceDescriptor(builder, trace, descriptorInfo))
        return false;

    bool hasMissFunctions = false;
    bool hasClosestHitFunctions = false;
    bool hasIntersectionFunctions = false;
    auto metalPayloadPointerType = _getMetalPayloadPointerType(builder, trace->getPayload());
    for (auto decoration : trace->getDecorations())
    {
        if (auto group = as<IRStructuralRayTracingMissGroupInfoDecoration>(decoration))
        {
            if (_generateVisibleStageAdapter(
                    module,
                    generatedMissAdapters,
                    StructuralRayTracingStageKind::Miss,
                    group->getMissType(),
                    group->getMiss(),
                    group->getContextType(),
                    group->getPayloadType(),
                    nullptr,
                    StructuralRayTracingHitAttributesKind::None,
                    metalPayloadPointerType))
            {
                hasMissFunctions = true;
            }
        }
        else if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
        {
            hasIntersectionFunctions = hasIntersectionFunctions ||
                                       !as<IRVoidLit>(group->getAnyHit()) ||
                                       !as<IRVoidLit>(group->getIntersection());
            auto hitAttributesKind =
                StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue());
            if (_generateVisibleStageAdapter(
                    module,
                    generatedClosestHitAdapters,
                    StructuralRayTracingStageKind::ClosestHit,
                    group->getClosestHitType(),
                    group->getClosestHit(),
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getHitAttributesType(),
                    hitAttributesKind,
                    metalPayloadPointerType))
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

    IRInst* origin;
    IRInst* direction;
    IRInst* minDistance;
    IRInst* maxDistance;
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
        rayFlags,
        instanceMask,
        sbtOffset,
        sbtStride,
        missIndex);

    auto intType = builder.getIntType();
    IRInst* operands[] = {
        builder.getIntValue(intType, IRIntegerValue(MetalStructuralRayTracingTag::Instancing)),
        builder.getIntValue(intType, 0),
        builder.getIntValue(intType, IRIntegerValue(_getGeometryKind(trace))),
        builder.getBoolValue(hasIntersectionFunctions),
        builder.getBoolValue(hasMissFunctions),
        builder.getBoolValue(hasClosestHitFunctions),
        origin,
        direction,
        minDistance,
        maxDistance,
        rayFlags,
        instanceMask,
        sbtOffset,
        sbtStride,
        missIndex,
        trace->getAccelerationStructure(),
        intersectionFunctions,
        missFunctions,
        closestHitFunctions,
        records,
        trace->getPayload(),
    };
    builder.emitIntrinsicInst(
        builder.getVoidType(),
        kIROp_MetalStructuralRayTracingTrace,
        SLANG_COUNT_OF(operands),
        operands);
    trace->removeAndDeallocate();
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

void prepareMetalStructuralRayTracing(IRModule* module, List<IRFunc*>& entryPoints)
{
    List<IRInst*> operations;
    _collectStructuralTraceOperations(module->getModuleInst(), operations);
    if (operations.getCount() == 0)
        return;

    Dictionary<IRInst*, HashSet<IRFunc*>> referencingEntryPoints;
    buildEntryPointReferenceGraph(referencingEntryPoints, module);

    IRBuilder builder(module);
    Dictionary<IRFunc*, IRFunc*> generatedMissAdapters;
    Dictionary<IRFunc*, IRFunc*> generatedClosestHitAdapters;
    for (auto operation : operations)
    {
        auto trace = cast<IRStructuralRayTracingTrace>(operation);
        auto enclosingFunc = _findEnclosingFunc(operation);
        if (enclosingFunc)
        {
            if (auto referencing = getReferencingEntryPoints(referencingEntryPoints, enclosingFunc))
            {
                for (auto entryPoint : *referencing)
                    _makeStructuralRayGenerationEntryPointPhysicalCompute(builder, entryPoint);
            }
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
            SLANG_ASSERT(_lowerNonEmptyTrace(
                module,
                trace,
                generatedMissAdapters,
                generatedClosestHitAdapters));
        }
    }

    lowerMetalStructuralRayTracingPayloadOperations(module);

    // Keep this parameter while the pass grows into adapter synthesis. It also documents that the
    // physical entry points being rewritten are the linked target program's selected entry points.
    SLANG_UNUSED(entryPoints);
}

} // namespace Slang
