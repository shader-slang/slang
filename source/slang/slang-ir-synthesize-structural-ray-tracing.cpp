#include "slang-ir-synthesize-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

static void _collectTraceOperations(IRInst* parent, List<IRInst*>& operations);

static Stage _getStructuralRayTracingNativeStage(StructuralRayTracingStageKind kind)
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

static IRFunc* _getStructuralRayTracingStageFunc(IRInst* value)
{
    return as<IRFunc>(value);
}

static String _getStructuralRayTracingStageName(IRType* stageType, IRFunc* invoke)
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

static void _addEmptyStructuralRayTracingEntryPointLayout(
    IRBuilder& builder,
    IRFunc* func,
    Stage stage)
{
    IRStructTypeLayout::Builder paramsTypeLayoutBuilder(&builder);
    IRVarLayout::Builder paramsLayoutBuilder(&builder, paramsTypeLayoutBuilder.build());
    paramsLayoutBuilder.setStage(stage);

    IRTypeLayout::Builder resultTypeLayoutBuilder(&builder);
    IRVarLayout::Builder resultLayoutBuilder(&builder, resultTypeLayoutBuilder.build());
    resultLayoutBuilder.setStage(stage);

    auto entryPointLayout =
        builder.getEntryPointLayout(paramsLayoutBuilder.build(), resultLayoutBuilder.build());
    builder.addLayoutDecoration(func, entryPointLayout);
}

static void _addStructuralRayTracingEntryPointInfo(
    IRBuilder& builder,
    IRFunc* adapter,
    StructuralRayTracingStageKind stageKind,
    IRFunc* invoke,
    IRType* contextType,
    IRType* payloadType,
    IRType* hitAttributesType,
    StructuralRayTracingHitAttributesKind hitAttributesKind,
    IRType* callableDataType)
{
    auto voidType = builder.getVoidType();
    IRInst* operands[] = {
        builder.getIntValue(builder.getIntType(), IRIntegerValue(stageKind)),
        invoke,
        contextType ? contextType : voidType,
        payloadType ? payloadType : voidType,
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

struct StructuralRayTracingGeneratedEntryPoint
{
    StructuralRayTracingStageKind stageKind;
    IRFunc* invoke;
    IRFunc* adapter;
};

static IRFunc* _findGeneratedStructuralRayTracingEntryPoint(
    const List<StructuralRayTracingGeneratedEntryPoint>& generated,
    StructuralRayTracingStageKind stageKind,
    IRFunc* invoke)
{
    for (auto& item : generated)
    {
        if (item.stageKind == stageKind && item.invoke == invoke)
            return item.adapter;
    }
    return nullptr;
}

static IRFunc* _generateStructuralRayTracingEntryPoint(
    IRModule* module,
    List<IRFunc*>& ioEntryPoints,
    List<StructuralRayTracingGeneratedEntryPoint>& generated,
    StructuralRayTracingStageKind stageKind,
    IRType* stageType,
    IRInst* invokeValue,
    IRType* contextType,
    IRType* payloadType = nullptr,
    IRType* hitAttributesType = nullptr,
    StructuralRayTracingHitAttributesKind hitAttributesKind =
        StructuralRayTracingHitAttributesKind::None,
    IRType* callableDataType = nullptr)
{
    auto invoke = _getStructuralRayTracingStageFunc(invokeValue);
    if (!invoke)
        return nullptr;
    if (auto existing = _findGeneratedStructuralRayTracingEntryPoint(generated, stageKind, invoke))
    {
        return existing;
    }

    IRBuilder builder(module);
    builder.setInsertInto(module->getModuleInst());
    auto adapter = builder.createFunc();
    adapter->setFullType(builder.getFuncType(List<IRType*>(), builder.getVoidType()));

    auto stage = _getStructuralRayTracingNativeStage(stageKind);
    auto name = _getStructuralRayTracingStageName(stageType, invoke);
    builder.addNameHintDecoration(adapter, name.getUnownedSlice());
    builder.addEntryPointDecoration(
        adapter,
        Profile(stage),
        name.getUnownedSlice(),
        toSlice("structural-ray-tracing"));
    builder.addKeepAliveDecoration(adapter);
    _addEmptyStructuralRayTracingEntryPointLayout(builder, adapter, stage);
    _addStructuralRayTracingEntryPointInfo(
        builder,
        adapter,
        stageKind,
        invoke,
        contextType,
        payloadType,
        hitAttributesType,
        hitAttributesKind,
        callableDataType);

    builder.setInsertInto(adapter);
    builder.emitBlock();
    List<IRInst*> arguments;
    for (UInt i = 0; i < invoke->getParamCount(); ++i)
        arguments.add(builder.emitDefaultConstruct(invoke->getParamType(i)));
    builder
        .emitCallInst(invoke->getResultType(), invoke, arguments.getCount(), arguments.getBuffer());
    builder.emitReturn();

    generated.add({stageKind, invoke, adapter});
    ioEntryPoints.add(adapter);
    return adapter;
}

static void _validateStructuralRayTracingGroupSlot(
    IRInst* traceOperation,
    IRIntLit* slotIndex,
    const char* section,
    HashSet<IRIntegerValue>& usedSlots,
    DiagnosticSink* sink)
{
    auto value = slotIndex->getValue();
    if (value < 0)
    {
        sink->diagnose(Diagnostics::InvalidStructuralRayTracingGroupSlot{
            .section = section,
            .slot = Int64(value),
            .location = traceOperation->sourceLoc});
        return;
    }
    if (!usedSlots.add(value))
    {
        sink->diagnose(Diagnostics::DuplicateStructuralRayTracingGroupSlot{
            .section = section,
            .slot = Int64(value),
            .location = traceOperation->sourceLoc});
    }
}

void synthesizePortableStructuralRayTracingEntryPoints(
    IRModule* module,
    List<IRFunc*>& ioEntryPoints,
    DiagnosticSink* sink)
{
    List<IRInst*> traceOperations;
    _collectTraceOperations(module->getModuleInst(), traceOperations);
    List<StructuralRayTracingGeneratedEntryPoint> generated;

    for (auto entryPoint : ioEntryPoints)
    {
        if (auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        {
            generated.add(
                {StructuralRayTracingStageKind(info->getStageKind()->getValue()),
                 as<IRFunc>(info->getInvoke()),
                 entryPoint});
        }
    }

    for (auto operation : traceOperations)
    {
        HashSet<IRIntegerValue> hitSlots;
        HashSet<IRIntegerValue> missSlots;
        HashSet<IRIntegerValue> callableSlots;
        for (auto decoration : operation->getDecorations())
        {
            if (auto group = as<IRStructuralRayTracingHitGroupInfoDecoration>(decoration))
            {
                _validateStructuralRayTracingGroupSlot(
                    operation,
                    group->getSlotIndex(),
                    "hit",
                    hitSlots,
                    sink);
                auto hitAttributesKind = StructuralRayTracingHitAttributesKind(
                    group->getHitAttributesKind()->getValue());
                if (hitAttributesKind == StructuralRayTracingHitAttributesKind::Curve)
                {
                    sink->diagnose(Diagnostics::StructuralRayTracingCurveRequiresMetal{
                        .location = operation->sourceLoc});
                    continue;
                }
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::ClosestHit,
                    group->getClosestHitType(),
                    group->getClosestHit(),
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getHitAttributesType(),
                    hitAttributesKind);
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::AnyHit,
                    group->getAnyHitType(),
                    group->getAnyHit(),
                    group->getContextType(),
                    group->getPayloadType(),
                    group->getHitAttributesType(),
                    hitAttributesKind);
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::Intersection,
                    group->getIntersectionType(),
                    group->getIntersection(),
                    group->getContextType());
            }
            else if (auto group = as<IRStructuralRayTracingMissGroupInfoDecoration>(decoration))
            {
                _validateStructuralRayTracingGroupSlot(
                    operation,
                    group->getSlotIndex(),
                    "miss",
                    missSlots,
                    sink);
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::Miss,
                    group->getMissType(),
                    group->getMiss(),
                    group->getContextType(),
                    group->getPayloadType());
            }
            else if (auto group = as<IRStructuralRayTracingCallableGroupInfoDecoration>(decoration))
            {
                _validateStructuralRayTracingGroupSlot(
                    operation,
                    group->getSlotIndex(),
                    "callable",
                    callableSlots,
                    sink);
                _generateStructuralRayTracingEntryPoint(
                    module,
                    ioEntryPoints,
                    generated,
                    StructuralRayTracingStageKind::Callable,
                    group->getCallableType(),
                    group->getCallable(),
                    group->getContextType(),
                    nullptr,
                    nullptr,
                    StructuralRayTracingHitAttributesKind::None,
                    group->getCallableDataType());
            }
        }
    }
}

struct StructuralRayTracingStageParameterThreader
{
    StructuralRayTracingStageParameterThreader(
        IRModule* module,
        IRType* parameterType,
        LayoutResourceKind resourceKind,
        const char* parameterName,
        const char* semanticName,
        bool isInput,
        bool isOutput,
        IRType* entryPointParameterType = nullptr,
        IRStructKey* entryPointValueKey = nullptr)
        : m_module(module)
        , m_parameterType(parameterType)
        , m_resourceKind(resourceKind)
        , m_parameterName(parameterName)
        , m_semanticName(semanticName)
        , m_isInput(isInput)
        , m_isOutput(isOutput)
        , m_entryPointParameterType(entryPointParameterType)
        , m_entryPointValueKey(entryPointValueKey)
    {
    }

    IRModule* m_module;
    IRType* m_parameterType;
    LayoutResourceKind m_resourceKind;
    const char* m_parameterName;
    const char* m_semanticName;
    bool m_isInput;
    bool m_isOutput;
    IRType* m_entryPointParameterType;
    IRStructKey* m_entryPointValueKey;
    Dictionary<IRFunc*, IRInst*> m_parameters;

    void registerParameter(IRFunc* func, IRInst* parameter) { m_parameters[func] = parameter; }

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
        if (auto found = m_parameters.tryGetValue(func))
            return *found;

        auto firstBlock = func->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        IRBuilder builder(m_module);
        auto entryPointDecoration = func->findDecoration<IREntryPointDecoration>();
        auto physicalParameterType = entryPointDecoration && m_entryPointParameterType
                                         ? m_entryPointParameterType
                                         : m_parameterType;
        auto parameter = builder.createParam(physicalParameterType);
        builder.addNameHintDecoration(parameter, UnownedTerminatedStringSlice(m_parameterName));
        parameter->insertBefore(firstBlock->getFirstOrdinaryInst());

        IRInst* parameterValue = parameter;
        if (entryPointDecoration && m_entryPointValueKey)
        {
            builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
            parameterValue =
                builder.emitFieldExtract(m_parameterType, parameter, m_entryPointValueKey);
        }
        m_parameters.add(func, parameterValue);

        if (entryPointDecoration)
        {
            if (m_isInput)
                builder.addSimpleDecoration<IRGlobalInputDecoration>(parameter);
            if (m_isOutput)
                builder.addSimpleDecoration<IRGlobalOutputDecoration>(parameter);
            if (m_semanticName)
            {
                builder.addSemanticDecoration(
                    parameter,
                    UnownedTerminatedStringSlice(m_semanticName));
            }

            IRTypeLayout::Builder typeLayoutBuilder(&builder);
            typeLayoutBuilder.addResourceUsage(m_resourceKind, LayoutSize(1));
            IRVarLayout::Builder varLayoutBuilder(&builder, typeLayoutBuilder.build());
            varLayoutBuilder.findOrAddResourceInfo(m_resourceKind);
            varLayoutBuilder.setStage(entryPointDecoration->getProfile().getStage());
            builder.addLayoutDecoration(parameter, varLayoutBuilder.build());
        }

        fixUpFuncType(func);

        // Register the parameter before rewriting callers so recursive call graphs terminate.
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

        return parameterValue;
    }

    void lower(IRInst* operation)
    {
        auto parameter = findOrCreateParameter(operation);
        operation->replaceUsesWith(parameter);
    }
};

static void _collectStageInputOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectStageInputOperations(child, operations);
        if (as<IRStructuralRayTracingStageInputOperation>(child))
            operations.add(child);
    }
}

static void _collectStructuralEntryPoints(IRModule* module, List<IRFunc*>& entryPoints)
{
    for (auto child = module->getModuleInst()->getFirstChild(); child; child = child->getNextInst())
    {
        if (auto func = as<IRFunc>(child))
        {
            if (func->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
                entryPoints.add(func);
        }
    }
}

void lowerMetalStructuralRayTracingStageInputOperations(
    IRModule* module,
    const Dictionary<IRFunc*, IRInst*>& entryPointPayloadValues)
{
    List<IRInst*> operations;
    _collectStageInputOperations(module->getModuleInst(), operations);
    List<IRFunc*> structuralEntryPoints;
    _collectStructuralEntryPoints(module, structuralEntryPoints);

    HashSet<IRType*> payloadTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        if (!entryPoint->findDecoration<IRMetalVisibleFunctionDecoration>() &&
            !entryPoint->findDecoration<IRMetalIntersectionFunctionDecoration>())
            continue;
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        if (info && !as<IRVoidType>(info->getPayloadType()))
            payloadTypes.add(info->getPayloadType());
    }

    HashSet<IRInst*> loweredOperations;
    for (auto payloadType : payloadTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getPtrType(payloadType, AddressSpace::ThreadLocal),
            LayoutResourceKind::RayPayload,
            "payload",
            nullptr,
            false,
            false);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (!info || info->getPayloadType() != payloadType)
                continue;
            if (auto payloadValue = entryPointPayloadValues.tryGetValue(entryPoint))
                threader.registerParameter(entryPoint, *payloadValue);
        }
        for (auto operation : operations)
        {
            if (operation->getOp() != kIROp_StructuralRayTracingGetPayload)
                continue;
            auto payloadPtrType = as<IRPtrTypeBase>(operation->getDataType());
            SLANG_ASSERT(payloadPtrType);
            if (payloadPtrType->getValueType() == payloadType)
            {
                threader.lower(operation);
                loweredOperations.add(operation);
            }
        }
    }

    for (auto operation : loweredOperations)
        operation->removeAndDeallocate();
    loweredOperations.clear();

    // Generated Metal adapters consume hit attributes from their native parameters or ray-data
    // state before reaching this point. A selected source-stage implementation remains exported as
    // an ordinary helper, though, so thread any attributes left in that helper graph through an
    // ordinary parameter. This keeps compiler-owned aggregate operations out of general type
    // legalization without assigning a native Metal stage ABI to the source helper itself.
    HashSet<IRType*> remainingHitAttributeTypes;
    operations.clear();
    _collectStageInputOperations(module->getModuleInst(), operations);
    for (auto operation : operations)
    {
        if (operation->getOp() == kIROp_StructuralRayTracingGetHitAttributes)
            remainingHitAttributeTypes.add(operation->getDataType());
    }

    for (auto attributeType : remainingHitAttributeTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getPtrType(attributeType, AddressSpace::ThreadLocal),
            LayoutResourceKind::HitAttributes,
            "attributes",
            nullptr,
            false,
            false);
        for (auto operation : operations)
        {
            if (operation->getOp() == kIROp_StructuralRayTracingGetHitAttributes &&
                operation->getDataType() == attributeType)
            {
                builder.setInsertBefore(operation);
                auto attributes = builder.emitLoad(threader.findOrCreateParameter(operation));
                operation->replaceUsesWith(attributes);
                loweredOperations.add(operation);
            }
        }
    }

    for (auto operation : loweredOperations)
        operation->removeAndDeallocate();

    operations.clear();
    _collectStageInputOperations(module->getModuleInst(), operations);
    IRBuilder builder(module);
    for (auto operation : operations)
    {
        auto stageInputOperation = cast<IRStructuralRayTracingStageInputOperation>(operation);
        if (!stageInputOperation->hasFallback())
            continue;

        builder.setInsertBefore(operation);
        List<IRInst*> arguments;
        for (UInt i = 1; i < operation->getOperandCount(); ++i)
            arguments.add(operation->getOperand(i));
        auto call = builder.emitCallInst(
            operation->getDataType(),
            stageInputOperation->getFallback(),
            arguments.getCount(),
            arguments.getBuffer());
        operation->replaceUsesWith(call);
        operation->removeAndDeallocate();
    }
}

static StructuralRayTracingHitAttributesKind _getHitAttributesKind(
    IRStructuralRayTracingEntryPointInfoDecoration* info)
{
    return StructuralRayTracingHitAttributesKind(info->getHitAttributesKind()->getValue());
}

void lowerPortableStructuralRayTracingStageInputOperations(IRModule* module)
{
    List<IRInst*> operations;
    _collectStageInputOperations(module->getModuleInst(), operations);
    List<IRFunc*> structuralEntryPoints;
    _collectStructuralEntryPoints(module, structuralEntryPoints);

    HashSet<IRType*> loweredPayloadTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        auto payloadType = info->getPayloadType();
        if (!as<IRVoidType>(payloadType))
            loweredPayloadTypes.add(payloadType);
    }
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetPayload)
            continue;

        auto payloadPtrType = as<IRPtrTypeBase>(operation->getDataType());
        SLANG_ASSERT(payloadPtrType);
        auto payloadType = payloadPtrType->getValueType();
        loweredPayloadTypes.add(payloadType);
    }

    for (auto payloadType : loweredPayloadTypes)
    {
        IRBuilder builder(module);
        if (!payloadType->findDecoration<IRRayPayloadDecoration>())
            builder.addRayPayloadDecoration(payloadType);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getBorrowInOutParamType(payloadType),
            LayoutResourceKind::RayPayload,
            "payload",
            "SV_RayPayload",
            true,
            true);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (info->getPayloadType() == payloadType)
                threader.findOrCreateParameter(entryPoint);
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() != kIROp_StructuralRayTracingGetPayload)
                continue;
            auto candidatePtrType = as<IRPtrTypeBase>(candidate->getDataType());
            SLANG_ASSERT(candidatePtrType);
            if (candidatePtrType->getValueType() == payloadType)
                threader.lower(candidate);
        }
    }

    HashSet<IRType*> loweredCallableDataTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        auto callableDataType = info->getCallableDataType();
        if (!as<IRVoidType>(callableDataType))
            loweredCallableDataTypes.add(callableDataType);
    }
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetCallableData)
            continue;

        auto callableDataPtrType = as<IRPtrTypeBase>(operation->getDataType());
        SLANG_ASSERT(callableDataPtrType);
        loweredCallableDataTypes.add(callableDataPtrType->getValueType());
    }

    for (auto callableDataType : loweredCallableDataTypes)
    {
        IRBuilder builder(module);
        StructuralRayTracingStageParameterThreader threader(
            module,
            builder.getBorrowInOutParamType(callableDataType),
            LayoutResourceKind::CallablePayload,
            "data",
            nullptr,
            true,
            true);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (info->getCallableDataType() == callableDataType)
                threader.findOrCreateParameter(entryPoint);
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() != kIROp_StructuralRayTracingGetCallableData)
                continue;
            auto candidatePtrType = as<IRPtrTypeBase>(candidate->getDataType());
            SLANG_ASSERT(candidatePtrType);
            if (candidatePtrType->getValueType() == callableDataType)
                threader.lower(candidate);
        }
    }

    HashSet<IRType*> loweredHitAttributeTypes;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        if (_getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Custom)
            loweredHitAttributeTypes.add(info->getHitAttributesType());
    }
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetHitAttributes)
            continue;

        auto attributeType = operation->getDataType();
        loweredHitAttributeTypes.add(attributeType);
    }

    for (auto attributeType : loweredHitAttributeTypes)
    {
        StructuralRayTracingStageParameterThreader threader(
            module,
            attributeType,
            LayoutResourceKind::HitAttributes,
            "attributes",
            "SV_IntersectionAttributes",
            true,
            false);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (_getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Custom &&
                info->getHitAttributesType() == attributeType)
            {
                threader.findOrCreateParameter(entryPoint);
            }
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() == kIROp_StructuralRayTracingGetHitAttributes &&
                candidate->getDataType() == attributeType)
            {
                threader.lower(candidate);
            }
        }
    }

    bool needsTriangleHitAttributes = false;
    for (auto entryPoint : structuralEntryPoints)
    {
        auto info = entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
        if (_getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Triangle)
            needsTriangleHitAttributes = true;
    }
    for (auto operation : operations)
    {
        if (operation->getOp() == kIROp_StructuralRayTracingGetTriangleBarycentricCoord)
            needsTriangleHitAttributes = true;
    }

    if (needsTriangleHitAttributes)
    {
        IRBuilder builder(module);
        auto barycentricType = builder.getVectorType(builder.getFloatType(), 2);
        auto nativeAttributeType = builder.createStructType();
        builder.addNameHintDecoration(
            nativeAttributeType,
            UnownedTerminatedStringSlice("StructuralTriangleHitAttributes"));
        auto barycentricKey = builder.createStructKey();
        builder.addNameHintDecoration(barycentricKey, UnownedTerminatedStringSlice("barycentrics"));
        builder.createStructField(nativeAttributeType, barycentricKey, barycentricType);

        StructuralRayTracingStageParameterThreader threader(
            module,
            barycentricType,
            LayoutResourceKind::HitAttributes,
            "attributes",
            "SV_IntersectionAttributes",
            true,
            false,
            nativeAttributeType,
            barycentricKey);
        for (auto entryPoint : structuralEntryPoints)
        {
            auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>();
            if (_getHitAttributesKind(info) == StructuralRayTracingHitAttributesKind::Triangle)
                threader.findOrCreateParameter(entryPoint);
        }
        for (auto candidate : operations)
        {
            if (candidate->getOp() == kIROp_StructuralRayTracingGetTriangleBarycentricCoord &&
                candidate->getDataType() == barycentricType)
            {
                threader.lower(candidate);
            }
        }
    }

    IRBuilder builder(module);
    for (auto operation : operations)
    {
        auto stageInputOperation = cast<IRStructuralRayTracingStageInputOperation>(operation);
        if (!stageInputOperation->hasFallback())
            continue;

        builder.setInsertBefore(operation);

        List<IRInst*> arguments;
        for (UInt i = 1; i < operation->getOperandCount(); ++i)
            arguments.add(operation->getOperand(i));

        auto call = builder.emitCallInst(
            operation->getDataType(),
            stageInputOperation->getFallback(),
            arguments.getCount(),
            arguments.getBuffer());
        operation->replaceUsesWith(call);
        operation->removeAndDeallocate();
    }

    for (auto operation : operations)
    {
        auto stageInputOperation = cast<IRStructuralRayTracingStageInputOperation>(operation);
        if (!stageInputOperation->hasFallback())
            operation->removeAndDeallocate();
    }

    for (auto entryPoint : structuralEntryPoints)
    {
        if (auto info =
                entryPoint->findDecoration<IRStructuralRayTracingEntryPointInfoDecoration>())
        {
            info->removeAndDeallocate();
        }
    }
}

static void _collectTraceOperations(IRInst* parent, List<IRInst*>& operations)
{
    for (auto child = parent->getFirstChild(); child; child = child->getNextInst())
    {
        _collectTraceOperations(child, operations);
        if (child->getOp() == kIROp_StructuralRayTracingTrace)
            operations.add(child);
    }
}

void lowerPortableStructuralRayTracingTraceOperations(IRModule* module)
{
    List<IRInst*> operations;
    _collectTraceOperations(module->getModuleInst(), operations);

    IRBuilder builder(module);
    for (auto operation : operations)
    {
        auto traceOperation = cast<IRStructuralRayTracingTrace>(operation);
        builder.setInsertBefore(operation);

        IRInst* arguments[] = {
            traceOperation->getTracer(),
            traceOperation->getDesc(),
            traceOperation->getAccelerationStructure(),
            traceOperation->getDescriptor(),
            traceOperation->getPayload(),
        };

        auto call = builder.emitCallInst(
            traceOperation->getDataType(),
            traceOperation->getFallback(),
            SLANG_COUNT_OF(arguments),
            arguments);
        operation->replaceUsesWith(call);
        operation->removeAndDeallocate();
    }
}

} // namespace Slang
