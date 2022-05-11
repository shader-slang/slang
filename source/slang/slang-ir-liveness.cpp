#include "slang-ir-liveness.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

#include "slang-ir-dominators.h"

namespace Slang
{

/* 
Discussion 
==========

Currently we are only tracking 'var'/IRVar local variables. They are accessed via pointers. This
means

* We don't need to care about extractField / extractElement, as they only work directly on the value
* We need to track aliases created via getFieldPtr / getElementPtr
* There is a distinction between a 'pointer' and an 'address'.
  * A "pointer" can 'escape' just as in other languages, and is the general case
  * If we are talking about an "address", then this is constrained by our language rules,
  
NOTE! Confusingly there is getElementPtr and getFieldAddress (and getAddress). I also don't see Addr/Addr type as a distinct thing 
from Ptr, so I assume that differentiation is aspirational?

A) We don't need to worry about a phi node temporary holding a pointer (or scope ending *on* the branch), because
the phi node will pass the result by value, leading to a *load* before the branch..

Other

```
 let foo : Ptr<SomeStruct> = var;
...
store(someOtherPtr, foo); // this is `store`, but not a store *to* foo!!!!!
...
```

Here a *pointer* is being stored into someOtherPtr. This means all bets are off. Liveness will have to be assumed anywhere the 
variable is accessible. 
TODO(JS): Note that currently this scenario isn't handled by this algorithm.

```
   let foo : Ptr<SomeStruct> = var;
   ...
   br SomeOtherThing(foo); // OH NO!!!
```

It is believed this can't happen in current code. Leading to assertion A) above.

* Long-term IR type-system thing: we should probably have an explicit instruction
  that casts a local `Ptr<Foo>` to either an `Out<Foo>` or `InOut<Foo>` for exactly
  these cases (and then use the *cast* operation to tell us what is going on).
*/

/*
Take the code sequence

```HLSL
    SomeStruct s;
    SomeStruct t = makeSomeStruct();
    SomeStruct u = {};
```

Produces something like...

```HLSL
    SomeStruct_0 s_1;
    SLANG_LIVE_START(s_1)
    SomeStruct_0 t_0;
    SLANG_LIVE_START(t_0)
    SomeStruct_0 _S4 = makeSomeStruct_0();
    t_0 = _S4;
    SomeStruct_0 u_0;
    SLANG_LIVE_START(u_0)
    SomeStruct_0 _S6 = { ... };
    u_0 = _S6;
```

This is good, in so far as the variables do get LIVE_START, however they are defined. It is perhaps 'bad' in so far as a temporary
is created that is then just copied into the variable. That temporary being something that is mutable, and can be partially modified (it's a struct)
could perhaps have liveness issues.
*/

/* This implementation could potentially be improved in a few ways:

## Use Dominator tree block indexing

The dominator tree has a mapping from block insts to integer values. These integer value could be used to allow information about blocks to 
be stored in an array. A downside, would be exposing that aspect of the implementation to the dominator tree.

## Store if a block has an access instruction

As it stands, when traversing the tree to find the last access, the implementation searches for each block backwards to find the last access
by looking if it's in the m_accessSet. 

This searching could be avoided, by when accesses are added the block they are in is marked as having an access. If a block had no accesses
this would remove the linear search for the first access instruction, if the block indicates it doesn't have any. The downside being that every 
access would also have to mark the block it is in.
*/

/* 

Note for a block we don't need to store all of the accesses. Only the last before starts, or the end. 

We could just determine this by 

1) Finding all of the accesses/aliases
2) Finding the blocks the accesses/starts are in
3) Iterate through instructions finding accesses/starts. Store of the access/starts for the block (in order)
4) Do the usual recursive thing 

In terms of finding blocks/accesses/starts we could use the same mechanism as now. 

*/

namespace { // anonymous

struct LivenessContext
{
    typedef LivenessLocation Location;

    // NOTE! Care must be taken changing the order. Some checks rely on Found having a smaller value than `NotFound`.
    // Allowing NotFound to be promoted to Found.
    enum class BlockResult
    {
        Found,              ///< All paths were either not dominated, found 
        NotFound,           ///< It is dominated but no access was found. 
        Visited,            ///< The block has been visited (as part of a traversal), but does not yet have a result. Used to detect loops.
        NotVisited,         ///< Not visited
        NotDominated,       ///< If it's not dominated it can't have a liveness end 

        CountOf,
    };

        /// True if a result can be premoted `from` to `to`
    static bool canPromote(BlockResult from, BlockResult to) { return (from == BlockResult::NotVisited) || (Index(to) <= Index(from) && from != BlockResult::NotDominated); }

    enum class AccessType
    {
        None,               ///< There is no access
        Alias,              ///< Produces an alias to the root
        Access,             ///< Is an access to the root (perhaps through an alias)
    };
    
    struct InstOrder
    {
        IRInst* inst;               ///< The inst (must be either a LiveStart or an access)
        IRBlock* block;             ///< Block the inst belongs to
        Index index;                ///< The index to the instruction in the block
    };

    struct BlockInfo
    {
        IRBlock* block;             ///< The block associated with this index
        BlockResult result;         ///< The result for this block
        Index orderStart;           ///< The start InstOrder index
        Count orderCount;           ///< The count of the amount of InstOrder
        Count startsCount;          ///< The number of starts (the number of accesses is orderCount - startsCount)
    };

    struct SuccessorResult
    {
        BlockResult result;         ///< The result 
        Index blockIndex;           ///< The block index of the successor
    };

        /// Process all the locations
    void processLocations(const List<Location>& locations);
    
    LivenessContext(IRModule* module):
        m_module(module)
    {
        m_sharedBuilder.init(module);
        m_builder.init(m_sharedBuilder);
    }

        /// For a given live range start find it's end and insert a LiveRangeEnd
        /// Can only be called after a call to _findAliasesAndAccesses for the root.
    void _findAndEmitRangeEnd(IRLiveRangeStart* liveStart);

        /// Processor a successor to a block
        /// Can only be called after a call to _findAliasesAndAccesses for the root.
    BlockResult _processSuccessor(Index blockIndex);

        /// Process a block 
        /// Can only be called after a call to _findAliasesAndAccesses for the root.
    BlockResult _processBlock(Index blockIndex);

        /// Process all the locations in the function (locations must be ordered by root)
    void _processLocationsInFunction(const Location* start, Count count);

        /// All locations must be to the same root
    void _processRoot(const Location* locations, Count count);

        // Add a live end instruction at the start of block, referencing the root 'root'.
    void _addLiveRangeEndAtBlockStart(IRBlock* block, IRInst* root);

        /// Find all the aliases and accesses to the root
        /// The information is stored in m_accessSet and m_aliases
    void _findAliasesAndAccesses(IRInst* root);

        /// Add a result for the block
        /// Allows for promotion if there is already a result
    BlockResult _addBlockResult(Index blockIndex, BlockResult result);

    RefPtr<IRDominatorTree> m_dominatorTree;    ///< The dominator tree for the current function

    IRLiveRangeStart* m_rootLiveStart = nullptr;
    IRBlock* m_rootLiveStartBlock = nullptr;

    IRInst* m_root = nullptr;
    IRBlock* m_rootBlock = nullptr;                 ///< The block the root is in
    
    List<IRInst*> m_aliases;                        ///< A list of instructions that alias to the root

    Dictionary<IRBlock*, Index> m_blockIndexMap;    ///< Map from a block to an index

    List<BlockInfo> m_blockInfos;                   ///< Information about blocks

    List<InstOrder> m_instOrders;                   ///< Instruction of interests location, and ordering

    List<IRLiveRangeStart*> m_liveRangeStarts;      ///< Live range starts for a root

    Dictionary<IRInst*, Index> m_instToOrderIndex;  ///< Maps from an instruction to it's entry in m_instOrders 

    IRModule* m_module;
    SharedIRBuilder m_sharedBuilder;
    IRBuilder m_builder;
};

void LivenessContext::_addLiveRangeEndAtBlockStart(IRBlock* block, IRInst* root)
{
    // Insert before the first ordinary inst
    auto inst = block->getFirstOrdinaryInst();
    SLANG_ASSERT(inst);

    m_builder.setInsertLoc(IRInsertLoc::before(inst));

    // Add the live end inst
    m_builder.emitLiveRangeEnd(root);
}

LivenessContext::BlockResult LivenessContext::_addBlockResult(Index blockIndex, BlockResult result)
{
    auto& currentResult = m_blockInfos[blockIndex].result;
    // Check we can promote
    SLANG_ASSERT(canPromote(currentResult, result));
    currentResult = result;
    return result;
}

LivenessContext::BlockResult LivenessContext::_processSuccessor(Index blockIndex)
{
    auto& blockInfo = m_blockInfos[blockIndex];

    // Check if there is already a result for this block. 
    // If there is just return that.
    auto result = blockInfo.result;
    if (result != BlockResult::NotVisited)
    {
        return result;
    }

    auto block = blockInfo.block;

    // If the block is *not* dominated by the root block, we know it can't 
    // end liveness. 
    // Return that it is not dominated, and add to the cache for the block
    if (!m_dominatorTree->properlyDominates(m_rootBlock, block))
    {
        return _addBlockResult(blockIndex, BlockResult::NotDominated);
    }

    // Mark that it is visited
    _addBlockResult(blockIndex, BlockResult::Visited);

    // Else process the block to try and find the last used instruction
    return _processBlock(blockIndex);
}

LivenessContext::BlockResult LivenessContext::_processBlock(Index blockIndex)
{
    auto& blockInfo = m_blockInfos[blockIndex];
    const auto block = blockInfo.block;
   
    const Index orderEnd = blockInfo.orderStart + blockInfo.orderCount;

    Index accessCount = blockInfo.orderCount - blockInfo.startsCount;
    Index startsCount = blockInfo.startsCount;

    Index orderIndex = blockInfo.orderStart;

    if (block == m_rootLiveStartBlock)
    {
        // Must be > 0 because it has the rootLiveStart!
        SLANG_ASSERT(startsCount > 0);

        for (; orderIndex < orderEnd; ++orderIndex)
        {
            auto inst = m_instOrders[orderIndex].inst;

            const auto op = inst->getOp();

            if (op == kIROp_LiveRangeStart)
            {
                startsCount--;
                if (inst == m_rootLiveStart)
                {
                    ++orderIndex;
                    break;
                }
            }
            else
            {
                // If it's not a live range start it must be an access
                --accessCount;
            }
        }
    }

    // If there are any remaining starts, the last access before that start is the result
    if (startsCount > 0)
    {
        IRInst* lastAccess = nullptr;
        for (; orderIndex < orderEnd; ++orderIndex)
        {
            auto inst = m_instOrders[orderIndex].inst;
            const auto op = inst->getOp();
            if (op == kIROp_LiveRangeStart)
            {
                if (lastAccess)
                {
                    // Just add end of scope after the inst 
                    m_builder.setInsertLoc(IRInsertLoc::after(lastAccess));

                    // Add the live end inst
                    m_builder.emitLiveRangeEnd(m_root);
                    return _addBlockResult(blockIndex, BlockResult::Found);
                }
                else
                {
                    // We didn't find anything
                    return _addBlockResult(blockIndex, BlockResult::NotFound);
                }
            }

            // Must be an access
            lastAccess = inst;
        }
    }

    // Zero initialize all the counts
    Index foundCounts[Index(BlockResult::CountOf)] = { 0 };

    // Find all the successors for this block
    auto successors = block->getSuccessors();

    const Index successorCount = successors.getCount();

    // TODO(JS): 
    // We could just use a stack of results, and not need to allocate, but lets just allocate here to keep it simple

    List<SuccessorResult> successorResults;       ///< Storage for results from successors

    // We need to store of the results for successors
    successorResults.setCount(successorCount);

    {
        auto cur = successors.begin();
        for (Index i = 0; i < successorCount; ++i, ++cur)
        {
            auto succ = *cur;

            SuccessorResult successorResult;

            successorResult.blockIndex = m_blockIndexMap[succ];

            // Process the successor
            successorResult.result = _processSuccessor(successorResult.blockIndex);

            // Store the result
            successorResults[i] = successorResult;

            // Change counts depending on the result
            foundCounts[Index(successorResult.result)]++;
        }
    }

    const Index foundCount = foundCounts[Index(BlockResult::Found)];
    const Index notFoundCount = foundCounts[Index(BlockResult::NotFound)];

    const Index otherCount = successorCount - (foundCount + notFoundCount);

    // If one or more of the successors (or successors of successors),
    // was found to have the last access, we need to mark the end of scope
    // at the start of any other paths (which are dominated).
    if (foundCount > 0)
    {
        // If all successors have result, or are not dominated
        if (foundCount + otherCount == successorCount)
        {
            return _addBlockResult(blockIndex, BlockResult::Found);
        }

        for (const auto& successorResult : successorResults)
        {
            if (successorResult.result == BlockResult::NotFound)
            {
                const auto successorBlockIndex = successorResult.blockIndex;
                _addLiveRangeEndAtBlockStart(m_blockInfos[successorBlockIndex].block, m_root);
                _addBlockResult(successorBlockIndex, BlockResult::Found);
            }
        }

        // This block, can now be marked as found 
        return _addBlockResult(blockIndex, BlockResult::Found);
    }

    if (orderIndex < orderEnd)
    {
        // Since we cant have any starts, anything afterwards must be accesses
        // and the last of those accesses must therefore be the last acces

        const auto lastAccess = m_instOrders[orderEnd - 1].inst;

        // Just add end of scope after the inst 
        m_builder.setInsertLoc(IRInsertLoc::after(lastAccess));

        // Add the live end inst
        m_builder.emitLiveRangeEnd(m_root);
        return _addBlockResult(blockIndex, BlockResult::Found);
    }

    // Didn't find any accesses in this block
    return _addBlockResult(blockIndex, BlockResult::NotFound);    
}

void LivenessContext::_findAliasesAndAccesses(IRInst* root)
{
    m_aliases.clear();

    // Add the root to the list of aliases, to start lookup
    m_aliases.add(root);

    // The challenge here is to try and determine when a root is no longer accessed, and so is no longer live
    //
    // Note that a root can be accessed directly, but also through `aliases`. For example if the root is a structure,
    // a pointer to a field in the root would be an alias.
    // 
    // In terms of liveness, the only accesses that are important are loads. This is because if the last operation on 
    // a root/alias is a store, if it is never read it will never be seen, so in effect doesn't matter.
    //
    // The algorithm here works as follows
    // 0) Prior to this function, a dominator tree is built for the function
    //    This is usefuly because variables defined in block A, is only accessible to blocks *dominated* by A
    // 1) Deterime all of the aliases, and accesses to the root
    //    Add all the access instructions into m_accessSet
    //    Add all the aliases to m_aliases

    for (Index i = 0; i < m_aliases.getCount(); ++i)
    {
        IRInst* alias = m_aliases[i];

        // Find all the uses of this alias/root
        for (IRUse* use = alias->firstUse; use; use = use->nextUse)
        {
            IRInst* cur = use->getUser();
            IRInst* base = nullptr;

            IRBlock* block = as<IRBlock>(cur->getParent());
            if (!block)
            {
                continue;
            }

            AccessType accessType = AccessType::None;

            // We want to find instructions that access the root
            switch (cur->getOp())
            {
                case kIROp_getElementPtr:
                {
                    base = static_cast<IRGetElementPtr*>(cur)->getBase();
                    accessType = AccessType::Alias;
                    break;
                }
                case kIROp_FieldAddress:
                {
                    base = static_cast<IRFieldAddress*>(cur)->getBase();
                    accessType = AccessType::Alias;
                    break;
                }
                case kIROp_getAddr:
                {
                    IRGetAddress* getAddr = static_cast<IRGetAddress*>(cur);
                    base = getAddr->getOperand(0);
                    accessType = AccessType::Alias;
                    break;
                }
                case kIROp_Call:
                {
                    // TODO(JS): This is arguably too conservative. 
                    // 
                    // Depending on how the parameter is used - in, out, inout changes the interpretation
                    // 
                    // *If we are talking about a real "pointer" then this is basically the general case again.
                    //     the callee  could store  the pointer into a global, dictionary, whatever.
                    //
                    // * If we are talking about an "address", then this is constrained by our language rules,
                    //    and we kind of need to find the type of the matching parameter :
                    //   * If the parameter is an `out` parameter, this is basically like a `store`
                    //   * If the parameter is an `inout` parameter, this is basically like a `load`

                    // We can assume it accesses the base
                    base = alias;
                    accessType = AccessType::Access;
                    break;
                }
                case kIROp_Load:
                {
                    // We only care about loads in terms of identifying liveness
                    base = static_cast<IRLoad*>(cur)->getPtr();
                    accessType = AccessType::Access;
                    break;
                }
                case kIROp_Store:
                {
                    // In terms of liveness, stores can be ignored
                    break;
                }
                case kIROp_getElement:
                case kIROp_FieldExtract:
                {
                    // These will never take place on the var which is accessed through a pointer, so can be ignored
                    break;
                }
                default: break;
            }

            // Make sure the access is through the alias (as opposed to some other part of the instructions 'use')
            if (base == alias)
            {
                switch (accessType)
                {
                    case AccessType::Alias:
                    {
                        // Add this instruction to the aliases
                        m_aliases.add(cur);
                        break;
                    }
                    case AccessType::Access:
                    {
                        InstOrder order;
                        order.inst = cur;
                        order.index = -1;
                        order.block = as<IRBlock>(cur->getParent());
                        break;
                    }
                    default: break;
                }
            }
        }
    }
}

void LivenessContext::_findAndEmitRangeEnd(IRLiveRangeStart* liveRangeStart)
{
    // Reset the result
    {
        for (auto& blockInfo : m_blockInfos)
        {
            blockInfo.result = BlockResult::NotVisited;
        }
    }

    // Store root information, so don't have to pass around methods
    m_rootLiveStart = liveRangeStart;
    m_rootLiveStartBlock = as<IRBlock>(liveRangeStart->getParent());
    m_root = liveRangeStart->getReferenced();
    m_rootBlock = as<IRBlock>(m_root->parent);

    // If either of these asserts fail it probably means there hasn't been a call
    // to `_findAliasesAndAccesses` which is required before this function can be called.
    // 
    // There must be at least one alias (the root itself!)
    SLANG_ASSERT(m_aliases.getCount() > 0);
    // The first alias should be the root itself
    SLANG_ASSERT(m_aliases[0] == m_root);

    // Now we want to find the last access in the graph of successors
    //
    // This works by recursively starting from the block where the variable is defined, walking depth first the graph of 
    // successors. We cache the results in m_blockResults
    //
    // There is an extra caveat around the dominator tree. In principal a variable in block A is accessible by any block that is 
    // dominated by A. It's actually more restricted than this - because IR has other rules that provide more tight scoping. 
    // The extra information can be seen in a loop instruction also indicating the break and continue blocks.
    // 
    // If we just traversed the successors, if there is a loop we'd end up in an infinite loop. We can partly avoid this because 
    // we know that the root is only available in blocks dominated by the root. There is also the scenario where there is a loop 
    // in blocks within the dominator tree. That is handled by marking 'Visited' when a final result isn't known, but we want to 
    // detect a loop. In most respect Visited behaves in the same manner as NotDominated.

    {
        const Index rootBlockIndex = m_blockIndexMap[m_rootBlock];

        // Mark the root as visited to stop an infinite loop
        _addBlockResult(rootBlockIndex, BlockResult::Visited);

        // Recursively find results
        auto foundResult = _processBlock(rootBlockIndex);

        if (foundResult == BlockResult::NotFound)
        {
            // Means there is no access to this variable(!)
            // Which means we can end the scope, after the the start scope
            m_builder.setInsertLoc(IRInsertLoc::after(m_rootLiveStart));
            m_builder.emitLiveRangeEnd(m_root);
        }
    }

    // Set back to nullptr for safety
    m_rootLiveStart = nullptr;
    m_root = nullptr;
    m_rootBlock = nullptr;
}

void LivenessContext::_processRoot(const Location* locations, Count locationsCount)
{
    if (locationsCount <= 0)
    {
        return;
    }

    // Reset the order range for all blocks
    for (auto& info : m_blockInfos)
    {
        info.orderStart = 0;
        info.orderCount = 0;
        info.startsCount = 0;
    }

    m_instOrders.clear();

    auto root = locations[0].root;

    // Add all the live starts
    m_liveRangeStarts.setCount(locationsCount);
    for (Index i = 0; i < locationsCount; ++i)
    {
        const auto& location = locations[i];
        SLANG_ASSERT(location.root == root);

        // Add the start location
        m_builder.setInsertLoc(location.startLocation);
        // Emit the range start
        auto liveStart = m_builder.emitLiveRangeStart(location.root);

        // Save the start
        m_liveRangeStarts[i] = liveStart;

        // Add to the order list
        InstOrder order;
        order.inst = liveStart;
        order.index = -1;
        order.block = as<IRBlock>(liveStart->getParent());

        m_instOrders.add(order);
    }

    // Find all of the aliases and access to this root
    _findAliasesAndAccesses(root);

    // Okay we now have InstOrder list, we can order by block
    m_instOrders.sort([&](const InstOrder& a, const InstOrder& b) -> bool { return a.block < b.block; });

    {
        const Count ordersCount = m_instOrders.getCount();

        Index start = 0;
        while (start < ordersCount)
        {
            auto block = m_instOrders[start].block;

            // Find the end
            Index end = start + 1;
            for(; end < ordersCount && m_instOrders[end].block == block; ++end);

            const Count count = end - start;

            Count startsCount = 0;

            // If we find more than one instruction, we need to sort the instructions with InstOrder entries, such that they are
            // in instruction order.
            if (count > 1)
            {
                // Lets work out the indices of these instructions
                m_instToOrderIndex.Clear();
                for (Index i = start; i < end; ++i)
                {
                    m_instToOrderIndex.Add(m_instOrders[i].inst, i);
                }

                Index index = 0;
                Count remaining = count;

                for (IRInst* cur = block->getFirstChild(); cur; cur = cur->getNextInst(), ++index)
                {
                    // If there is an order associated with this instruction, set it's index
                    auto orderIndexPtr = m_instToOrderIndex.TryGetValue(cur);
                    if (orderIndexPtr)
                    {
                        // Increase the start count
                        startsCount += Index(cur->getOp() == kIROp_LiveRangeStart);

                        auto& instOrder = m_instOrders[*orderIndexPtr];

                        // It can't bet set yet!
                        SLANG_ASSERT(instOrder.index == -1);

                        // Set it's index within the block
                        instOrder.index = index;
                        // If there are none left to find we are done
                        if (--remaining <= 0)
                        {
                            break;
                        }
                    }
                }

                // We can now order by index
                std::sort(m_instOrders.getBuffer() + start, m_instOrders.getBuffer() + end, [&](const InstOrder& a, const InstOrder& b) { return a.index < b.index; });
            }
            else
            {
                // Must be one
                SLANG_ASSERT(count == 1);
                // Update the starts count
                startsCount += Index(m_instOrders[start].inst->getOp() == kIROp_LiveRangeStart);
            }

            // Find the block index
            const Index blockIndex = m_blockIndexMap[block];

            // Set the range/startsCount
            auto& blockInfo = m_blockInfos[blockIndex];
            blockInfo.orderStart = start;
            blockInfo.orderCount = count;
            blockInfo.startsCount = startsCount;

            // next
            start = end;
        }
    }

    // Now we want to find all of the ends
    for (auto liveStart : m_liveRangeStarts)
    {
        // We want to process this root to find all of the ends
        _findAndEmitRangeEnd(liveStart);
    }
}

void LivenessContext::_processLocationsInFunction(const Location* locations, Count count)
{
    if (count <= 0)
    {
        return;
    }
    
    const auto func = locations[0].function;
    SLANG_UNUSED(func);
        
    // Set up the map from blocks to indices
    m_blockIndexMap.Clear();
    m_blockInfos.clear();
    {
        Index index = 0;
        for (auto block : func->getChildren())
        {
            IRBlock* blockInst = as<IRBlock>(block);
            m_blockIndexMap.Add(blockInst, index++);

            BlockInfo blockInfo; 
            blockInfo.block = blockInst;
            blockInfo.result = BlockResult::NotVisited;
            blockInfo.orderStart = -1;
            blockInfo.orderCount = 0;

            m_blockInfos.add(blockInfo);
        }
    }

    // Lets add all of the blocks 
    Index start = 0;
    while (start < count)
    {
        SLANG_ASSERT(locations[start].function == func);

        // Get the root at the start of this span
        IRInst*const root = locations[start].root;

        // Look for the end of the run of locations with the same root
        Index end = start + 1;
        for (; end < count && locations[end].root == root; ++end);

        // Process the root 
        _processRoot(locations + start, end - start);       

        // Set start to the beginning of the next run
        start = end;
    }
}

void LivenessContext::processLocations(const List<Location>& inLocations)
{
    List<Location> locations(inLocations);

    // Sort so we have in function order, and within a function in root order
    locations.sort([&](const Location& a, const Location& b) -> bool { return a.function < b.function || (a.function == b.function && a.root < b.root); });

    const auto locationCount = locations.getCount();

    Index start = 0;
    while (start < locationCount)
    {
        auto func = locations[start].function;
        Index end = start + 1;

        for (;end < locationCount && locations[end].function == func; ++end);

        // All of the locations from [start, end) are in the same function. Lets process all in one go...
        
        // Create the dominator tree, for the function
        m_dominatorTree = computeDominatorTree(func);

        // Process all of the locations in the function
        _processLocationsInFunction(locations.getBuffer() + start, end - start);

        // Look for next run
        start = end;
    }
}

} // anonymous

static void _processFunction(IRFunc* funcInst, List<LivenessLocation>& ioLocations)
{
    // If it has no body, then we are done
    if (funcInst->getFirstBlock() == nullptr)
    {
        return;
    }

    // Iterate through blocks in the function, looking for variables to live track
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        for (auto inst = block->getFirstChild(); inst; inst = inst->getNextInst())
        {
            // We look for var declarations.
            if (auto varInst = as<IRVar>(inst))
            {
                LivenessLocation location;

                location.function = funcInst;
                location.startLocation = IRInsertLoc::after(varInst);
                location.root = varInst;

                ioLocations.add(location);
            }
        }
    }
}

/* static */void LivenessUtil::locateVariables(IRModule* module, List<Location>& ioLocations)
{
    // When we process liveness, is prior to output for a target
    // So post specialization

    IRModuleInst* moduleInst = module->getModuleInst();

    for (IRInst* child : moduleInst->getChildren())
    {
        // We want to find all of the functions, and process them
        if (auto funcInst = as<IRFunc>(child))
        {
            // Then we want to look through their definition
            // inserting instructions that mark the liveness start/end
            _processFunction(funcInst, ioLocations);
        }
    }
}

/* static */void LivenessUtil::addLivenessRanges(IRModule* module, const List<Location>& inLocations)
{
    LivenessContext context(module);
    context.processLocations(inLocations);
}

} // namespace Slang
    