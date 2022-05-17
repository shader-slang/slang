#include "slang-ir-liveness.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

#include "slang-ir-dominators.h"

namespace Slang
{

/* 
Discussion 
==========

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

    // NOTE! Care must be taken changing the order. 
    // canPromote checks if a result can be 'promoted'.
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
    
        /// Block info (indexed via BlockIndex), that is valid across analysing liveness of a root
    struct BlockInfo
    {
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

        BlockResult result;         ///< The result for this block
        Index runStart;             ///< The start index in m_instRuns index. This defines a instruction of interest in order in a block.
        Count runCount;             ///< The count of the amount insts in the run
        IRInst* lastInst;           ///< Last inst seen
        Count instCount;            ///< The total amount of start/access instruction seen in the block
    };

        /// Block info (indexed via BlockIndex), that is valid across a function
    struct FunctionBlockInfo
    {
        void init(IRBlock* inBlock)
        {
            block = inBlock;
            successorsStart = 0;
            successorsCount = 0;
        }

        IRBlock* block;
        Index successorsStart;      ///< Indexes into block successors
        Count successorsCount;
    };

        /// Process all the locations
    void processLocations(const List<Location>& locations);
    
    LivenessContext(IRModule* module):
        m_module(module)
    {
        // Disable warning if not used
        SLANG_UNUSED(&LivenessContext::_isAnyRunInst);
        
        m_sharedBuilder.init(module);
        m_builder.init(m_sharedBuilder);
    }

        /// For a given live range start find it's end/s and insert a LiveRangeEnd/s
        /// Can only be called after a call to _findAliasesAndAccesses for the root.
    void _findAndEmitRangeEnd(IRLiveRangeStart* liveStart);

        /// Process a successor to a block
        /// Can only be called after a call to _findAliasesAndAccesses for the root.
    BlockResult _processSuccessor(BlockIndex blockIndex);

        /// Process a block 
        /// Can only be called after a call to _findAliasesAndAccesses for the root.
    BlockResult _processBlock(BlockIndex blockIndex, const ConstArrayView<IRInst*>& run);

        /// Process all the locations in the function 
        /// NOTE: All locations must be to the same function, and ordered by root. 
    void _processLocationsInFunction(const Location* start, Count count);

        /// Process a root
        /// NOTE: All locations must be to the same root
    void _processRoot(const Location* locations, Count count);

        /// Find all the aliases and accesses to the root
        /// The information is stored in m_accessSet and m_aliases
    void _findAliasesAndAccesses(IRInst* root);

        /// Add a result for the block
        /// Allows for promotion if there is already a result
    BlockResult _addBlockResult(BlockIndex blockIndex, BlockResult result);

        /// Find the runs of 'important instructions' all of the blocks
        /// 'important instructions are root starts, and accesses to the root
        /// The run stores these instructions in the order they appear in the block within the run.
    void _findInstRunsForBlocks();

        /// Adds an instruction that is an access to the root
    void _addAccessInst(IRInst* inst);
        /// Add a live range start 
    void _addStartInst(IRLiveRangeStart* inst) { _addInst(inst); }
        /// Add an 'important instruction' that is significant for liveness tracking and so will be added to run
    void _addInst(IRInst* inst);

        /// True if it's an instruction of interest and so will go within a run for a block
    bool _isNormalRunInst(IRInst* inst);

        /// Returns true if is a normal run inst, or if is a return that accesses
    bool _isAnyRunInst(IRInst* inst);

        // Returns the index in the run of a start for the current root, else -1
    Index _indexOfRootStart(const ConstArrayView<IRInst*>& run);

        /// Returns the last index within the run which is an access, else -1
    Index _findLastAccess(const ConstArrayView<IRInst*>& run);

        /// Adds an LiveRangeEnd for the root after `inst` if there isn't one there already
    void _maybeAddEndAfterInst(IRInst* inst);

    void _maybeAddEndBeforeInst(IRInst* inst);

        /// Maybe insert an end after the instruction
    void _maybeAddEndAfterRunIndex(BlockIndex blockIndex, const ConstArrayView<IRInst*>& run, Index runIndex);

        // Add a live end instruction at the start of block, referencing the root
    void _maybeAddEndAtBlockStart(BlockIndex blockIndex);

        /// Look from inst for an LiveEndRange to the root.
    IRInst* _findRootEnd(IRInst* inst);

        /// Complete the block using the run, which can *cannot* contain the current root start
    BlockResult _completeBlock(BlockIndex blockIndex, const ConstArrayView<IRInst*>& run);

        /// Get block info
    BlockInfo* _getBlockInfo(BlockIndex blockIndex) { return &m_blockInfos[Index(blockIndex)]; }

        /// Get the block from the index
    IRBlock* _getBlock(BlockIndex blockIndex) const { return m_functionBlockInfos[Index(blockIndex)].block; }

        /// True if the terminator can be considered an access
        /// This allows us to elide a scope end if the root is returned 
    bool _isAccessTerminator(IRTerminatorInst* terminator);

        /// Gets the instructions of interest for this info, in the order they appear within the block
    ConstArrayView<IRInst*> _getRun(const BlockInfo* info) 
    {
        IRInst*const* buffer = m_instRuns.getBuffer();
        return ConstArrayView<IRInst*>(buffer + info->runStart, info->runCount);
    }
        /// Gets all of the successors for the blockIdnex
    ConstArrayView<BlockIndex> _getSuccessors(BlockIndex blockIndex)
    {
        const auto& info = m_functionBlockInfos[Index(blockIndex)];
        return makeConstArrayView(m_blockSuccessors.getBuffer() + info.successorsStart, info.successorsCount);
    }

    RefPtr<IRDominatorTree> m_dominatorTree;        ///< The dominator tree for the current function

    IRLiveRangeStart* m_rootLiveStart = nullptr;    ///< The current live start for the root
    IRBlock* m_rootLiveStartBlock = nullptr;        ///< The current block for the live start

    IRInst* m_root = nullptr;                       ///< The current root 
    IRBlock* m_rootBlock = nullptr;                 ///< The block the root is in
    
    List<BlockResult> m_successorResults;           ///< Storage for successor results

    List<IRInst*> m_aliases;                        ///< A list of instructions that alias to the root

    HashSet<IRInst*> m_accessSet;                   ///< If instruction is in set it is an `access` indicating it must be live at least up to this instruction 

    Dictionary<IRBlock*, BlockIndex> m_blockIndexMap;   ///< Map from a block to a block index
    List<BlockInfo> m_blockInfos;                       ///< Information about blocks, for the current root
    List<FunctionBlockInfo> m_functionBlockInfos;       ///< Information about blocks across the current function
    List<BlockIndex> m_blockSuccessors;                 ///< Successors for a blocks, accessed via 

    List<IRInst*> m_instRuns;                       ///< Instructions of interest in order. Indexed into via BlockInfo [runStart, runStart + runCount)

    List<IRLiveRangeStart*> m_liveRangeStarts;      ///< Live range starts for a root

    IRModule* m_module;
    SharedIRBuilder m_sharedBuilder;
    IRBuilder m_builder;
};

void LivenessContext::_maybeAddEndAtBlockStart(BlockIndex blockIndex)
{
    auto block = _getBlock(blockIndex);

    // Insert before the first ordinary inst
    auto inst = block->getFirstOrdinaryInst();
    // A block has to end with a terminator... so must always be an ordinary inst, if there is a function body
    SLANG_ASSERT(inst);
    _maybeAddEndBeforeInst(inst);
}

LivenessContext::BlockResult LivenessContext::_addBlockResult(BlockIndex blockIndex, BlockResult result)
{
    auto& currentResult = _getBlockInfo(blockIndex)->result;
    // Check we can promote
    SLANG_ASSERT(canPromote(currentResult, result));
    currentResult = result;
    return result;
}

LivenessContext::BlockResult LivenessContext::_processSuccessor(BlockIndex blockIndex)
{
    auto blockInfo = _getBlockInfo(blockIndex);
    
    // Check if there is already a result for this block. 
    // If there is just return that.
    auto result = blockInfo->result;

    switch (result)
    {
        case BlockResult::NotVisited: 
        {
            // If not visited we need to process
            break;
        }
        case BlockResult::Visited:
        {
            const auto block = _getBlock(blockIndex);

            // If visited, it can't have a domination issue 
            // Unless it is the start block (the block containing live start) *and* the root is 
            // in the block. 
            // The live start can only be after the var, because the var is only in scope then.
            //
            
            // We need to check if we are in the live start block, as we then need to process 
            // up until the live start.
            if (block == m_rootLiveStartBlock)
            {
                // We want the run to search to go from the start up to *this specific* liveness start
                // (as opposed to any liveness start for the root)
                auto run = _getRun(blockInfo);

                // We need to fix the run to be *after* this specific start
                const Index startIndex = run.indexOf(m_rootLiveStart);
                SLANG_ASSERT(startIndex >= 0);

                // We want to run all the way up to the start
                return _processBlock(blockIndex, run.head(startIndex));
            }

            // Otherwise just return result
            return result;
        }
        default:    
        {
            // Otherwise just return result
            return result;
        }
    }

    const auto block = _getBlock(blockIndex);

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
    return _processBlock(blockIndex, _getRun(blockInfo));
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
        auto inst = run[i];
        // If it's not a live start, it must be an access
        if (as<IRLiveRangeStart>(inst) == nullptr)
        {
            // Check it really is an access inst..
            SLANG_ASSERT(_isAnyRunInst(inst));
            return i;
        }
    }
    return -1;
}

IRInst* LivenessContext::_findRootEnd(IRInst* inst)
{
    for (auto cur = inst; cur; cur = cur->getNextInst())
    {
        IRLiveRangeEnd* end = as<IRLiveRangeEnd>(cur);
        if (end == nullptr)
        {
            break;
        }

        // If we hit an end which is already the root, then we don't need to add an
        // end of the root
        if (end->getReferenced() == m_root)
        {
            return cur;
        }
    }

    return nullptr;
}

void LivenessContext::_maybeAddEndAfterRunIndex(BlockIndex blockIndex, const ConstArrayView<IRInst*>& run, Index runIndex)
{
    SLANG_UNUSED(blockIndex);
    return _maybeAddEndAfterInst(run[runIndex]);
}

void LivenessContext::_maybeAddEndAfterInst(IRInst* inst)
{
    // We can't add after the inst, if it's a terminator
    // or if we find an end. 
    if (as<IRTerminatorInst>(inst) == nullptr && 
        !_findRootEnd(inst->getNextInst()))
    {
        // Just add end of scope after the inst 
        m_builder.setInsertLoc(IRInsertLoc::after(inst));
        // Add the live end inst
        m_builder.emitLiveRangeEnd(m_root);
    }
}

void LivenessContext::_maybeAddEndBeforeInst(IRInst* inst)
{
    if (!_findRootEnd(inst))
    {
        // Just add end of scope after the inst 
        m_builder.setInsertLoc(IRInsertLoc::before(inst));
        // Add the live end inst
        m_builder.emitLiveRangeEnd(m_root);
    }
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
        _maybeAddEndAfterRunIndex(blockIndex, run, lastAccessIndex);
        
        // Add the result
        return _addBlockResult(blockIndex, BlockResult::Found);
    }

    // We didn't find anything, so mark as not found
    return _addBlockResult(blockIndex, BlockResult::NotFound);
}

LivenessContext::BlockResult LivenessContext::_processBlock(BlockIndex blockIndex, const ConstArrayView<IRInst*>& run)
{
    // Note that the run must be some part of the run for the block indicated by blockIndex. One of
    // 
    // * If root start block - before the start (if accessed via successor)
    // * If root start block - after the start (if accessed initially in search)
    // * Otherwise the whole run for the block
    //
    // Since this is the case, we know start is not part of the run
    SLANG_ASSERT(run.indexOf(m_rootLiveStart) < 0);

    // If there is *another* start to the same root, we can't traverse to other blocks, and the last access 
    // in this block must be the result
    {
        // NOTE! We shouldn't/can't use run.indexOf here, because we are looking for *any* start to the root 
        // _indexOfRootStart does this search.
        // Moreover we know (it's a condition on run passed into this function) run cannot contain the root start.
        const Index startIndex = _indexOfRootStart(run);
        if (startIndex >= 0)
        {
            // Complete the block with this run
            return _completeBlock(blockIndex, run.head(startIndex));
        }
    }

    // Zero initialize all the counts
    Index foundCounts[Index(BlockResult::CountOf)] = { 0 };

    // Find all the successors for this block
    auto successors = _getSuccessors(blockIndex);

    const Index successorCount = successors.getCount();

    // Set up space to store successor results
    RAIIStackArray<BlockResult> successorResults(&m_successorResults);
    successorResults.setCount(successorCount);

    for (Index i = 0; i < successorCount; ++i)
    {
        const auto successorBlockIndex = successors[i];

        // NOTE! Care is needed around successorResults, because _processorSuccessor may cause the underlying list 
        // to be reallocated. 
        // If we always access through successorResults (ie RAIIStackArray type), things will be fine though.
            
        // Process the successor
        const auto successorResult = _processSuccessor(successorBlockIndex);

        successorResults[i] = successorResult;

        // Change counts depending on the result
        foundCounts[Index(successorResult)]++;
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

        auto successorResultsView = successorResults.getConstView();

        for (Index i = 0; i < successorCount; ++i)
        {
            const auto successorResult = successorResultsView[i];

            if (successorResult == BlockResult::NotFound)
            {
                const auto successorBlockIndex = successors[i];
                _maybeAddEndAtBlockStart(successorBlockIndex);
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

    auto blockInfo = _getBlockInfo(blockIndex);

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
        auto blockInfo = _getBlockInfo(rootStartBlockIndex);
        auto run = _getRun(blockInfo);

        // The run *must* contain this specific start start
        const auto startIndex = run.indexOf(m_rootLiveStart);
        SLANG_ASSERT(startIndex >= 0);

        // Make run scanning start *after* the start
        run = run.tail(startIndex + 1);

        // Mark the root as visited to stop an infinite loop
        _addBlockResult(rootStartBlockIndex, BlockResult::Visited);

        // Recursively find results
        auto foundResult = _processBlock(rootStartBlockIndex, run);

        if (foundResult == BlockResult::NotFound)
        {
            // Means there is no access to this variable(!)
            // Which means we can end the scope, after the the start scope
            _maybeAddEndAfterInst(m_rootLiveStart);
        }
    }

    // Set back to nullptr for safety
    m_rootLiveStart = nullptr;
    m_rootLiveStartBlock = nullptr;
}

bool LivenessContext::_isNormalRunInst(IRInst* inst)
{
    const auto op = inst->getOp();

    // Detect if it's a range start for the root.
    if (op == kIROp_LiveRangeStart)
    {
        auto start = as<IRLiveRangeStart>(inst);
        return start->getReferenced() == m_root;
    }

    // NOTE!
    // The ops in th elist above are the only ops *currently* that indicate an access.
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

bool LivenessContext::_isAccessTerminator(IRTerminatorInst* terminator)
{
    // This is to special case when a return, returns a root or an alias
    // 
    // We need to detect if the return value accesses the root

    if (terminator->getOp() == kIROp_ReturnVal)
    {
        // We are going to special case return if it hits an alias
        auto returnVal = static_cast<IRReturnVal*>(terminator);

        auto val = returnVal->getVal();
        
        // TODO(JS): This is perhaps somewhat argable, but it means if 
        // we have a cast between uint/int (for example) that isn't a problem

        // Strip construct
        if (val->getOp() == kIROp_Construct && val->getOperandCount() == 1)
        {
            val = val->getOperand(0);
        }

        // If it *is* the root it's an access
        if (val == m_root)
        {
            return true;
        }

        // If it's a load, see what is being loaded from an alias to the root
        if (auto load = as<IRLoad>(val))
        {
            const auto valPtr = load->getPtr();
            return m_aliases.contains(valPtr);
        }
    }

    return false;
}

bool LivenessContext::_isAnyRunInst(IRInst* inst)
{
    if (auto terminator = as<IRTerminatorInst>(inst))
    {
        return _isAccessTerminator(terminator);
    }
    return _isNormalRunInst(inst);
}

void LivenessContext::_findInstRunsForBlocks()
{
    const Count count = m_blockInfos.getCount();
    for (Index i = 0; i < count; ++i)
    {
        const auto blockIndex = BlockIndex(i);

        // Get the block
        auto block = _getBlock(blockIndex);

        // Get the block info
        auto* blockInfo = _getBlockInfo(blockIndex);

        const auto start = m_instRuns.getCount();
        blockInfo->runStart = start;

        if (blockInfo->instCount == 0)
        {
            // Nothing to do if it's empty
            SLANG_ASSERT(blockInfo->runCount == 0);
        }
        else if (blockInfo->instCount == 1)
        {
            // This is the easy case, since we don't need to determine the order of the instructions   
            SLANG_ASSERT(blockInfo->lastInst);
            m_instRuns.add(blockInfo->lastInst);
            blockInfo->runCount = 1;
        }
        else
        {
            // TODO(JS):
            // NOTE That we don't need to keep all accesses in the run, only the last accesses 
            // prior to a start or end of the block.
            //
            // For now we just add them all.

            blockInfo->runCount = blockInfo->instCount;

            m_instRuns.setCount(start + blockInfo->instCount);
            IRInst** dst = m_instRuns.getBuffer() + start;

            // Find all of the instructions of interest in order
            for (auto inst : block->getChildren())
            {
                if (_isNormalRunInst(inst))
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

        SLANG_ASSERT(blockInfo->runCount == blockInfo->instCount);

        // Special case the terminator - we allow a return that accesses the root 
        // to be added to the run. 
        //
        // TODO(JS): We might want this behavior to be switchable with an option.
        // If we don't add the terminator, everything else will behave correctly with regard
        // adding live range end markers.
        {
            auto terminator = block->getTerminator();
            if (_isAccessTerminator(terminator))
            {
                m_instRuns.add(terminator);
                blockInfo->runCount++;
            }
        }

        SLANG_ASSERT(blockInfo->runStart + blockInfo->runCount == m_instRuns.getCount());

        // The run count must be at least as many as the found instCount
        // There can be more instructions as we allow some special cases (for example around return)
        SLANG_ASSERT(blockInfo->runCount >= blockInfo->instCount);
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

    // Set the root
    m_root = root;
    m_rootBlock = as<IRBlock>(m_root->parent);

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
    _findInstRunsForBlocks();

    // Now we want to find all of the ends for each start
    for (auto liveStart : m_liveRangeStarts)
    {
        // We want to process this RangeStart for the root, to find all of the ends
        _findAndEmitRangeEnd(liveStart);
    }

    // No root is active in processing
    m_root = nullptr;
    m_rootBlock = nullptr;
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

    // We are going to precalculate a variety of things for blocks. 
    // Most processing is performed via BlockIndex, so we need to set up a map from the block pointer to the index
    // By having as an index we can easily/quickly associate information with blocks with arrays

    // Set up the map from blocks to indices
    m_blockIndexMap.Clear();

    m_blockInfos.clear();
    m_functionBlockInfos.clear();
    m_blockSuccessors.clear();

    {
        // First we find all the blocks in the function, we add to the map
        // and initialize the functionBlockInfos, which hold information about blocks that is constant across a function
        // We will associate successors too, but we can only do this once we have set up the map
        Index index = 0;
        for (auto block : func->getChildren())
        {
            IRBlock* blockInst = as<IRBlock>(block);
            m_blockIndexMap.Add(blockInst, BlockIndex(index++));

            FunctionBlockInfo functionBlockInfo;
            functionBlockInfo.init(blockInst);

            m_functionBlockInfos.add(functionBlockInfo);
        }

        // Allocate space for the root block infos
        m_blockInfos.setCount(index);

        // Now we have the map, work out the successors as BlockIndex for each block
        // and add those to m_blockSuccessors. They are indexed via successorsIndex/Count in the FunctionBlockInfos
        for (auto& info : m_functionBlockInfos)
        {
            auto block = info.block;

            // Add all the successors 
            auto successors = block->getSuccessors();
           
            const Index successorsStart = m_blockSuccessors.getCount();
            const Count successorsCount = successors.getCount();

            info.successorsStart = successorsStart;
            info.successorsCount = successorsCount;

            m_blockSuccessors.setCount(successorsStart + successorsCount);

            BlockIndex* dst = m_blockSuccessors.getBuffer() + successorsStart;

            for (auto successor : successors)
            {
                *dst++ = m_blockIndexMap[successor];
            }
        }
    }

    // Find the run of locations that all access the same root
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
    // Make a copy of all the locations
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
    