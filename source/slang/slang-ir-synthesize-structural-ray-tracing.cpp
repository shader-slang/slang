#include "slang-ir-synthesize-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-structural-ray-tracing.h"

namespace Slang
{

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

        IRInst* arguments[] =
        {
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
