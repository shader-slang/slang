#include "slang-ir-simplify-cfg.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"
#include "slang-ir-dominators.h"
#include "slang-ir-restructure.h"
#include "slang-ir-util.h"

namespace Slang
{

struct CFGSimplificationContext
{
    RefPtr<RegionTree> regionTree;
    RefPtr<IRDominatorTree> domTree;
};

static BreakableRegion* findBreakableRegion(Region* region)
{
    for (;;)
    {
        if (auto b = as<BreakableRegion>(region))
            return b;
        region = region->getParent();
        if (!region)
            return nullptr;
    }
}

// Test if a loop is trivial: a trivial loop runs for a single iteration without any back edges, and
// there is only one break out of the loop at the very end. The function generates `regionTree` if
// it is needed and hasn't been generated yet.
static bool isTrivialSingleIterationLoop(
    IRGlobalValueWithCode* func,
    IRLoop* loop,
    CFGSimplificationContext& inoutContext)
{
    auto targetBlock = loop->getTargetBlock();
    if (targetBlock->getPredecessors().getCount() != 1) return false;
    if (*targetBlock->getPredecessors().begin() != loop->getParent()) return false;

    int useCount = 0;
    for (auto use = loop->getBreakBlock()->firstUse; use; use = use->nextUse)
    {
        if (use->getUser() == loop)
            continue;
        useCount++;
        if (useCount > 1)
            return false;
    }

    // The loop has passed simple test.
    // 
    // We need to verify this is a trivial loop by checking if there is any multi-level breaks
    // that skips out of this loop.

    if (!inoutContext.domTree)
        inoutContext.domTree = computeDominatorTree(func);
    if (!inoutContext.regionTree)
        inoutContext.regionTree = generateRegionTreeForFunc(func, nullptr);

    SimpleRegion* targetBlockRegion = nullptr;
    if (!inoutContext.regionTree->mapBlockToRegion.TryGetValue(targetBlock, targetBlockRegion))
        return false;
    BreakableRegion* loopBreakableRegion = findBreakableRegion(targetBlockRegion);
    LoopRegion* loopRegion = as<LoopRegion>(loopBreakableRegion);
    if (!loopRegion)
        return false;
    for (auto block : func->getBlocks())
    {
        if (!inoutContext.domTree->dominates(loop->getTargetBlock(), block))
            continue;
        if (inoutContext.domTree->dominates(loop->getBreakBlock(), block))
            continue;
        SimpleRegion* region = nullptr;
        if (!inoutContext.regionTree->mapBlockToRegion.TryGetValue(block, region))
            return false;

        for (auto branchTarget : block->getSuccessors())
        {
            SimpleRegion* targetRegion = nullptr;
            if (!inoutContext.regionTree->mapBlockToRegion.TryGetValue(branchTarget, targetRegion))
                return false;
            // If multi-level break out that skips over this loop exists, then this is not a trivial loop.
            if (targetRegion->isDescendentOf(loopRegion))
                continue;
            if (targetBlock != loop->getBreakBlock())
                return false;
            if (findBreakableRegion(region) != loopRegion)
            {
                // If the break is initiated from a nested region, this is not trivial.
                return false;
            }
        }
    }

    return true;
}

static bool removeDeadBlocks(IRGlobalValueWithCode* func)
{
    bool changed = false;
    List<IRBlock*> workList;
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return false;

    for (auto block = firstBlock->getNextBlock(); block; block = block->getNextBlock())
    {
        workList.add(block);
    }

    HashSet<IRBlock*> workListSet;
    List<IRBlock*> nextWorkList;
    for (;;)
    {
        for (Index i = 0; i < workList.getCount(); i++)
        {
            auto block = workList[i];
            if (!block->hasUses() && as<IRTerminatorInst>(block->getFirstInst()))
            {
                for (auto succ : block->getSuccessors())
                {
                    if (workListSet.Add(succ))
                    {
                        nextWorkList.add(succ);
                    }
                }
                block->removeAndDeallocate();
                changed = true;
            }
        }
        if (nextWorkList.getCount())
        {
            workList = _Move(nextWorkList);
            workListSet.Clear();
        }
        else
        {
            break;
        }
    }
    return changed;
}

static bool isTrivialIfElse(IRIfElse* condBranch, List<IRInst*>& outArgs)
{
    bool isTrueBranchTrivial = false;
    IRUnconditionalBranch* trueBlockBranch = nullptr;
    if (condBranch->getTrueBlock() != condBranch->getAfterBlock())
    {
        if (auto br = as<IRUnconditionalBranch>(condBranch->getTrueBlock()->getFirstOrdinaryInst()))
        {
            if (br->getTargetBlock() == condBranch->getAfterBlock() && br->getOp() == kIROp_unconditionalBranch)
            {
                isTrueBranchTrivial = true;
                trueBlockBranch = br;
            }
        }
    }
    else
    {
        isTrueBranchTrivial = true;
    }
    bool isFalseBranchTrivial = false;
    IRUnconditionalBranch* falseBlockBranch = nullptr;
    if (condBranch->getFalseBlock() != condBranch->getAfterBlock())
    {
        if (auto br = as<IRUnconditionalBranch>(condBranch->getFalseBlock()->getFirstOrdinaryInst()))
        {
            if (br->getTargetBlock() == condBranch->getAfterBlock() && br->getOp() == kIROp_unconditionalBranch)
            {
                isFalseBranchTrivial = true;
                falseBlockBranch = br;
            }
        }
    }
    else
    {
        isFalseBranchTrivial = true;
    }
    if (isTrueBranchTrivial && isFalseBranchTrivial)
    {
        // If one of the branch target is afterBlock itself, and the other branch
        // is a trivial block that jumps into the afterBlock, this if-else is trivial.
        // In this case the argCount must be 0 because a block with phi parameters can't
        // be used as targets in a conditional branch.
        if (!trueBlockBranch || !falseBlockBranch)
            return true;

        // If both branches are trivial blocks, we must compare the arguments.
        if (trueBlockBranch->getArgCount() != falseBlockBranch->getArgCount())
        {
            // This should never happen, return false now to be safe.
            return false;
        }
        for (UInt i = 0; i < trueBlockBranch->getArgCount(); i++)
        {
            outArgs.add(trueBlockBranch->getArg(i));
            if (trueBlockBranch->getArg(i) != falseBlockBranch->getArg(i))
            {
                // argument is different, the if-else is non-trivial.
                return false;
            }
        }
        return true;
    }
    return false;
}

static bool isTrueLit(IRInst* lit)
{
    if (auto boolLit = as<IRBoolLit>(lit))
        return boolLit->getValue();
    return false;
}
static bool isFalseLit(IRInst* lit)
{
    if (auto boolLit = as<IRBoolLit>(lit))
        return !boolLit->getValue();
    return false;
}

static bool simplifyBoolPhiParam(IRIfElse* ifElse, Array<IRBlock*, 2>& preds, IRParam* param, UInt paramIndex)
{
    // For bool params where its value is assigned from the same `if-else` statement,
    // we can simplify it into an expression of the condition of the source `if-else`.

    if (!param->getDataType() || param->getDataType()->getOp() != kIROp_BoolType)
        return false;

    auto branch0 = as<IRUnconditionalBranch>(preds[0]->getTerminator());
    if (!branch0)
        return false;
    if (branch0->getArgCount() <= paramIndex)
        return false;
    auto branch1 = as<IRUnconditionalBranch>(preds[1]->getTerminator());
    if (!branch1)
        return false;
    if (branch1->getArgCount() <= paramIndex)
        return false;

    IRInst* replacement = nullptr;
    if (isTrueLit(branch0->getArg(paramIndex)) && isFalseLit(branch1->getArg(paramIndex)))
    {
        replacement = ifElse->getCondition();
    }
    else if (isFalseLit(branch0->getArg(paramIndex)) && isTrueLit(branch1->getArg(paramIndex)))
    {
        IRBuilder builder(param);
        setInsertBeforeOrdinaryInst(&builder, param);
        replacement = builder.emitNot(builder.getBoolType(), ifElse->getCondition());
    }
    if (replacement)
    {
        param->replaceUsesWith(replacement);
        param->removeAndDeallocate();
        branch0->removeArgument(paramIndex);
        branch1->removeArgument(paramIndex);
        return true;
    }
    return false;
}

static bool simplifyBoolPhiParams(IRBlock* block)
{
    if (!block)
        return false;

    if (block->getPredecessors().getCount() != 2)
        return false;

    Array<IRBlock*, 2> preds;
    for (auto pred : block->getPredecessors())
        preds.add(pred);

    IRBlock* ifElseBlock = nullptr;
    if (preds[0]->getPredecessors().getCount() != 1)
        return false;
    ifElseBlock = *(preds[0]->getPredecessors().begin());
    if (preds[1]->getPredecessors().getCount() != 1)
        return false;
    auto p = *(preds[1]->getPredecessors().begin());
    if (p != ifElseBlock)
        return false;

    auto ifElse = as<IRIfElse>(ifElseBlock->getTerminator());
    if (!ifElse)
        return false;

    if (ifElse->getTrueBlock() == preds[1])
    {
        Swap(preds[0], preds[1]);
    }
    SLANG_ASSERT(ifElse->getTrueBlock() == preds[0] && ifElse->getFalseBlock() == preds[1]);

    List<IRParam*> params;
    for (auto param : block->getParams())
        params.add(param);
    bool changed = false;
    for (Index i = params.getCount() - 1; i >= 0; i--)
    {
        changed |= simplifyBoolPhiParam(ifElse, preds, params[i], (UInt)i);
    }
    return changed;
}

static bool removeTrivialPhiParams(IRBlock* block)
{
    // We can remove a phi parmeter if:
    // 1. all arguments to a parameter is the same (not really a phi).
    // 2. the arguments to the parameter is always the same as arguments to another existing parameter (duplicate phi).

    bool changed = false;
    List<IRParam*> params;
    struct ParamState
    {
        bool areKnownValueSame = true;
        IRInst* knownValue = nullptr;
        OrderedHashSet<UInt> sameAsParamSet;
    };
    List<ParamState> args;
    List<IRUnconditionalBranch*> termInsts;
    for (auto param : block->getParams())
    {
        params.add(param);
        args.add(ParamState());
    }

    if (!params.getCount())
        return false;

    for (UInt i = 1; i < (UInt)args.getCount(); i++)
        for (UInt j = 0; j < i; j++)
            args[i].sameAsParamSet.Add(j);

    for (auto pred : block->getPredecessors())
    {
        auto termInst = as<IRUnconditionalBranch>(pred->getTerminator());
        if (!termInst)
            return false;
        SLANG_ASSERT(termInst->getArgCount() == (UInt)args.getCount());
        termInsts.add(termInst);
        for (UInt i = 0; i < termInst->getArgCount(); i++)
        {
            if (args[i].areKnownValueSame)
            {
                if (args[i].knownValue == nullptr)
                    args[i].knownValue = termInst->getArg(i);
                else if (args[i].knownValue != termInst->getArg(i))
                    args[i].areKnownValueSame = false;
            }
            for (UInt j = 0; j < i; j++)
            {
                if (termInst->getArg(i) != termInst->getArg(j))
                {
                    args[i].sameAsParamSet.Remove(j);
                }
            }
        }
    }
    for (Index i = args.getCount() - 1; i >= 0; i--)
    {
        IRInst* targetVal = nullptr;
        if (args[i].areKnownValueSame)
        {
            targetVal = args[i].knownValue;
        }
        else if (args[i].sameAsParamSet.Count())
        {
            auto targetParamId = *args[i].sameAsParamSet.begin();
            targetVal = params[targetParamId];
        }
        if (targetVal)
        {
            params[i]->replaceUsesWith(args[i].knownValue);
            params[i]->removeAndDeallocate();
            for (auto termInst : termInsts)
                termInst->removeArgument((UInt)i);
            changed = true;
        }
    }
    return changed;
}

static bool processFunc(IRGlobalValueWithCode* func)
{
    auto firstBlock = func->getFirstBlock();
    if (!firstBlock)
        return false;

    // Lazily generated region tree.
    CFGSimplificationContext simplificationContext;

    IRBuilder builder(func->getModule());

    bool changed = false;
    for (;;)
    {
        List<IRBlock*> workList;
        HashSet<IRBlock*> processedBlock;
        workList.add(func->getFirstBlock());
        while (workList.getCount())
        {
            auto block = workList.getFirst();
            workList.fastRemoveAt(0);
            while (block)
            {
                // If all arguments to a phi parameter are the known to be the same,
                // we can safely replace the phi parameter with the argument.
                if (block != func->getFirstBlock())
                {
                    changed |= simplifyBoolPhiParams(block);
                    changed |= removeTrivialPhiParams(block);
                }

                if (auto loop = as<IRLoop>(block->getTerminator()))
                {
                    // If continue block is unreachable, remove it.
                    auto continueBlock = loop->getContinueBlock();
                    if (continueBlock && !continueBlock->hasMoreThanOneUse())
                    {
                        loop->continueBlock.set(loop->getTargetBlock());
                        continueBlock->removeAndDeallocate();
                    }

                    // If there isn't any actual back jumps into loop target and there is a trivial
                    // break at the end of the loop, we can remove the header and turn it into
                    // a normal branch.
                    auto targetBlock = loop->getTargetBlock();
                    if (isTrivialSingleIterationLoop(func, loop, simplificationContext))
                    {
                        builder.setInsertBefore(loop);
                        List<IRInst*> args;
                        for (UInt i = 0; i < loop->getArgCount(); i++)
                        {
                            args.add(loop->getArg(i));
                        }
                        builder.emitBranch(targetBlock, args.getCount(), args.getBuffer());
                        loop->removeAndDeallocate();
                        changed = true;
                    }
                }
                else if (auto condBranch = as<IRIfElse>(block->getTerminator()))
                {
                    // If both branches of `if-else` are trivial jumps into after block,
                    // we can get rid of the entire conditional branch and replace it
                    // with a jump into the after block.
                    List<IRInst*> args;
                    if (isTrivialIfElse(condBranch, args))
                    {
                        builder.setInsertBefore(condBranch);
                        builder.emitBranch(condBranch->getAfterBlock(), (Int)args.getCount(), args.getBuffer());
                        condBranch->removeAndDeallocate();
                        changed = true;
                    }
                }

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
                if (block->hasMoreThanOneUse())
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
                break;
            }
            for (auto successor : block->getSuccessors())
            {
                if (processedBlock.Add(successor))
                {
                    workList.add(successor);
                }
            }
        }
        bool blocksRemoved = removeDeadBlocks(func);
        changed |= blocksRemoved;
        if (!blocksRemoved)
            break;
    }
    return changed;
}

bool simplifyCFG(IRModule* module)
{
    bool changed = false;
    for (auto inst : module->getGlobalInsts())
    {
        if (auto genericInst = as<IRGeneric>(inst))
        {
            inst = findGenericReturnVal(genericInst);
        }
        if (auto func = as<IRFunc>(inst))
        {
            changed |= processFunc(func);
        }
    }
    return changed;
}

bool simplifyCFG(IRGlobalValueWithCode* func)
{
    return processFunc(func);
}

} // namespace Slang
