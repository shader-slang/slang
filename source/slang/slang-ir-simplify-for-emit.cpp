#include "slang-ir-simplify-for-emit.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-util.h"

namespace Slang
{

struct SimplifyForEmitContext : public InstPassBase
{
    SimplifyForEmitContext(IRModule* inModule)
        : InstPassBase(inModule)
    {}

    List<IRInst*> followUpWorkList;
    HashSet<IRInst*> followUpWorkListSet;

    void addToFollowUpWorkList(IRInst* inst)
    {
        if (followUpWorkListSet.Add(inst))
            followUpWorkList.add(inst);
    }

    void processMakeStruct(IRInst* makeStruct)
    {
        auto structType = as<IRStructType>(makeStruct->getDataType());
        if (!structType)
            return;
        for (auto use = makeStruct->firstUse; use;)
        {
            auto nextUse = use->nextUse;
            auto user = use->getUser();
            if (auto store = as<IRStore>(user))
            {                
                IRBuilder builder(sharedBuilderStorage);
                builder.setInsertBefore(user);
                UInt i = 0;
                for (auto field : structType->getFields())
                {
                    auto fieldAddr = builder.emitFieldAddress(
                        builder.getPtrType(field->getFieldType()),
                        store->getPtr(),
                        field->getKey());
                    builder.emitStore(fieldAddr, makeStruct->getOperand(i));
                    addToFollowUpWorkList(makeStruct->getOperand(i));
                    i++;
                }
                store->removeAndDeallocate();
            }
            use = nextUse;
        }
        if (!makeStruct->hasUses())
            makeStruct->removeAndDeallocate();
    }

    void processMakeArray(IRInst* makeArray)
    {
        auto arrayType = as<IRArrayType>(makeArray->getDataType());
        if (!arrayType)
            return;

        for (auto use = makeArray->firstUse; use;)
        {
            auto nextUse = use->nextUse;
            auto user = use->getUser();
            if (auto store = as<IRStore>(user))
            {
                IRBuilder builder(sharedBuilderStorage);
                builder.setInsertBefore(user);
                for (UInt i = 0; i < makeArray->getOperandCount(); i++)
                {
                    auto elementAddr = builder.emitElementAddress(
                        builder.getPtrType(arrayType->getElementType()),
                        store->getPtr(),
                        builder.getIntValue(builder.getIntType(), (IRIntegerValue)i));
                    builder.emitStore(elementAddr, makeArray->getOperand(i));
                    addToFollowUpWorkList(makeArray->getOperand(i));
                }
                store->removeAndDeallocate();
            }
            use = nextUse;
        }
        if (!makeArray->hasUses())
            makeArray->removeAndDeallocate();
    }

    void processMakeArrayFromElement(IRInst* makeArray)
    {
        auto arrayType = as<IRArrayType>(makeArray->getDataType());
        if (!arrayType)
            return;
        auto arraySize = as<IRIntLit>(arrayType->getElementCount());
        if (!arraySize)
            return;

        for (auto use = makeArray->firstUse; use;)
        {
            auto nextUse = use->nextUse;
            auto user = use->getUser();
            if (auto store = as<IRStore>(user))
            {
                IRBuilder builder(sharedBuilderStorage);
                builder.setInsertBefore(user);
                for (IRIntegerValue i = 0; i < arraySize->getValue(); i++)
                {
                    auto elementAddr = builder.emitElementAddress(
                        builder.getPtrType(arrayType->getElementType()),
                        store->getPtr(),
                        builder.getIntValue(builder.getIntType(), i));
                    builder.emitStore(elementAddr, makeArray->getOperand(0));
                    addToFollowUpWorkList(makeArray->getOperand(0));
                }
                store->removeAndDeallocate();
            }
            use = nextUse;
        }
        if (!makeArray->hasUses())
            makeArray->removeAndDeallocate();
    }

    void processLoadUse(IRGlobalValueWithCode* func, IRLoad* load, IRUse* use)
    {
        auto user = use->getUser();
        if (user->getParent() != load->getParent())
            return;
        for (auto inst = load->getNextInst(); inst; inst = inst->getNextInst())
        {
            if (inst == user)
                break;
            if (canInstHaveSideEffectAtAddress(func, inst, load->getPtr()))
                return;
        }

        // If we reach here, it is OK to defer the load at use site.
        IRBuilder builder(sharedBuilderStorage);
        builder.setInsertBefore(user);
        auto newLoad = builder.emitLoad(load->getPtr());
        use->set(newLoad);
    }

    void processLoad(IRLoad* inst)
    {
        auto func = getParentFunc(inst);
        if (!func)
            return;

        for (auto use = inst->firstUse; use;)
        {
            auto nextUse = use->nextUse;
            processLoadUse(func, inst, use);
            use = nextUse;
        }

        if (!inst->hasUses())
            inst->removeAndDeallocate();
    }

    void processElementExtract(IRInst* inst)
    {
        // Create a duplicate for each use site.
        // This is safe because the result value of this inst should never
        // change regardless of where the inst is defined.
        // By creating the duplicates right before use sites, we will enable
        // the emit logic to always fold these insts.
        for (auto use = inst->firstUse; use;)
        {
            auto nextUse = use->nextUse;

            auto user = use->getUser();
            if (user->getPrevInst() == inst)
            {
                use = nextUse;
                continue;
            }

            IRBuilder builder(sharedBuilderStorage);
            builder.setInsertBefore(user);
            List<IRInst*> args;
            for (UInt i = 0; i < inst->getOperandCount(); i++)
                args.add(inst->getOperand(i));
            auto newInst = builder.emitIntrinsicInst(inst->getFullType(), inst->getOp(), inst->getOperandCount(), args.getBuffer());
            use->set(newInst);

            use = nextUse;
        }
        if (!inst->hasUses())
            inst->removeAndDeallocate();
    }

    void processVar(IRInst* var)
    {
        // Defer var to its first use, if the use is in the same basic block as the var.
        HashSet<IRInst*> userInSameBlock;
        for (auto use = var->firstUse; use; use = use->nextUse)
            if (use->getUser()->getParent() == var->getParent())
            {
                userInSameBlock.Add(use->getUser());
            }
        IRInst* firstUser = nullptr;
        for (auto inst = var->getNextInst(); inst; inst = inst->getNextInst())
        {
            if (userInSameBlock.Contains(inst))
            {
                firstUser = inst;
                break;
            }
        }
        if (!firstUser)
            return;
        var->insertBefore(firstUser);
    }

    void processInst(IRInst* inst)
    {
        // We inspect each inst and see if the following simplifications
        // can be applied:
        // 1. If we see `store(addr, MakeArray/Struct)`, we should turn them
        //    into direct stores into each element/field and remove the need
        //    to create a temporary for the `MakeArray/Struct` inst.
        // 2. If we see `load(addr)`, we duplicate the load right at each
        //    use site if it can be determined safe to do so. This allows
        //    emit logic to skip producing a temp var for the loaded result.
        switch (inst->getOp())
        {
        case kIROp_MakeStruct:
            processMakeStruct(inst);
            break;
        case kIROp_MakeArray:
            processMakeArray(inst);
            break;
        case kIROp_MakeArrayFromElement:
            processMakeArrayFromElement(inst);
            break;
        case kIROp_Load:
            processLoad(as<IRLoad>(inst));
            break;
        case kIROp_GetElement:
        case kIROp_FieldExtract:
            processElementExtract(inst);
            break;
        case kIROp_Var:
            processVar(inst);
            break;
        }
    }

    void eliminateCompositeConstruct(IRGlobalValueWithCode* func)
    {
        followUpWorkList.clear();
        followUpWorkListSet.Clear();

        for (auto block : func->getBlocks())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                switch (inst->getOp())
                {
                case kIROp_MakeStruct:
                case kIROp_MakeArray:
                case kIROp_MakeArrayFromElement:
                    addToFollowUpWorkList(inst);
                    break;
                }
            }
        }
        for (Index i = 0; i < followUpWorkList.getCount(); i++)
            processInst(followUpWorkList[i]);
    }

    void deferAndDuplicateLoad(IRGlobalValueWithCode* func)
    {
        followUpWorkList.clear();
        followUpWorkListSet.Clear();

        for (auto block : func->getBlocks())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                switch (inst->getOp())
                {
                case kIROp_Load:
                    addToFollowUpWorkList(inst);
                    break;
                }
            }
        }
        for (Index i = 0; i < followUpWorkList.getCount(); i++)
            processInst(followUpWorkList[i]);
    }

    void deferVarDecl(IRGlobalValueWithCode* func)
    {
        followUpWorkList.clear();
        followUpWorkListSet.Clear();

        for (auto block : func->getBlocks())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                switch (inst->getOp())
                {
                case kIROp_Var:
                    addToFollowUpWorkList(inst);
                    break;
                }
            }
        }
        for (Index i = 0; i < followUpWorkList.getCount(); i++)
            processInst(followUpWorkList[i]);
    }

    void deferAndDuplicateElementExtract(IRGlobalValueWithCode* func)
    {
        followUpWorkList.clear();
        followUpWorkListSet.Clear();

        for (auto block = func->getLastBlock(); block; block = block->getPrevBlock())
        {
            for (auto inst = block->getLastChild(); inst; inst = inst->getPrevInst())
            {
                switch (inst->getOp())
                {
                case kIROp_GetElement:
                case kIROp_FieldExtract:
                    addToFollowUpWorkList(inst);
                    break;
                }
            }
        }
        for (Index i = 0; i < followUpWorkList.getCount(); i++)
            processInst(followUpWorkList[i]);
    }

    void processFunc(IRGlobalValueWithCode* func)
    {
        eliminateCompositeConstruct(func);
        deferAndDuplicateElementExtract(func);
        deferAndDuplicateLoad(func);
        deferVarDecl(func);
    }

    void processModule()
    {
        sharedBuilderStorage.init(module);
        processInstsOfType<IRFunc>(kIROp_Func, [this](IRFunc* f) { processFunc(f); });
    }
};

void simplifyForEmit(IRModule* module)
{
    SimplifyForEmitContext context(module);
    context.processModule();
}

}
