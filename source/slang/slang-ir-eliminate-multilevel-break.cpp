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

    struct BreakableRegionInfo : RefObject
    {
        BreakableRegionInfo* parent = nullptr;
        int level = 0;
        IRInst* headerInst;
        List<IRBlock*> blocks;
        HashSet<IRBlock*> blockSet;
        List<RefPtr<BreakableRegionInfo>> childRegions;
        IRBlock* getBreakBlock()
        {
            switch (headerInst->getOp())
            {
            case kIROp_loop:
                return as<IRLoop>(headerInst)->getBreakBlock();
            case kIROp_Switch:
                return as<IRSwitch>(headerInst)->getBreakLabel();
            default:
                SLANG_UNREACHABLE("Unknown breakable inst");
            }
        }

        template<typename Func>
        void forEach(const Func& f)
        {
            f(this);
            for (auto child : childRegions)
                child->forEach(f);
        }
    };

    struct MultiLevelBreakInfo
    {
        IRUnconditionalBranch* breakInst;
        BreakableRegionInfo* currentRegion;
        BreakableRegionInfo* breakTargetRegion;
    };

    struct FuncContext
    {
        List<RefPtr<BreakableRegionInfo>> regions;
        HashSet<IRBlock*> breakBlocks;
        Dictionary<IRBlock*, BreakableRegionInfo*> mapBreakBlockToRegion;
        Dictionary<IRBlock*, BreakableRegionInfo*> mapBlockToRegion;
        HashSet<IRBlock*> processedBlocks;
        List<MultiLevelBreakInfo> multiLevelBreaks;

        void collectBreakableRegionBlocks(BreakableRegionInfo& info)
        {
            // Push break block to a stack so we can easily check if a block is a break block in its
            // parent regions.
            breakBlocks.Add(info.getBreakBlock());
            
            auto successors = as<IRBlock>(info.headerInst->getParent())->getSuccessors();
            for (auto successor : successors)
            {
                if (!breakBlocks.Add(successor))
                    continue;
                if (info.blockSet.Add(successor))
                    info.blocks.add(successor);
            }

            for (Index i = 0; i < info.blocks.getCount(); i++)
            {
                auto block = info.blocks[i];
                if (!processedBlocks.Add(block))
                    continue;
                switch (block->getTerminator()->getOp())
                {
                case kIROp_loop:
                case kIROp_Switch:
                    {
                        // Both region and switch insts mark the start a breakable region.
                        RefPtr<BreakableRegionInfo> childRegion = new BreakableRegionInfo();
                        childRegion->headerInst = block->getTerminator();
                        childRegion->parent = &info;
                        childRegion->level = info.level + 1;
                        collectBreakableRegionBlocks(*childRegion);
                        info.childRegions.add(childRegion);
                        block = childRegion->getBreakBlock();
                        if (info.blockSet.Add(block))
                        {
                            info.blocks.add(block);
                        }
                        continue;
                    }
                default:
                    break;
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

            // Pop the break block from stack since we are no longer processing the region.
            breakBlocks.Remove(info.getBreakBlock());
        }

        void gatherInfo(IRGlobalValueWithCode* func)
        {
            for (auto block : func->getBlocks())
            {
                if (processedBlocks.Contains(block))
                    continue;
                auto terminator = block->getTerminator();
                switch (terminator->getOp())
                {
                case kIROp_loop:
                case kIROp_Switch:
                    {
                        RefPtr<BreakableRegionInfo> regionInfo = new BreakableRegionInfo();
                        regionInfo->headerInst = terminator;
                        collectBreakableRegionBlocks(*regionInfo);
                        regions.add(regionInfo);
                    }
                    break;
                default:
                    break;
                }
            }
            for (auto& l : regions)
            {
                l->forEach(
                    [&](BreakableRegionInfo* region)
                    {
                        mapBreakBlockToRegion.Add(region->getBreakBlock(), region);
                        for (auto block : region->blocks)
                            mapBlockToRegion.Add(block, region);
                    });
            }

            for (auto block : func->getBlocks())
            {
                auto terminator = block->getTerminator();
                if (auto branch = as<IRUnconditionalBranch>(terminator))
                {
                    if (as<IRLoop>(terminator))
                        continue;
                    BreakableRegionInfo* breakTargetRegion = nullptr;
                    BreakableRegionInfo* currentRegion = nullptr;
                    if (!mapBreakBlockToRegion.TryGetValue(branch->getTargetBlock(), breakTargetRegion))
                        continue;
                    if (mapBlockToRegion.TryGetValue(block, currentRegion))
                    {
                        if (currentRegion != breakTargetRegion)
                        {
                            MultiLevelBreakInfo breakInfo;
                            breakInfo.breakInst = branch;
                            breakInfo.breakTargetRegion = breakTargetRegion;
                            breakInfo.currentRegion = currentRegion;
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

        OrderedHashSet<BreakableRegionInfo*> skippedOverRegions;
        auto unreachableBlock = builder.emitBlock();
        builder.setInsertInto(unreachableBlock);
        builder.emitUnreachable();
        builder.setInsertInto(func);

        // Rewrite multi-level breaks with single level break + target level argument.
        for (auto breakInfo : funcInfo.multiLevelBreaks)
        {
            auto region = breakInfo.currentRegion;
            while (region)
            {
                skippedOverRegions.Add(region);
                region = region->parent;
                if (region == breakInfo.breakTargetRegion)
                    break;
            }
            builder.setInsertBefore(breakInfo.breakInst);
            auto targetLevelInst = builder.getIntValue(builder.getIntType(), breakInfo.breakTargetRegion->level);
            builder.emitBranch(breakInfo.currentRegion->getBreakBlock(), 1, &targetLevelInst);
            breakInfo.breakInst->removeAndDeallocate();
        }

        // Rewrite skipped-over break blocks to accept a target level argument.
        builder.setInsertInto(func);
        OrderedDictionary<IRBlock*, int> mapNewBreakBlockToRegionLevel;
        for (auto skippedRegion : skippedOverRegions)
        {
            auto breakBlock = skippedRegion->getBreakBlock();

            // The existing break block cannot have parameters. We assume that PHI-elimination is
            // run before this pass.
            SLANG_RELEASE_ASSERT(breakBlock->getFirstParam() == nullptr);

            // The new CFG structure will be: newBreakBlock --> newBreakBodyBlock { IfElse
            // (-->oldBreakBlock, -->outerBreakBlock) } `newBreakBlock` defines the `IRParam` for
            // the break target, then immediately jumps to `newBreakBodyBlock` for the actual
            // branch. We need this separation to avoid introducing critical edge to the CFG (blocks
            // cannot have more than 1 predecessors and more than 1 successors at the same time).
            auto jumpToOuterBlock = builder.createBlock();
            auto newBreakBodyBlock = builder.createBlock();
            auto newBreakBlock = builder.createBlock();
            newBreakBlock->insertBefore(breakBlock);
            newBreakBodyBlock->insertAfter(breakBlock);
            jumpToOuterBlock->insertAfter(newBreakBlock);
            mapNewBreakBlockToRegionLevel[newBreakBlock] = skippedRegion->level;
            breakBlock->replaceUsesWith(newBreakBlock);
            builder.setInsertInto(newBreakBlock);
            auto targetLevelParam = builder.emitParam(builder.getIntType());
            builder.emitBranch(newBreakBodyBlock);
            builder.setInsertInto(newBreakBodyBlock);
            auto levelNeq = builder.emitNeq(targetLevelParam, builder.getIntValue(builder.getIntType(), skippedRegion->level));
            builder.emitIfElse(levelNeq, jumpToOuterBlock, breakBlock, unreachableBlock);
            builder.setInsertInto(jumpToOuterBlock);
            if (skippedOverRegions.Contains(skippedRegion->parent))
            {
                builder.emitBranch(skippedRegion->parent->getBreakBlock(), 1, (IRInst**)&targetLevelParam);
            }
            else
            {
                builder.emitBranch(skippedRegion->parent->getBreakBlock());
            }
        }

        // Once we have rewritten regions' break blocks with additional targetLevel parameter, all
        // original branches into that block without a parameter will now need to provide a default
        // value equal to the level of its corresponding region.
        for (auto breakBlockKV : mapNewBreakBlockToRegionLevel)
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
                        if (user->getOp() == kIROp_Switch && &(as<IRSwitch>(user)->breakLabel) == use)
                        {
                            // If this is the "breakLabel" operand of the original switch inst, don't do anything
                            // since it is not an actual branch.
                            continue;
                        }
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
