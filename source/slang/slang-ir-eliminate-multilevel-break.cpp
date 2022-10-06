// slang-ir-eliminate-multilevel-break.cpp
#include "slang-ir-eliminate-multilevel-break.h"
#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-eliminate-phis.h"
#include "slang-ir-dominators.h"

namespace Slang
{

struct EliminateMultiLevelBreakContext
{
    IRModule* irModule;

    struct LoopInfo : RefObject
    {
        LoopInfo* parent = nullptr;
        int level = 0;
        IRLoop* loopInst;
        List<IRBlock*> blocks;
        HashSet<IRBlock*> blockSet;
        List<RefPtr<LoopInfo>> childLoops;
        IRBlock* getBreakBlock() { return loopInst->getBreakBlock(); }
        template<typename Func>
        void forEach(const Func& f)
        {
            f(this);
            for (auto child : childLoops)
                child->forEach(f);
        }
    };

    struct MultiLevelBreakInfo
    {
        IRUnconditionalBranch* breakInst;
        LoopInfo* currentLoop;
        LoopInfo* breakTargetLoop;
    };

    struct FuncContext
    {
        List<RefPtr<LoopInfo>> loops;
        HashSet<IRBlock*> breakBlocks;
        Dictionary<IRBlock*, LoopInfo*> mapBreakBlockToLoop;
        Dictionary<IRBlock*, LoopInfo*> mapBlockToLoop;
        HashSet<IRBlock*> processedBlocks;
        List<MultiLevelBreakInfo> multiLevelBreaks;

        void collectLoopBlocks(LoopInfo& info)
        {
            auto startBlock = info.loopInst->getTargetBlock();
            info.blockSet.Add(startBlock);
            info.blocks.add(startBlock);
            breakBlocks.Add(info.loopInst->getBreakBlock());
            for (Index i = 0; i < info.blocks.getCount(); i++)
            {
                auto block = info.blocks[i];
                if (!processedBlocks.Add(block))
                    continue;
                if (auto loopInst = as<IRLoop>(block->getTerminator()))
                {
                    RefPtr<LoopInfo> childLoop = new LoopInfo();
                    childLoop->loopInst = loopInst;
                    childLoop->parent = &info;
                    childLoop->level = info.level + 1;
                    collectLoopBlocks(*childLoop);
                    info.childLoops.add(childLoop);
                    block = loopInst->getBreakBlock();
                    if (info.blockSet.Add(block))
                    {
                        info.blocks.add(block);
                    }
                    continue;
                }
                for (auto succ : block->getSuccessors())
                {
                    if (!breakBlocks.Contains(succ))
                    {
                        if (info.blockSet.Add(succ))
                            info.blocks.add(succ);
                    }
                }
            }
            breakBlocks.Remove(info.loopInst->getBreakBlock());
        }

        void gatherInfo(IRGlobalValueWithCode* func)
        {
            for (auto block : func->getBlocks())
            {
                if (processedBlocks.Contains(block))
                    continue;
                auto terminator = block->getTerminator();
                if (auto loop = as<IRLoop>(terminator))
                {
                    RefPtr<LoopInfo> loopInfo = new LoopInfo();
                    loopInfo->loopInst = loop;
                    collectLoopBlocks(*loopInfo);
                    loops.add(loopInfo);
                }
            }

            for (auto& l : loops)
            {
                l->forEach(
                    [&](LoopInfo* loop)
                    {
                        mapBreakBlockToLoop.Add(loop->loopInst->getBreakBlock(), loop);
                        for (auto block : loop->blocks)
                            mapBlockToLoop.Add(block, loop);
                    });
            }

            for (auto block : func->getBlocks())
            {
                auto terminator = block->getTerminator();
                if (auto branch = as<IRUnconditionalBranch>(terminator))
                {
                    if (as<IRLoop>(terminator))
                        continue;
                    LoopInfo* breakLoop = nullptr;
                    LoopInfo* currentLoop = nullptr;
                    if (!mapBreakBlockToLoop.TryGetValue(branch->getTargetBlock(), breakLoop))
                        continue;
                    if (mapBlockToLoop.TryGetValue(block, currentLoop))
                    {
                        if (currentLoop != breakLoop)
                        {
                            MultiLevelBreakInfo breakInfo;
                            breakInfo.breakInst = branch;
                            breakInfo.breakTargetLoop = breakLoop;
                            breakInfo.currentLoop = currentLoop;
                            multiLevelBreaks.add(breakInfo);
                        }
                    }
                }
            }
        }
    };

    void processFunc(IRGlobalValueWithCode* func)
    {
        // If func does not have any multi-level breaks, return.
        {
            FuncContext funcInfo;
            funcInfo.gatherInfo(func);

            if (funcInfo.multiLevelBreaks.getCount() == 0)
                return;
        }

        // To make things easy, eliminate Phis before perform transformations.
        eliminatePhisInFunc(LivenessMode::Disabled, irModule, func);

        // Before modifying the cfg, we gather all required info from the existing cfg.
        FuncContext funcInfo;
        funcInfo.gatherInfo(func);

        if (funcInfo.multiLevelBreaks.getCount() == 0)
            return;

        SharedIRBuilder sharedBuilder;
        sharedBuilder.init(irModule);
        IRBuilder builder(&sharedBuilder);
        builder.setInsertInto(func);

        OrderedHashSet<LoopInfo*> skippedOverLoops;
        auto unreachableBlock = builder.emitBlock();
        builder.setInsertInto(unreachableBlock);
        builder.emitUnreachable();
        builder.setInsertInto(func);

        // Rewrite multi-level breaks with single level break + target level argument.
        for (auto breakInfo : funcInfo.multiLevelBreaks)
        {
            auto loop = breakInfo.currentLoop;
            while (loop)
            {
                skippedOverLoops.Add(loop);
                loop = loop->parent;
                if (loop == breakInfo.breakTargetLoop)
                    break;
            }
            builder.setInsertBefore(breakInfo.breakInst);
            auto targetLevelInst = builder.getIntValue(builder.getIntType(), breakInfo.breakTargetLoop->level);
            builder.emitBranch(breakInfo.currentLoop->getBreakBlock(), 1, &targetLevelInst);
            breakInfo.breakInst->removeAndDeallocate();
        }

        // Rewrite skipped-over break blocks to accept a target level argument.
        builder.setInsertInto(func);
        OrderedDictionary<IRBlock*, int> mapNewBreakBlockToLoopLevel;
        for (auto skippedLoop : skippedOverLoops)
        {
            auto breakBlock = skippedLoop->getBreakBlock();

            // The existing break block cannot have parameters. We assume that PHI-elimination is
            // run before this pass.
            SLANG_RELEASE_ASSERT(breakBlock->getFirstParam() == nullptr);

            // The new CFG structure will be: newBreakBlock --> newBreakBodyBlock { IfElse (-->oldBreakBlock, -->outerBreakBlock) }
            // `newBreakBlock` defines the `IRParam` for the break target, then immediately jumps to `newBreakBodyBlock` for the actual branch. We need this
            // separation to avoid introducing critical edge to the CFG (blocks cannot have more
            // than 1 predecessors and more than 1 successors at the same time).
            auto jumpToOuterBlock = builder.emitBlock();
            auto newBreakBodyBlock = builder.emitBlock();
            auto newBreakBlock = builder.emitBlock();
            mapNewBreakBlockToLoopLevel[newBreakBlock] = skippedLoop->level;
            breakBlock->replaceUsesWith(newBreakBlock);
            builder.setInsertInto(newBreakBlock);
            auto targetLevelParam = builder.emitParam(builder.getIntType());
            builder.emitBranch(newBreakBodyBlock);
            builder.setInsertInto(newBreakBodyBlock);
            auto levelNeq = builder.emitNeq(targetLevelParam, builder.getIntValue(builder.getIntType(), skippedLoop->level));
            builder.emitIfElse(levelNeq, jumpToOuterBlock, breakBlock, unreachableBlock);
            builder.setInsertInto(jumpToOuterBlock);
            if (skippedOverLoops.Contains(skippedLoop->parent))
            {
                builder.emitBranch(skippedLoop->parent->getBreakBlock(), 1, (IRInst**)&targetLevelParam);
            }
            else
            {
                builder.emitBranch(skippedLoop->parent->getBreakBlock());
            }
        }

        // Once we have rewritten loops' break blocks with additional targetLevel parameter, all
        // original branches into that block without a parameter will now need to provide a default
        // value equal to the level of its corresponding loop.
        for (auto breakBlockKV : mapNewBreakBlockToLoopLevel)
        {
            auto breakBlock = breakBlockKV.Key;
            auto level = breakBlockKV.Value;
            IRInst* levelInst = nullptr;
            List<IRUse*> uses;
            for (auto use = breakBlock->firstUse; use; use = use->nextUse)
            {
                uses.add(use);
            }
            for (auto use : uses)
            {
                auto user = use->getUser();
                switch (user->getOp())
                {
                case kIROp_conditionalBranch:
                case kIROp_ifElse:
                case kIROp_Switch:
                    // For complex branches, insert an intermediate block so we can specify the
                    // target index argument.
                    {
                        builder.setInsertInto(func);
                        auto tmpBlock = builder.createBlock();
                        tmpBlock->insertAfter(user->getParent());
                        builder.setInsertInto(tmpBlock);
                        if (!levelInst)
                            levelInst = builder.getIntValue(builder.getIntType(), level);
                        builder.emitBranch(breakBlock, 1, &levelInst);
                        use->set(tmpBlock);
                    }
                    break;
                case kIROp_loop:
                    // Ignore.
                    continue;
                case kIROp_unconditionalBranch:
                    {
                        auto originalBranch = as<IRUnconditionalBranch>(user);
                        if (originalBranch->getOperandCount() == 1)
                        {
                            builder.setInsertBefore(originalBranch);
                            if (!levelInst)
                                levelInst = builder.getIntValue(builder.getIntType(), level);
                            builder.emitBranch(breakBlock, 1, &levelInst);
                            originalBranch->removeAndDeallocate();
                        }
                    }
                    break;
                }
                
            }
        }
    }
};

void eliminateMultiLevelBreak(IRModule* irModule)
{
    EliminateMultiLevelBreakContext context;
    context.irModule = irModule;
    for (auto globalInst : irModule->getGlobalInsts())
    {
        if (auto codeInst = as<IRGlobalValueWithCode>(globalInst))
        {
            context.processFunc(codeInst);
        }
    }
}

void eliminateMultiLevelBreakForFunc(IRModule* irModule, IRGlobalValueWithCode* func)
{
    EliminateMultiLevelBreakContext context;
    context.irModule = irModule;
    context.processFunc(func);
}

} // namespace Slang
