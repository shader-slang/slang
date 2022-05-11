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

/*
A helper class to enable using a backing array, used in a stack like manner. */
template <typename T>
class RAIIStackArray
{
public:
    ArrayView<T> getView() { return makeArrayView(m_list->getBuffer() + m_startIndex, m_list->getCount() - m_startIndex); }
    ConstArrayView<T> getConstView() const { return makeConstArrayView(m_list->getBuffer() + m_startIndex, m_list->getCount() - m_startIndex); }

    void setCount(Count count) { m_list->setCount(m_startIndex + count); }
    Count getCount() const { return m_list->getCount() - m_startIndex; }

    T& operator[](Index i) { return (*m_list)[m_startIndex + i]; }
    const T& operator[](Index i) const { return (*m_list)[m_startIndex + i]; }

    RAIIStackArray(List<T>* list):
        m_startIndex(list->getCount()),
        m_list(list)
    {
    }
    ~RAIIStackArray()
    {
        m_list->setCount(m_startIndex);
    }

    const Index m_startIndex;
    List<T>*const m_list;
};

struct LivenessContext
{
    typedef LivenessLocation Location;

    enum class BlockIndex : Index;

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
    
    struct BlockInfo
    {
        void init(IRBlock* inBlock) 
        { 
            block = inBlock; 
            reset(); 
        }
        void reset() 
        {
            result = BlockResult::NotVisited;
            runStart = 0;
            runCount = 0;
            lastInst = nullptr;
            instCount = 0;
        }
        void resetResult()
        {
            result = BlockResult::NotVisited;
        }

        IRBlock* block;             ///< The block associated with this index
        BlockResult result;         ///< The result for this block
        Index runStart;             ///< The start InstOrder index
        Count runCount;             ///< The count of the amount of InstOrder
        IRInst* lastInst;           ///< Last inst seen
        Count instCount;            ///< The total amount of start/access instruction seen in the block
    };

    struct SuccessorResult
    {
        BlockResult result;         ///< The result 
        BlockIndex blockIndex;      ///< The block index of the successor
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
    BlockResult _processSuccessor(BlockIndex blockIndex);

        /// Process a block 
        /// Can only be called after a call to _findAliasesAndAccesses for the root.
    BlockResult _processBlock(BlockIndex blockIndex);

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
    BlockResult _addBlockResult(BlockIndex blockIndex, BlockResult result);

    void _findBlockInstRuns();

        /// Adds an instruction that is an access to the root
    void _addAccessInst(IRInst* inst);
        /// Add a live range start 
    void _addStartInst(IRLiveRangeStart* inst) { _addInst(inst); }
        /// Add an instruction that is significant for liveness tracking - start or an access
    void _addInst(IRInst* inst);

        /// True if it's an instruction of interest and so will go within a run for a block
    bool _isRunInst(IRInst* inst);

        // Returns the index in the run of a start for the current root, else -1
    Index _indexOfRootStart(const ConstArrayView<IRInst*>& run);

    Index _findLastAccess(const ConstArrayView<IRInst*>& run);

        /// Find a liveRange end from a given inst
    IRLiveRangeEnd* findEndRootFromInst(IRInst* inst);

        /// Complete the block using the run, which can *cannot* contain a start for the current root
    BlockResult _completeBlock(BlockIndex blockIndex, const ConstArrayView<IRInst*>& run);

    BlockInfo* _getInfo(BlockIndex blockIndex) { return &m_blockInfos[Index(blockIndex)]; }

    ConstArrayView<IRInst*> _getRun(const BlockInfo* info) 
    {
        IRInst*const* buffer = m_instRuns.getBuffer();
        return ConstArrayView<IRInst*>(buffer + info->runStart, info->runCount);
    }

    RefPtr<IRDominatorTree> m_dominatorTree;    ///< The dominator tree for the current function

    IRLiveRangeStart* m_rootLiveStart = nullptr;
    IRBlock* m_rootLiveStartBlock = nullptr;

    IRInst* m_root = nullptr;
    IRBlock* m_rootBlock = nullptr;                 ///< The block the root is in
    
    List<SuccessorResult> m_successorResults;       ///< Storage for successor results

    List<IRInst*> m_aliases;                        ///< A list of instructions that alias to the root

    HashSet<IRInst*> m_accessSet;                   ///< If instruction is in set it is an access 

    Dictionary<IRBlock*, BlockIndex> m_blockIndexMap;    ///< Map from a block to a block index
    List<BlockInfo> m_blockInfos;                   ///< Information about blocks

    List<IRInst*> m_instRuns;                       ///< Instructions of interest in order. Indexed into via BlockInfo [runStart, runStart + runCount)

    List<IRLiveRangeStart*> m_liveRangeStarts;      ///< Live range starts for a root

    IRModule* m_module;
    SharedIRBuilder m_sharedBuilder;
    IRBuilder m_builder;
};

void LivenessContext::_addLiveRangeEndAtBlockStart(IRBlock* block, IRInst* root)
{
    // Insert before the first ordinary inst
    auto inst = block->getFirstOrdinaryInst();
    SLANG_ASSERT(inst);

    // Only add the end if couldn't find one
    if (!findEndRootFromInst(inst))
    {
        m_builder.setInsertLoc(IRInsertLoc::before(inst));

        // Add the live end inst
        m_builder.emitLiveRangeEnd(root);
    }
}

LivenessContext::BlockResult LivenessContext::_addBlockResult(BlockIndex blockIndex, BlockResult result)
{
    auto& currentResult = _getInfo(blockIndex)->result;
    // Check we can promote
    SLANG_ASSERT(canPromote(currentResult, result));
    currentResult = result;
    return result;
}

LivenessContext::BlockResult LivenessContext::_processSuccessor(BlockIndex blockIndex)
{
    auto blockInfo = _getInfo(blockIndex);

    // Check if there is already a result for this block. 
    // If there is just return that.
    auto result = blockInfo->result;
    if (result != BlockResult::NotVisited)
    {
        return result;
    }

    auto block = blockInfo->block;

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

Index LivenessContext::_indexOfRootStart(const ConstArrayView<IRInst*>& run)
{
    const Count count = run.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (auto liveStart = as<IRLiveRangeStart>(run[i]))
        {
            if (liveStart->getReferenced() == m_root)
            {
                return i;
            }
        }
    }
    return -1;
}

Index LivenessContext::_findLastAccess(const ConstArrayView<IRInst*>& run)
{
    for (Index i = run.getCount() - 1; i >= 0; --i)
    {
        // If it's not a live start, it must be an access
        if (as<IRLiveRangeStart>(run[i]) == nullptr)
        {
            // Check it really is an access inst..
            SLANG_ASSERT(_isRunInst(run[i]));
            return i;
        }
    }
    return -1;
}

IRLiveRangeEnd* LivenessContext::findEndRootFromInst(IRInst* cur)
{
    for (; cur ; cur = cur->getNextInst())
    {
        // We are only looking for ends
        IRLiveRangeEnd* end = as<IRLiveRangeEnd>(cur);
        if (end == nullptr)
        {
            return nullptr;
        }
        // If the end is for the root we've found it
        if (end->getReferenced() == m_root)
        {
            return end;
        }
    }
    return nullptr;
}

LivenessContext::BlockResult LivenessContext::_completeBlock(BlockIndex blockIndex, const ConstArrayView<IRInst*>& run)
{
    // We can't have a root start in the run!
    SLANG_ASSERT(_indexOfRootStart(run) < 0);

    // Look for the last access
    const auto lastAccessIndex = _findLastAccess(run);

    // If we found one, that is the end of the range
    if (lastAccessIndex >= 0)
    {
        IRInst* lastAccessInst = run[lastAccessIndex];

        // See if there is an end already after this 
        // Skip the access (we will want our end after the access)
        if (!findEndRootFromInst(lastAccessInst->getNextInst()))
        {
            // Just add end of scope after the inst 
            m_builder.setInsertLoc(IRInsertLoc::after(lastAccessInst));
            // Add the live end inst
            m_builder.emitLiveRangeEnd(m_root);
        }

        // Add the result
        return _addBlockResult(blockIndex, BlockResult::Found);
    }

    // We didn't find anything, so mark as not found
    return _addBlockResult(blockIndex, BlockResult::NotFound);
}

LivenessContext::BlockResult LivenessContext::_processBlock(BlockIndex blockIndex)
{
    auto blockInfo = _getInfo(blockIndex);
    const auto block = blockInfo->block;
   
    auto run = _getRun(blockInfo);

    if (block == m_rootLiveStartBlock)
    {
        // We need to fix the run to be *after* the start
        const Index startIndex = run.indexOf(m_rootLiveStart);
        SLANG_ASSERT(startIndex >= 0);

        // Skip to the next index
        const Index nextIndex = startIndex + 1;
        // Fix the run
        run = ConstArrayView<IRInst*>(run.getBuffer() + nextIndex, run.getCount() - nextIndex);
    }

    // If there is another start to the same root, we can't traverse to other blocks, and the last access 
    // in this block must be the result
    {
        const Index startIndex = _indexOfRootStart(run);
        if (startIndex >= 0)
        {
            // Fix the run
            run = ConstArrayView<IRInst*>(run.getBuffer(), startIndex);
            // Complete the block with this run
            return _completeBlock(blockIndex, run);
        }
    }

    // Zero initialize all the counts
    Index foundCounts[Index(BlockResult::CountOf)] = { 0 };

    // Find all the successors for this block
    auto successors = block->getSuccessors();

    const Index successorCount = successors.getCount();

    // Set up space to store successor results
    RAIIStackArray<SuccessorResult> successorResults(&m_successorResults);
    successorResults.setCount(successorCount);

    {
        auto cur = successors.begin();
        for (Index i = 0; i < successorCount; ++i, ++cur)
        {
            auto succ = *cur;

            const auto successorBlockIndex = m_blockIndexMap[succ];

            // NOTE! Care is needed around successorResults, because _processorSuccessor may cause the underlying list 
            // to be reallocated. 
            // If we always access through successorResults (ie RAIIStackArray type), things will be fine though.
            
            // Process the successor
            SuccessorResult successorResult;
            successorResult.blockIndex = successorBlockIndex;
            successorResult.result = _processSuccessor(successorBlockIndex);

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

        for (const auto& successorResult : successorResults.getConstView())
        {
            if (successorResult.result == BlockResult::NotFound)
            {
                const auto successorBlockIndex = successorResult.blockIndex;
                _addLiveRangeEndAtBlockStart(_getInfo(successorBlockIndex)->block, m_root);
                _addBlockResult(successorBlockIndex, BlockResult::Found);
            }
        }

        // This block, can now be marked as found 
        return _addBlockResult(blockIndex, BlockResult::Found);
    }

    return _completeBlock(blockIndex, run);
}

void LivenessContext::_addInst(IRInst* inst)
{
    // Get the block it's in
    auto block = as<IRBlock>(inst->getParent());

    // Get the index to get the info
    auto blockIndex = m_blockIndexMap[block];
    auto blockInfo = _getInfo(blockIndex);

    // Increase the count
    ++blockInfo->instCount;

    // Record that this is an instruction of interest for this block
    //
    // This only really exists to capture the scenario of only having one inst in a block, so we can just overwrite what's
    // already there.
    blockInfo->lastInst = inst;
}

void LivenessContext::_addAccessInst(IRInst* inst)
{
    // Add to the access set
    m_accessSet.Add(inst);

    // Add the instruction to the block info
    _addInst(inst);
}

void LivenessContext::_findAliasesAndAccesses(IRInst* root)
{
    // Clear all the aliases
    m_aliases.clear();
    // Clear the access set
    m_accessSet.Clear();

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
                        _addAccessInst(cur);
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
    for (auto& blockInfo : m_blockInfos)
    {
        blockInfo.resetResult();
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
        const BlockIndex rootStartBlockIndex = m_blockIndexMap[m_rootLiveStartBlock];

        // Mark the root as visited to stop an infinite loop
        _addBlockResult(rootStartBlockIndex, BlockResult::Visited);

        // Recursively find results
        auto foundResult = _processBlock(rootStartBlockIndex);

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
    m_rootLiveStartBlock = nullptr;
}

bool LivenessContext::_isRunInst(IRInst* inst)
{
    const auto op = inst->getOp();

    // If it's a live start then we need to track
    if (op == kIROp_LiveRangeStart)
    {
        return true;
    }

    // NOTE!
    // These are the only ops *currently* that indicate an access.
    // Has to be consistent with `_findAliasesAndAccesses`
    if (op == kIROp_Call || 
        op == kIROp_Load || 
        op == kIROp_Call)
    {
        // Just because it's the right type *doesn't* mean it's an access, it has to also 
        // be in the access set
        return m_accessSet.Contains(inst);
    }

    return false;
}


void LivenessContext::_findBlockInstRuns()
{
    for (auto& blockInfo : m_blockInfos)
    {
        if (blockInfo.instCount == 0)
        {
            // Nothing to do if it's empty
            SLANG_ASSERT(blockInfo.runCount == 0);
        }
        else if (blockInfo.instCount == 1)
        {
            // This is the easy case, since we don't need to determine the order of the instructions
            blockInfo.runStart = m_instRuns.getCount();
            blockInfo.runCount = 1;
            SLANG_ASSERT(blockInfo.lastInst);
            m_instRuns.add(blockInfo.lastInst);
            continue;
        }
        else
        {
            // TODO(JS):
            // NOTE That we don't need to keep all accesses in the run, only the last accesses 
            // prior to a start/end.
            //
            // For now we just add them all.

            const auto start = m_instRuns.getCount();
            blockInfo.runStart = start;
            blockInfo.runCount = blockInfo.instCount;

            m_instRuns.setCount(start + blockInfo.instCount);
            IRInst** dst = m_instRuns.getBuffer() + start;

            // Find all of the instructions of interest in order
            auto block = blockInfo.block;

            for (auto inst : block->getChildren())
            {
                if (_isRunInst(inst))
                {
                    *dst++ = inst;
                    if (dst == m_instRuns.end())
                    {
                        break;
                    }
                }
            }
            SLANG_ASSERT(dst == m_instRuns.end());
        }

        // The run count and the inst count must match at this point
        SLANG_ASSERT(blockInfo.runCount == blockInfo.instCount);
    }
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
        info.reset();
    }
    m_instRuns.clear();
    
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

        _addStartInst(liveStart);
    }

    // Find all of the aliases and access to this root
    _findAliasesAndAccesses(root);

    // Find the runs of 'instructions of interest' (accesses/starts) for all the blocks
    _findBlockInstRuns();

    // Now we want to find all of the ends
    for (auto liveStart : m_liveRangeStarts)
    {
        // We want to process this RangeStart for the root, to find all of the ends
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
    
    // Create the dominator tree, for the function
    m_dominatorTree = computeDominatorTree(func);

    // Set up the map from blocks to indices
    m_blockIndexMap.Clear();
    m_blockInfos.clear();

    {
        Index index = 0;
        for (auto block : func->getChildren())
        {
            IRBlock* blockInst = as<IRBlock>(block);
            m_blockIndexMap.Add(blockInst, BlockIndex(index++));
            BlockInfo blockInfo;
            blockInfo.init(blockInst);
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
                // Set the livness start to be after the var
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
    