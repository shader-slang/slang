#include "slang-ir-metal-structural-ray-tracing.h"

#include "slang-ir-call-graph.h"
#include "slang-ir-inline.h"
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

struct MetalCandidateResultInfo
{
    IRStructType* type = nullptr;
};

static MetalCandidateResultInfo _createMetalCandidateResultType(IRModule* module)
{
    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());

    MetalCandidateResultInfo result;
    result.type = builder.createStructType();
    builder.addNameHintDecoration(
        result.type,
        UnownedTerminatedStringSlice("StructuralRayTracingCandidateResult"));

    auto acceptKey = builder.createStructKey();
    builder.addNameHintDecoration(acceptKey, UnownedTerminatedStringSlice("accept"));
    builder.addTargetSystemValueDecoration(acceptKey, toSlice("accept_intersection"));
    builder.createStructField(result.type, acceptKey, builder.getBoolType());

    auto continueSearchKey = builder.createStructKey();
    builder.addNameHintDecoration(
        continueSearchKey,
        UnownedTerminatedStringSlice("continueSearch"));
    builder.addTargetSystemValueDecoration(continueSearchKey, toSlice("continue_search"));
    builder.createStructField(result.type, continueSearchKey, builder.getBoolType());
    return result;
}

static IRInst* _emitMetalCandidateResult(
    IRBuilder& builder,
    const MetalCandidateResultInfo& resultInfo,
    bool accept,
    bool continueSearch)
{
    IRInst* values[] = {
        builder.getBoolValue(accept),
        builder.getBoolValue(continueSearch),
    };
    return builder.emitMakeStruct(resultInfo.type, SLANG_COUNT_OF(values), values);
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
    bool& hasAnyHitTermination)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectCallsAndAnyHitTerminations(child, calls, hasAnyHitTermination);
        if (auto call = as<IRCall>(child))
            calls.add(call);
        else if (
            child->getOp() == kIROp_StructuralRayTracingIgnoreHit ||
            child->getOp() == kIROp_StructuralRayTracingAcceptHitAndEndSearch)
            hasAnyHitTermination = true;
    }
}

static bool _functionCanReach(IRFunc* function, IRFunc* target, HashSet<IRFunc*>& activeFunctions)
{
    if (!activeFunctions.add(function))
        return false;

    List<IRCall*> calls;
    bool hasAnyHitTermination = false;
    _collectCallsAndAnyHitTerminations(function, calls, hasAnyHitTermination);
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

static void _inlineAnyHitTerminatingCalls(IRFunc* adapter)
{
    List<IRFunc*> reachableFunctions;
    HashSet<IRFunc*> reachableFunctionSet;
    reachableFunctions.add(adapter);
    reachableFunctionSet.add(adapter);
    for (Index i = 0; i < reachableFunctions.getCount(); ++i)
    {
        List<IRCall*> calls;
        bool hasAnyHitTermination = false;
        _collectCallsAndAnyHitTerminations(reachableFunctions[i], calls, hasAnyHitTermination);
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
        bool hasAnyHitTermination = false;
        _collectCallsAndAnyHitTerminations(func, calls, hasAnyHitTermination);
        if (hasAnyHitTermination)
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
            bool hasAnyHitTermination = false;
            _collectCallsAndAnyHitTerminations(func, calls, hasAnyHitTermination);
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
        bool hasAnyHitTermination = false;
        _collectCallsAndAnyHitTerminations(adapter, calls, hasAnyHitTermination);
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

static IRFunc* _generateTriangleAnyHitCandidateAdapter(
    IRModule* module,
    Dictionary<IRInst*, IRFunc*>& generated,
    const MetalCandidateResultInfo& resultInfo,
    IRStructuralRayTracingHitGroupInfoDecoration* group)
{
    auto invoke = as<IRFunc>(group->getAnyHit());
    if (!invoke)
        return nullptr;
    auto groupType = group->getGroupType();
    if (auto existing = generated.tryGetValue(groupType))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    adapter->setFullType(builder.getFuncType(0, nullptr, resultInfo.type));

    auto name = _getMetalCandidateName(groupType);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addKeepAliveDecoration(adapter);
    IRInst* intersectionOperands[] = {
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(MetalStructuralRayTracingGeometryKind::Triangle)),
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(MetalStructuralRayTracingTag::Instancing)),
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
        group->getHitAttributesType(),
        StructuralRayTracingHitAttributesKind::Triangle);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn(_emitMetalCandidateResult(builder, resultInfo, true, true));

    _inlineAnyHitTerminatingCalls(adapter);
    _lowerAnyHitTerminations(adapter, resultInfo);
    generated.add(groupType, adapter);
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

static void _convertCandidatePayloadToRayData(IRFunc* adapter)
{
    auto info = adapter->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
    if (!info || as<IRVoidType>(info->getPayloadType()))
        return;

    auto payloadType = cast<IRType>(info->getPayloadType());
    IRParam* payloadParam = nullptr;
    for (auto param : adapter->getParams())
    {
        auto ptrType = as<IRPtrTypeBase>(param->getDataType());
        if (ptrType && ptrType->getValueType() == payloadType)
            payloadParam = param;
    }
    if (!payloadParam)
        return;

    auto firstBlock = adapter->getFirstBlock();
    SLANG_ASSERT(firstBlock);
    auto firstOrdinaryInst = firstBlock->getFirstOrdinaryInst();
    SLANG_ASSERT(firstOrdinaryInst);

    IRBuilder builder(adapter);
    builder.setInsertBefore(firstOrdinaryInst);
    auto payloadStorage = builder.emitVar(payloadType);
    builder.addNameHintDecoration(payloadStorage, UnownedTerminatedStringSlice("payloadStorage"));
    payloadParam->replaceUsesWith(payloadStorage);
    builder.emitStore(payloadStorage, builder.emitLoad(payloadParam));

    List<IRReturn*> returns;
    _collectReturns(adapter, returns);
    for (auto returnInst : returns)
    {
        builder.setInsertBefore(returnInst);
        builder.emitStore(payloadParam, builder.emitLoad(payloadStorage));
    }

    payloadParam->setFullType(builder.getRefParamType(payloadType, AddressSpace::Generic));
    builder.addTargetSystemValueDecoration(payloadParam, toSlice("payload"));
    fixUpFuncType(adapter);
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
    Dictionary<IRFunc*, IRFunc*>& generatedClosestHitAdapters,
    Dictionary<IRInst*, IRFunc*>& generatedCandidateAdapters,
    HashSet<IRFunc*>& candidateAdapterSet,
    const MetalCandidateResultInfo& candidateResultInfo)
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
            auto hitAttributesKind =
                StructuralRayTracingHitAttributesKind(group->getHitAttributesKind()->getValue());
            if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Triangle)
            {
                if (auto candidate = _generateTriangleAnyHitCandidateAdapter(
                        module,
                        generatedCandidateAdapters,
                        candidateResultInfo,
                        group))
                {
                    hasIntersectionFunctions = true;
                    candidateAdapterSet.add(candidate);
                }
            }
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
    Dictionary<IRInst*, IRFunc*> generatedCandidateAdapters;
    HashSet<IRFunc*> candidateAdapterSet;
    auto candidateResultInfo = _createMetalCandidateResultType(module);
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
                generatedClosestHitAdapters,
                generatedCandidateAdapters,
                candidateAdapterSet,
                candidateResultInfo));
        }
    }

    lowerMetalStructuralRayTracingPayloadOperations(module);
    for (auto adapter : candidateAdapterSet)
    {
        _convertCandidatePayloadToRayData(adapter);
        if (auto readNone = adapter->findDecoration<IRReadNoneDecoration>())
            readNone->removeAndDeallocate();
        if (auto info = adapter->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        {
            info->removeAndDeallocate();
        }
    }

    // Keep this parameter while the pass grows into adapter synthesis. It also documents that the
    // physical entry points being rewritten are the linked target program's selected entry points.
    SLANG_UNUSED(entryPoints);
}

} // namespace Slang
