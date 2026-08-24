#include "slang-ir-synthesize-structural-ray-tracing.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct StructuralRayTracingPayloadThreader
{
    StructuralRayTracingPayloadThreader(IRModule* module, IRType* payloadType)
        : m_module(module)
    {
        IRBuilder builder(module);
        m_payloadParamType = builder.getBorrowInOutParamType(payloadType);
        if (!payloadType->findDecoration<IRRayPayloadDecoration>())
            builder.addRayPayloadDecoration(payloadType);
    }

    IRModule* m_module;
    IRType* m_payloadParamType;
    Dictionary<IRFunc*, IRInst*> m_payloadParams;

    IRFunc* findEnclosingFunc(IRInst* inst)
    {
        for (auto parent = inst; parent; parent = parent->getParent())
        {
            if (auto func = as<IRFunc>(parent))
                return func;
        }
        return nullptr;
    }

    IRInst* findOrCreatePayloadParam(IRInst* inst)
    {
        auto func = findEnclosingFunc(inst);
        SLANG_ASSERT(func);
        return findOrCreatePayloadParam(func);
    }

    IRInst* findOrCreatePayloadParam(IRFunc* func)
    {
        if (auto found = m_payloadParams.tryGetValue(func))
            return *found;

        auto firstBlock = func->getFirstBlock();
        SLANG_ASSERT(firstBlock);

        IRBuilder builder(m_module);
        auto payloadParam = builder.createParam(m_payloadParamType);
        builder.addNameHintDecoration(payloadParam, UnownedTerminatedStringSlice("payload"));
        payloadParam->insertBefore(firstBlock->getFirstOrdinaryInst());
        m_payloadParams.add(func, payloadParam);

        if (func->findDecoration<IREntryPointDecoration>())
        {
            auto entryPointDecoration = func->findDecoration<IREntryPointDecoration>();
            builder.addSimpleDecoration<IRGlobalInputDecoration>(payloadParam);
            builder.addSimpleDecoration<IRGlobalOutputDecoration>(payloadParam);
            builder.addSemanticDecoration(
                payloadParam,
                UnownedTerminatedStringSlice("SV_RayPayload"));

            IRTypeLayout::Builder typeLayoutBuilder(&builder);
            typeLayoutBuilder.addResourceUsage(LayoutResourceKind::RayPayload, LayoutSize(1));
            IRVarLayout::Builder varLayoutBuilder(&builder, typeLayoutBuilder.build());
            varLayoutBuilder.findOrAddResourceInfo(LayoutResourceKind::RayPayload);
            varLayoutBuilder.setStage(entryPointDecoration->getProfile().getStage());
            builder.addLayoutDecoration(payloadParam, varLayoutBuilder.build());
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
            args.add(findOrCreatePayloadParam(call));

            builder.setInsertBefore(call);
            auto newCall = builder.emitCallInst(
                call->getDataType(),
                call->getCallee(),
                args.getCount(),
                args.getBuffer());
            call->replaceUsesWith(newCall);
            call->removeAndDeallocate();
        }

        return payloadParam;
    }

    void lower(IRInst* operation)
    {
        auto payloadParam = findOrCreatePayloadParam(operation);
        operation->replaceUsesWith(payloadParam);
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

        StructuralRayTracingPayloadThreader threader(module, payloadType);
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

    IRBuilder builder(module);
    for (auto operation : operations)
    {
        if (operation->getOp() == kIROp_StructuralRayTracingGetPayload)
            continue;

        auto stageInputOperation = cast<IRStructuralRayTracingStageInputOperation>(operation);
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
        if (operation->getOp() == kIROp_StructuralRayTracingGetPayload)
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
