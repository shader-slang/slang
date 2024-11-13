#include "slang-ir-defer-buffer-load.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-insts.h"
#include "slang-ir-redundancy-removal.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct DeferBufferLoadContext
{
    struct AccessChain
    {
        List<IRInst*> chain;
        mutable HashCode64 hash = 0;

        bool operator==(const AccessChain& rhs) const
        {
            ensureHash();
            rhs.ensureHash();
            if (hash != rhs.hash)
                return false;
            if (chain.getCount() != rhs.chain.getCount())
                return false;
            for (Index i = 0; i < chain.getCount(); i++)
            {
                if (chain[i] != rhs.chain[i])
                    return false;
            }
            return true;
        }
        void ensureHash() const
        {
            if (hash == 0)
            {
                for (auto inst : chain)
                {
                    hash = combineHash(hash, Slang::getHashCode(inst));
                }
            }
        }
        HashCode64 getHashCode() const
        {
            ensureHash();
            return hash;
        }
    };

    // Map an original SSA value to a pointer that can be used to load the value.
    Dictionary<AccessChain, IRInst*> mapAccessChainToPtr;
    Dictionary<IRInst*, IRInst*> mapValueToPtr;
    // Map an ptr to its loaded value.
    Dictionary<IRInst*, IRInst*> mapPtrToValue;

    IRFunc* currentFunc = nullptr;
    IRDominatorTree* dominatorTree = nullptr;

    // Find the block that is dominated by all dependent blocks, and is the earliest block that
    // dominates the target block.
    // This is the place where we can insert the load instruction such that all access chain
    // operands are defined and the load can be made avaialble to the location of valueInst.
    //
    IRBlock* findEarliestDominatingBlock(IRInst* valueInst, List<IRBlock*>& dependentBlocks)
    {
        auto targetBlock = getBlock(valueInst);
        while (targetBlock)
        {
            auto idom = dominatorTree->getImmediateDominator(targetBlock);
            if (!idom)
                break;
            bool isValid = true;
            for (auto block : dependentBlocks)
            {
                if (!dominatorTree->dominates(block, idom))
                {
                    isValid = false;
                    break;
                }
            }
            if (isValid)
            {
                targetBlock = idom;
            }
            else
            {
                break;
            }
        }
        return targetBlock;
    }

    // Find the earliest instruction before which we can insert the load instruction such that
    // all dependent instructions for the load address are defined, and the load can reach all
    // locations where the address is available.
    //
    IRInst* findEarliestInsertionPoint(IRInst* valueInst, AccessChain& chain)
    {
        List<IRBlock*> dependentBlocks;
        List<IRInst*> dependentInsts;
        for (auto inst : chain.chain)
        {
            if (auto block = getBlock(inst))
            {
                dependentBlocks.add(block);
                dependentInsts.add(inst);
            }
        }
        auto targetBlock = findEarliestDominatingBlock(valueInst, dependentBlocks);
        IRInst* insertBeforeInst =
            targetBlock == getBlock(valueInst) ? valueInst : targetBlock->getTerminator();
        for (;;)
        {
            auto prev = insertBeforeInst->getPrevInst();
            if (!prev)
                break;
            bool valid = true;
            for (auto inst : dependentInsts)
            {
                if (!dominatorTree->dominates(inst, prev) || inst == prev)
                {
                    valid = false;
                    break;
                }
            }
            if (valid)
            {
                insertBeforeInst = prev;
            }
            else
            {
                break;
            }
        }
        return insertBeforeInst;
    }

    // Ensure that for an original SSA value, we have formed a pointer that can be used to load the
    // value.
    IRInst* ensurePtr(IRInst* valueInst)
    {
        IRInst* result = nullptr;
        if (mapValueToPtr.tryGetValue(valueInst, result))
            return result;
        AccessChain chain;
        IRInst* current = valueInst;
        while (current)
        {
            bool processed = false;
            switch (current->getOp())
            {
            case kIROp_GetElement:
            case kIROp_FieldExtract:
                chain.chain.add(current->getOperand(1));
                current = current->getOperand(0);
                processed = true;
                break;
            default:
                break;
            }
            if (!processed)
                break;
        }
        chain.chain.add(current);
        chain.chain.reverse();
        if (mapAccessChainToPtr.tryGetValue(chain, result))
            return result;

        // Find the proper place to insert the load instruction.
        // This is the location where all operands of the access chain are defined.
        // And is the earliest block so all possible uses of the value at access chain
        // can be reached.
        IRBuilder b(valueInst);

        auto insertBeforeInst = findEarliestInsertionPoint(valueInst, chain);
        b.setInsertBefore(insertBeforeInst);

        switch (valueInst->getOp())
        {
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
            {
                result = b.emitRWStructuredBufferGetElementPtr(
                    valueInst->getOperand(0),
                    valueInst->getOperand(1));
                break;
            }
        case kIROp_GetElement:
            {
                auto ptr = ensurePtr(valueInst->getOperand(0));
                if (!ptr)
                    return nullptr;
                result = b.emitElementAddress(ptr, valueInst->getOperand(1));
                break;
            }
        case kIROp_FieldExtract:
            {
                auto ptr = ensurePtr(valueInst->getOperand(0));
                if (!ptr)
                    return nullptr;
                result = b.emitFieldAddress(ptr, valueInst->getOperand(1));
                break;
            }
        }
        if (result)
        {
            mapAccessChainToPtr[chain] = result;
            mapValueToPtr[valueInst] = result;
        }
        return result;
    }

    static bool isStructuredBufferLoad(IRInst* inst)
    {
        // Note: we cannot defer loads from RWStructuredBuffer because there can be other
        // instructions that modify the buffer.
        switch (inst->getOp())
        {
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
            return true;
        default:
            return false;
        }
    }

    // Ensure that for a pointer value, we have created a load instruction to materialize the value.
    IRInst* materializePointer(IRBuilder& builder, IRInst* loadInst)
    {
        auto ptr = ensurePtr(loadInst);
        if (!ptr)
            return nullptr;
        IRInst* result = nullptr;
        if (mapPtrToValue.tryGetValue(ptr, result))
            return result;
        builder.setInsertAfter(ptr);
        result = builder.emitLoad(ptr);
        mapPtrToValue[ptr] = result;
        return result;
    }

    static bool isSimpleType(IRInst* type)
    {
        if (as<IRBasicType>(type))
            return true;
        if (as<IRVectorType>(type))
            return true;
        if (as<IRMatrixType>(type))
            return true;
        return false;
    }

    void deferBufferLoadInst(IRBuilder& builder, List<IRInst*>& workList, IRInst* loadInst)
    {
        // Don't defer the load anymore if the type is simple.
        if (isSimpleType(loadInst->getDataType()))
        {
            if (!isStructuredBufferLoad(loadInst))
            {
                auto materializedVal = materializePointer(builder, loadInst);
                loadInst->replaceUsesWith(materializedVal);
            }
            return;
        }

        // Otherwise, look for all uses and try to defer the load before actual use of the value.
        ShortList<IRInst*> pendingWorkList;
        bool needMaterialize = false;
        traverseUses(
            loadInst,
            [&](IRUse* use)
            {
                if (needMaterialize)
                    return;

                auto user = use->getUser();
                switch (user->getOp())
                {
                case kIROp_GetElement:
                case kIROp_FieldExtract:
                    {
                        auto basePtr = ensurePtr(loadInst);
                        if (!basePtr)
                            return;
                        pendingWorkList.add(user);
                    }
                    break;
                default:
                    if (!isStructuredBufferLoad(loadInst))
                    {
                        needMaterialize = true;
                        return;
                    }
                    break;
                }
            });

        if (needMaterialize)
        {
            auto val = materializePointer(builder, loadInst);
            loadInst->replaceUsesWith(val);
            loadInst->removeAndDeallocate();
        }
        else
        {
            // Append to worklist in reverse order so we process the uses in natural appearance
            // order.
            for (Index i = pendingWorkList.getCount() - 1; i >= 0; i--)
                workList.add(pendingWorkList[i]);
        }
    }

    void deferBufferLoadInFunc(IRFunc* func)
    {
        removeRedundancyInFunc(func);

        currentFunc = func;
        dominatorTree = func->getModule()->findOrCreateDominatorTree(func);

        List<IRInst*> workList;

        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (isStructuredBufferLoad(inst))
                {
                    workList.add(inst);
                }
            }
        }

        IRBuilder builder(func);
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto inst = workList[i];
            deferBufferLoadInst(builder, workList, inst);
        }
    }

    void deferBufferLoad(IRGlobalValueWithCode* inst)
    {
        if (auto func = as<IRFunc>(inst))
        {
            deferBufferLoadInFunc(func);
        }
        else if (auto generic = as<IRGeneric>(inst))
        {
            auto inner = findGenericReturnVal(generic);
            if (auto innerFunc = as<IRFunc>(inner))
                deferBufferLoadInFunc(innerFunc);
        }
    }
};

void deferBufferLoad(IRModule* module)
{
    DeferBufferLoadContext context;
    for (auto childInst : module->getGlobalInsts())
    {
        if (auto code = as<IRGlobalValueWithCode>(childInst))
        {
            context.deferBufferLoad(code);
        }
    }
}

} // namespace Slang
