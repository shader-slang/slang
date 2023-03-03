// slang-ir-autodiff-region.h
#pragma once

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-autodiff.h"

namespace Slang
{
struct IndexedRegion : public RefObject
{
    IRLoop* loop;
    IndexedRegion* parent;

    IndexedRegion(IRLoop* loop, IndexedRegion* parent) : loop(loop), parent(parent)
    { }

    IRBlock* getInitializerBlock() { return as<IRBlock>(loop->getParent()); }
    IRBlock* getConditionBlock() 
    {
        auto condBlock = as<IRBlock>(loop->getTargetBlock());
        SLANG_RELEASE_ASSERT(as<IRIfElse>(condBlock->getTerminator()));
        return condBlock;
    }

    IRBlock* getBreakBlock() { return loop->getBreakBlock(); }

    IRBlock* getUpdateBlock() 
    { 
        auto initBlock = getInitializerBlock();

        auto condBlock = getConditionBlock();

        IRBlock* lastLoopBlock = nullptr;

        for (auto predecessor : condBlock->getPredecessors())
        {
            if (predecessor != initBlock)
                lastLoopBlock = predecessor;
        }

        // Should find atleast one predecessor that is _not_ the 
        // init block (that contains the loop info). This 
        // predecessor would be the last block in the loop
        // before looping back to the condition.
        // 
        SLANG_RELEASE_ASSERT(lastLoopBlock);

        return lastLoopBlock;
    }
};

struct IndexTrackingInfo : public RefObject
{
    // After lowering, store references to the count
    // variables associated with this region
    //
    IRInst*         primalCountParam   = nullptr;
    IRInst*         diffCountParam     = nullptr;

    IRVar*          primalCountLastVar   = nullptr;

    enum CountStatus
    {
        Unresolved,
        Dynamic,
        Static
    };

    CountStatus    status           = CountStatus::Unresolved;

    // Inferred maximum number of iterations.
    Count          maxIters         = -1;
};

struct IndexedRegionMap : public RefObject
{
    Dictionary<IRBlock*, IndexedRegion*> map;
    List<RefPtr<IndexedRegion>> regions;

    IndexedRegion* newRegion(IRLoop* loop, IndexedRegion* parent)
    {
        auto region = new IndexedRegion(loop, parent);
        regions.add(region);

        return region;
    }

    void mapBlock(IRBlock* block, IndexedRegion* region)
    {
        map.Add(block, region);
    }

    bool hasMapping(IRBlock* block)
    {
        return map.ContainsKey(block);
    }

    IndexedRegion* getRegion(IRBlock* block)
    {
        return map[block];
    }

    List<IndexedRegion*> getAllAncestorRegions(IRBlock* block)
    {
        List<IndexedRegion*> regionList;

        IndexedRegion* region = getRegion(block);
        for (; region; region = region->parent)
            regionList.add(region);

        return regionList;
    }
};

RefPtr<IndexedRegionMap> buildIndexedRegionMap(IRGlobalValueWithCode* func)
{
    RefPtr<IndexedRegionMap> regionMap = new IndexedRegionMap;

    List<IRBlock*> workList;
    
    regionMap->mapBlock(func->getFirstBlock(), nullptr);
    workList.add(func->getFirstBlock());

    while (workList.getCount() > 0)
    {
        auto currentBlock = workList.getLast();
        workList.removeLast();

        auto terminator = currentBlock->getTerminator();
        auto currentRegion = regionMap->getRegion(currentBlock);

        switch (terminator->getOp())
        {
            case kIROp_loop:
            {
                auto loopRegion = regionMap->newRegion(as<IRLoop>(terminator), currentRegion);
                auto condBlock = as<IRLoop>(terminator)->getTargetBlock();

                regionMap->mapBlock(condBlock, loopRegion);
                workList.add(condBlock);
                                
                auto ifElse = as<IRIfElse>(condBlock->getTerminator());
                SLANG_RELEASE_ASSERT(ifElse);

                // TODO: this is one of the places we'll need to change if we support loops that
                // loop on either the true or false side. For now, we assume the loop is on the
                // true side only.
                //
                regionMap->mapBlock(ifElse->getFalseBlock(), currentRegion);
                workList.add(ifElse->getFalseBlock());
            }
        }

        for (auto successor : currentBlock->getSuccessors())
        {   
            // If already mapped, skip.
            if (regionMap->hasMapping(successor))
                continue;
            regionMap->mapBlock(successor, currentRegion);
            workList.add(successor);
        }
    }
    
    return regionMap;
}


};