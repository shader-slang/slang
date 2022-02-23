#include "slang-ir-simplify-cfg.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

void processFunc(IRFunc* func)
{
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return;

    List<IRBlock*> workList;
    HashSet<IRBlock*> processedBlock;
    workList.add(func->getFirstBlock());
    while (workList.getCount())
    {
        auto block = workList.getFirst();
        workList.fastRemoveAt(0);
        while (block)
        {
            auto branch = as<IRUnconditionalBranch>(block->getTerminator());
            // If `block` does not end with an unconditional branch, bail.
            if (!branch)
                break;
            auto successor = branch->getTargetBlock();
            // Only perform the merge if `block` is the only predecessor of `successor`.
            if (successor->getPredecessors().getCount() != 1)
                break;

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
}

void simplifyCFG(IRModule* module)
{
    for (auto inst : module->getGlobalInsts())
    {
        if (auto func = as<IRFunc>(inst))
        {
            processFunc(func);
        }
    }
}

} // namespace Slang
