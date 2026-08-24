#include "slang-ir-synthesize-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

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
        bool isOutput)
        : m_module(module)
        , m_parameterType(parameterType)
        , m_resourceKind(resourceKind)
        , m_parameterName(parameterName)
        , m_semanticName(semanticName)
        , m_isInput(isInput)
        , m_isOutput(isOutput)
    {
    }

    IRModule* m_module;
    IRType* m_parameterType;
    LayoutResourceKind m_resourceKind;
    const char* m_parameterName;
    const char* m_semanticName;
    bool m_isInput;
    bool m_isOutput;
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
        auto parameter = builder.createParam(m_parameterType);
        builder.addNameHintDecoration(parameter, UnownedTerminatedStringSlice(m_parameterName));
        parameter->insertBefore(firstBlock->getFirstOrdinaryInst());
        m_parameters.add(func, parameter);

        if (func->findDecoration<IREntryPointDecoration>())
        {
            auto entryPointDecoration = func->findDecoration<IREntryPointDecoration>();
            if (m_isInput)
                builder.addSimpleDecoration<IRGlobalInputDecoration>(parameter);
            if (m_isOutput)
                builder.addSimpleDecoration<IRGlobalOutputDecoration>(parameter);
            builder.addSemanticDecoration(parameter, UnownedTerminatedStringSlice(m_semanticName));

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

        return parameter;
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

void lowerPortableStructuralRayTracingStageInputOperations(IRModule* module)
{
    List<IRInst*> operations;
    _collectStageInputOperations(module->getModuleInst(), operations);

    HashSet<IRType*> loweredPayloadTypes;
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetPayload)
            continue;

        auto payloadPtrType = as<IRPtrTypeBase>(operation->getDataType());
        SLANG_ASSERT(payloadPtrType);
        auto payloadType = payloadPtrType->getValueType();
        if (!loweredPayloadTypes.add(payloadType))
            continue;

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

    HashSet<IRType*> loweredHitAttributeTypes;
    for (auto operation : operations)
    {
        if (operation->getOp() != kIROp_StructuralRayTracingGetHitAttributes)
            continue;

        auto attributeType = operation->getDataType();
        if (!loweredHitAttributeTypes.add(attributeType))
            continue;

        StructuralRayTracingStageParameterThreader threader(
            module,
            attributeType,
            LayoutResourceKind::HitAttributes,
            "attributes",
            "SV_IntersectionAttributes",
            true,
            false);
        for (auto candidate : operations)
        {
            if (candidate->getOp() == kIROp_StructuralRayTracingGetHitAttributes &&
                candidate->getDataType() == attributeType)
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
        builder.setInsertBefore(operation);

        List<IRInst*> arguments;
        for (UInt i = 1; i < operation->getOperandCount(); ++i)
            arguments.add(operation->getOperand(i));

        auto call = builder.emitCallInst(
            operation->getDataType(),
            operation->getOperand(0),
            arguments.getCount(),
            arguments.getBuffer());
        operation->replaceUsesWith(call);
        operation->removeAndDeallocate();
    }
}

} // namespace Slang
