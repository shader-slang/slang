#include "slang-ir-simplify-cfg.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

bool processFunc(IRFunc* func)
{
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return false;

    bool changed = false;

    List<IRBlock*> workList;
    HashSet<IRBlock*> processedBlock;
    workList.add(func->getFirstBlock());
    while (workList.getCount())
    {
        auto block = workList.getFirst();
        workList.fastRemoveAt(0);
        while (block)
        {
            // If `block` does not end with an unconditional branch, bail.
            if (block->getTerminator()->getOp() != kIROp_unconditionalBranch)
                break;
            auto branch = as<IRUnconditionalBranch>(block->getTerminator());
            auto successor = branch->getTargetBlock();
            // Only perform the merge if `block` is the only predecessor of `successor`.
            // We also need to make sure not to merge a block that serves as the
            // merge point in CFG. Such blocks will have more than one use.
            if (successor->hasMoreThanOneUse())
                break;
            changed = true;
            Index paramIndex = 0;
            auto inst = successor->getFirstDecorationOrChild();
            while (inst)
            {
                auto next = inst->getNextInst();
                if (inst->getOp() == kIROp_Param)
                {
                    inst->replaceUsesWith(branch->getArg(paramIndex));
                    paramIndex++;
                }
                else
                {
                    inst->removeFromParent();
                    inst->insertAtEnd(block);
                }
                inst = next;
            }
            branch->removeAndDeallocate();
            assert(!successor->hasUses());
            successor->removeAndDeallocate();
        }
        for (auto successor : block->getSuccessors())
        {
            if (processedBlock.Add(successor))
            {
                workList.add(successor);
            }
        }
    }
    return changed;
}

bool simplifyCFG(IRModule* module)
{
    bool changed = false;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            changed |= processFunc(func);
        }
    }
    return changed;
}

} // namespace Slang
