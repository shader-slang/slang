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
        else if (as<IRStructuralRayTracingStageInputOperation>(child))
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

    _inlineCandidateOperationCalls(adapter);
    _lowerAnyHitTerminations(adapter, resultInfo);
    generated.add(groupType, adapter);
    return adapter;
}

struct MetalProceduralCandidateState
{
    IRVar* hasCandidate = nullptr;
    IRVar* currentMaxDistance = nullptr;
    IRVar* distance = nullptr;
    IRVar* hitKind = nullptr;
    IRVar* attributes = nullptr;
    IRInst* minDistance = nullptr;
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
    IRInst* attributes,
    IRInst* distance,
    IRInst* hitKind)
{
    List<IRInst*> operations;
    _collectStageInputOperations(helper, operations);
    for (auto operation : operations)
    {
        IRInst* replacement = nullptr;
        switch (operation->getOp())
        {
        case kIROp_StructuralRayTracingGetHitAttributes:
            replacement = attributes;
            break;
        case kIROp_StructuralRayTracingGetRayTCurrent:
            replacement = distance;
            break;
        case kIROp_StructuralRayTracingGetHitKind:
            replacement = hitKind;
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
    auto attributesType = cast<IRType>(group->getHitAttributesType());
    IRType* parameterTypes[] = {
        builder.getBorrowInParamType(attributesType, AddressSpace::ThreadLocal),
        builder.getFloatType(),
        builder.getUIntType(),
    };
    helper->setFullType(
        builder.getFuncType(SLANG_COUNT_OF(parameterTypes), parameterTypes, resultInfo.type));

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
        group->getHitAttributesType(),
        StructuralRayTracingHitAttributesKind::Custom);

    builder.setInsertInto(helper);
    builder.emitBlock();
    auto attributesAddress = builder.emitParam(parameterTypes[0]);
    auto distance = builder.emitParam(builder.getFloatType());
    auto hitKind = builder.emitParam(builder.getUIntType());
    builder.addNameHintDecoration(
        attributesAddress,
        UnownedTerminatedStringSlice("attributesAddress"));
    builder.addNameHintDecoration(distance, UnownedTerminatedStringSlice("distance"));
    builder.addNameHintDecoration(hitKind, UnownedTerminatedStringSlice("hitKind"));
    auto attributes = builder.emitLoad(attributesAddress);

    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn(_emitMetalCandidateResult(builder, resultInfo, true, true));

    _inlineCandidateOperationCalls(helper);
    _lowerAnyHitDecisionInputs(helper, attributes, distance, hitKind);
    _lowerAnyHitTerminations(helper, resultInfo);
    return helper;
}

static void _lowerProceduralReportHitOperations(
    IRFunc* adapter,
    const MetalProceduralCandidateState& state,
    IRFunc* anyHitDecision,
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
            auto attributesAddress = builder.emitVar(attributes->getDataType());
            builder.emitStore(attributesAddress, attributes);
            IRInst* arguments[] = {attributesAddress, distance, effectiveHitKind};
            auto decision = builder.emitCallInst(
                filterResultInfo.type,
                anyHitDecision,
                SLANG_COUNT_OF(arguments),
                arguments);
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

static IRFunc* _generateBoundingBoxCandidateAdapter(
    IRModule* module,
    Dictionary<IRInst*, IRFunc*>& generated,
    HashSet<IRFunc*>& generatedHelpers,
    const MetalCandidateResultInfo& filterResultInfo,
    const MetalCandidateResultInfo& proceduralResultInfo,
    IRStructuralRayTracingHitGroupInfoDecoration* group)
{
    auto invoke = as<IRFunc>(group->getIntersection());
    if (!invoke)
        return nullptr;
    auto groupType = group->getGroupType();
    if (auto existing = generated.tryGetValue(groupType))
        return *existing;

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    IRType* parameterTypes[] = {builder.getFloatType(), builder.getFloatType()};
    adapter->setFullType(builder.getFuncType(
        SLANG_COUNT_OF(parameterTypes),
        parameterTypes,
        proceduralResultInfo.type));

    auto name = _getMetalCandidateName(groupType);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addKeepAliveDecoration(adapter);
    IRInst* intersectionOperands[] = {
        builder.getIntValue(
            builder.getIntType(),
            IRIntegerValue(MetalStructuralRayTracingGeometryKind::BoundingBox)),
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
        StructuralRayTracingStageKind::Intersection,
        invoke,
        group->getContextType(),
        group->getPayloadType(),
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

    MetalProceduralCandidateState state;
    state.minDistance = minDistance;
    state.hasCandidate = builder.emitVar(builder.getBoolType());
    state.currentMaxDistance = builder.emitVar(builder.getFloatType());
    state.distance = builder.emitVar(builder.getFloatType());
    state.hitKind = builder.emitVar(builder.getUIntType());
    state.attributes = builder.emitVar(cast<IRType>(group->getHitAttributesType()));
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
    auto anyHitDecision = _generateAnyHitDecisionHelper(module, filterResultInfo, group);
    if (anyHitDecision)
        generatedHelpers.add(anyHitDecision);
    _lowerProceduralReportHitOperations(
        adapter,
        state,
        anyHitDecision,
        filterResultInfo,
        proceduralResultInfo);
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
    HashSet<IRFunc*>& candidateHelperSet,
    const MetalCandidateResultInfo& filterResultInfo,
    const MetalCandidateResultInfo& proceduralResultInfo)
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
                        filterResultInfo,
                        group))
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
                        filterResultInfo,
                        proceduralResultInfo,
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
    HashSet<IRFunc*> candidateHelperSet;
    auto filterResultInfo =
        _createMetalCandidateResultType(module, "StructuralRayTracingFilterResult", false);
    auto proceduralResultInfo =
        _createMetalCandidateResultType(module, "StructuralRayTracingIntersectionResult", true);
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
                candidateHelperSet,
                filterResultInfo,
                proceduralResultInfo));
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
    for (auto helper : candidateHelperSet)
    {
        if (auto readNone = helper->findDecoration<IRReadNoneDecoration>())
            readNone->removeAndDeallocate();
        if (auto info = helper->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
            info->removeAndDeallocate();
    }

    // Keep this parameter while the pass grows into adapter synthesis. It also documents that the
    // physical entry points being rewritten are the linked target program's selected entry points.
    SLANG_UNUSED(entryPoints);
}

} // namespace Slang
