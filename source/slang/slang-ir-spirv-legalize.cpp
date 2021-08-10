// slang-ir-spirv-legalize.cpp
#include "slang-ir-spirv-legalize.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-emit-base.h"
#include "slang-glsl-extension-tracker.h"

namespace Slang
{

//
// Legalization of IR for direct SPIRV emit.
//

struct StorageClassPropagationContext : public SourceEmitterBase
{
    SPIRVEmitSharedContext* m_sharedContext;

    IRModule* m_module;
    // We will use a single work list of instructions that need
    // to be considered for specialization or simplification,
    // whether generic, existential, etc.
    //
    OrderedHashSet<IRInst*> workList;

    void addToWorkList(IRInst* inst)
    {
        if (workList.Add(inst))
        {
            addUsersToWorkList(inst);
        }
    }

    void addUsersToWorkList(IRInst* inst)
    {
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();

            addToWorkList(user);
        }
    }

    StorageClassPropagationContext(SPIRVEmitSharedContext* sharedContext, IRModule* module)
        : m_sharedContext(sharedContext), m_module(module)
    {
    }

    void processGlobalParam(IRGlobalParam* inst) { processGlobalVar(inst); }

    void processGlobalVar(IRInst* inst)
    {
        auto oldPtrType = as<IRPtrTypeBase>(inst->getDataType());
        if (!oldPtrType)
            return;

        // If the pointer type is already qualified with address spaces (such as
        // lowered pointer type from a `HLSLStructuredBufferType`), make no
        // further modifications.
        if (oldPtrType->hasAddressSpace())
        {
            addUsersToWorkList(inst);
            return;
        }

        auto varLayout = getVarLayout(inst);
        if (!varLayout)
            return;

        SpvStorageClass storageClass = SpvStorageClassPrivate;
        for (auto rr : varLayout->getOffsetAttrs())
        {
            switch (rr->getResourceKind())
            {
            case LayoutResourceKind::Uniform:
            case LayoutResourceKind::ShaderResource:
            case LayoutResourceKind::DescriptorTableSlot:
                storageClass = SpvStorageClassUniform;
                break;
            case LayoutResourceKind::VaryingInput:
                storageClass = SpvStorageClassInput;
                break;
            case LayoutResourceKind::VaryingOutput:
                storageClass = SpvStorageClassOutput;
                break;
            case LayoutResourceKind::UnorderedAccess:
                storageClass = SpvStorageClassStorageBuffer;
                break;
            case LayoutResourceKind::PushConstantBuffer:
                storageClass = SpvStorageClassPushConstant;
                break;
            default:
                break;
            }
        }
        auto rate = inst->getRate();
        if (as<IRGroupSharedRate>(rate))
        {
            storageClass = SpvStorageClassWorkgroup;
        }
        IRBuilder builder;
        builder.sharedBuilder = &m_sharedContext->m_sharedIRBuilder;
        builder.setInsertBefore(inst);
        auto newPtrType =
            builder.getPtrType(oldPtrType->getOp(), oldPtrType->getValueType(), storageClass);
        inst->setFullType(newPtrType);
        addUsersToWorkList(inst);
        return;
    }

    void processCall(IRCall* inst)
    {
        auto funcValue = inst->getOperand(0);
        if (auto targetIntrinsic = Slang::findBestTargetIntrinsicDecoration(
                funcValue, m_sharedContext->m_targetRequest->getTargetCaps()))
        {
            SpvSnippet* snippet = m_sharedContext->getParsedSpvSnippet(targetIntrinsic);
            if (!snippet)
                return;
            if (snippet->resultStorageClass != SpvStorageClassMax)
            {
                auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
                if (!ptrType)
                    return;
                IRBuilder builder;
                builder.sharedBuilder = &m_sharedContext->m_sharedIRBuilder;
                builder.setInsertBefore(inst);
                auto qualPtrType = builder.getPtrType(
                    ptrType->getOp(), ptrType->getValueType(), snippet->resultStorageClass);
                List<IRInst*> args;
                for (UInt i = 0; i < inst->getArgCount(); i++)
                    args.add(inst->getArg(i));
                auto newCall = builder.emitCallInst(qualPtrType, funcValue, args);
                inst->replaceUsesWith(newCall);
                inst->removeAndDeallocate();
                addUsersToWorkList(newCall);
            }
        }
    }

    void processGetElementPtr(IRGetElementPtr* inst)
    {
        if (auto ptrType = as<IRPtrTypeBase>(inst->getBase()->getDataType()))
        {
            if (!ptrType->hasAddressSpace())
                return;
            auto oldResultType = as<IRPtrTypeBase>(inst->getDataType());
            if (oldResultType->getAddressSpace() != ptrType->getAddressSpace())
            {
                IRBuilder builder;
                builder.sharedBuilder = &m_sharedContext->m_sharedIRBuilder;
                builder.setInsertBefore(inst);
                auto newPtrType = builder.getPtrType(
                    oldResultType->getOp(),
                    oldResultType->getValueType(),
                    ptrType->getAddressSpace());
                auto newInst =
                    builder.emitElementAddress(newPtrType, inst->getBase(), inst->getIndex());
                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                addUsersToWorkList(newInst);
            }
        }
    }

    void processFieldAddress(IRFieldAddress* inst)
    {
        if (auto ptrType = as<IRPtrTypeBase>(inst->getBase()->getDataType()))
        {
            if (!ptrType->hasAddressSpace())
                return;
            auto oldResultType = as<IRPtrTypeBase>(inst->getDataType());
            if (oldResultType->getAddressSpace() != ptrType->getAddressSpace())
            {
                IRBuilder builder;
                builder.sharedBuilder = &m_sharedContext->m_sharedIRBuilder;
                builder.setInsertBefore(inst);
                auto newPtrType = builder.getPtrType(
                    oldResultType->getOp(),
                    oldResultType->getValueType(),
                    ptrType->getAddressSpace());
                auto newInst =
                    builder.emitFieldAddress(newPtrType, inst->getBase(), inst->getField());
                inst->replaceUsesWith(newInst);
                inst->removeAndDeallocate();
                addUsersToWorkList(newInst);
            }
        }
    }

    void processStructuredBufferType(IRHLSLStructuredBufferTypeBase* inst)
    {
        IRBuilder builder;
        builder.sharedBuilder = &m_sharedContext->m_sharedIRBuilder;
        builder.setInsertBefore(inst);
        auto arrayType = builder.getUnsizedArrayType(inst->getElementType());
        auto ptrType = builder.getPtrType(kIROp_PtrType, arrayType, SpvStorageClassStorageBuffer);
        inst->replaceUsesWith(ptrType);
        inst->removeAndDeallocate();
        addUsersToWorkList(ptrType);
    }

    void propagate()
    {
        addToWorkList(m_module->getModuleInst());
        while (workList.Count() != 0)
        {
            IRInst* inst = workList.getLast();
            workList.removeLast();
            switch (inst->getOp())
            {
            case kIROp_GlobalParam:
                processGlobalParam(as<IRGlobalParam>(inst));
                break;
            case kIROp_GlobalVar:
                processGlobalVar(as<IRGlobalVar>(inst));
                break;
            case kIROp_Call:
                processCall(as<IRCall>(inst));
                break;
            case kIROp_getElementPtr:
                processGetElementPtr(as<IRGetElementPtr>(inst));
                break;
            case kIROp_FieldAddress:
                processFieldAddress(as<IRFieldAddress>(inst));
                break;
            case kIROp_HLSLStructuredBufferType:
            case kIROp_HLSLRWStructuredBufferType:
                processStructuredBufferType(as<IRHLSLStructuredBufferTypeBase>(inst));
                break;
            default:
                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    addToWorkList(child);
                }
                break;
            }
        }
    }
};

void propagateStorageClass(SPIRVEmitSharedContext* sharedContext, IRModule* module)
{
    StorageClassPropagationContext context(sharedContext, module);
    context.propagate();
}

void legalizeIRForSPIRV(
    SPIRVEmitSharedContext* context,
    IRModule* module,
    DiagnosticSink* sink)
{
    SLANG_UNUSED(sink);
    propagateStorageClass(context, module);
}

} // namespace Slang
