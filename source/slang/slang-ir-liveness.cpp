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

namespace { // anonymous

struct LivenessContext
{
    // NOTE! Care must be taken changing the order. Some checks rely on Found having a smaller value than `NotFound`.
    // Allowing NotFound to be promoted to Found.
    enum class FoundResult
    {
        Found,              ///< All paths were either not dominated, found 
        NotFound,           ///< It is dominated but no access was found. 
        Visited,
        NotDominated,       ///< If it's not dominated it can't have a liveness end 

        CountOf,
    };

    enum class AccessType
    {
        None,               ///< There is no access
        Alias,              ///< Produces an alias to the root
        Access,             ///< Is an access to the root (perhaps through an alias)
    };

    struct RootInfo
    {
        IRInst* root;
        IRLiveRangeStart* liveStart;
    };

        /// Processor a successor to a block
    FoundResult processSuccessor(IRBlock* block);

        /// Process a block 
    FoundResult processBlock(IRBlock* block);

        /// Process a 'root'. A variable that has liveness tracking
    void processRoot(const RootInfo& rootInfo);

        /// Process the module
    void processModule();

    LivenessContext(IRModule* module):
        m_module(module)
    {
        m_sharedBuilder.init(module);
        m_builder.init(m_sharedBuilder);
    }

        /// Process a function in the module
    void _processFunction(IRFunc* funcInst);

        // Add a live end instruction at the start of block, referencing the root 'root'.
    void _addLiveRangeEndAtBlockStart(IRBlock* block, IRInst* root);

        /// Find the last instruction in block that accesses one of the roots or it's aliases
        /// Requires that m_accessSet contains all of the accesses
        /// Returns nullptr if no access instruction was found in the block.
    IRInst* _findLastAccessInBlock(IRBlock* block);

        /// Add a result for the block
        /// Allows for promotion from NotFound -> Found if there is already a result
    FoundResult _addResult(IRBlock* block, FoundResult result);

    RefPtr<IRDominatorTree> m_dominatorTree;    ///< The dominator tree for the current function

    IRInst* m_root;                             ///< The root item we are searching for accesses to, to determine scope/liveness
    IRBlock* m_rootBlock;                       ///< The block the root is in
    
    HashSet<IRInst*> m_accessSet;               ///< Holds a set of all the functions that in some way access the root.

    List<IRInst*> m_aliases;                    ///< A list of instructions that alias to the root

    Dictionary<IRBlock*, FoundResult> m_blockResult;    ///< Cached result for each block

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

IRInst* LivenessContext::_findLastAccessInBlock(IRBlock* block)
{
    // Search the instructions in this block in reverse order, to find first access
    for (IRInst* cur = block->getLastChild(); cur; cur = cur->getPrevInst())
    {
        if (m_accessSet.Contains(cur))
        {
            return cur;
        }
    }
    return nullptr;
}

LivenessContext::FoundResult LivenessContext::_addResult(IRBlock* block, FoundResult result)
{
    auto currentResultPtr = m_blockResult.TryGetValueOrAdd(block, result);
    if (currentResultPtr)
    {
        const auto currentResult = *currentResultPtr;
        // If it were NotDominated, it cannot be promoted to Found/NotFound.
        SLANG_ASSERT(currentResult != FoundResult::NotDominated);

        // We can only promote from NotFound -> Found

        SLANG_ASSERT(Index(result) <= Index(currentResult));

        // Set the new result
        *currentResultPtr = result;
    }

    return result;
}

LivenessContext::FoundResult LivenessContext::processSuccessor(IRBlock* block)
{
    // Check if there is already a result for this block. 
    // If there is just return that.
    auto res = m_blockResult.TryGetValue(block);
    if (res)
    {
        return *res;
    }

    // If the block is *not* dominated by the root block, we know it can't 
    // end liveness. 
    // Return that it is not dominated, and add to the cache for the block
    if (!m_dominatorTree->properlyDominates(m_rootBlock, block))
    {
        return _addResult(block, FoundResult::NotDominated);
    }

    // Mark that it is visited
    m_blockResult.Add(block, FoundResult::Visited);

    // Else process the block to try and find the last used instruction
    return processBlock(block);
}

LivenessContext::FoundResult LivenessContext::processBlock(IRBlock* block)
{
    // Find all the successors for this block
    auto successors = block->getSuccessors();
    const Index count = successors.getCount();

    // We need to store of the results for successors
    List<FoundResult> successorResults;
    successorResults.setCount(count);

    // Zero initialize all the counts
    Index foundCounts[Index(FoundResult::CountOf)] = { 0 };

    {
        auto cur = successors.begin();
        for (Index i = 0; i < count; ++i, ++cur)
        {
            auto succ = *cur;

            // Process the successor
            const auto successorResult = processSuccessor(succ);

            // Store the result
            successorResults[i] = successorResult;

            // Change counts depending on the result
            foundCounts[Index(successorResult)]++;
        }
    }

    const Index foundCount = foundCounts[Index(FoundResult::Found)];
    const Index notFoundCount = foundCounts[Index(FoundResult::NotFound)];

    const Index otherCount = count - (foundCount + notFoundCount);

    // If one or more of the successors (or successors of successors),
    // was found to have the last access, we need to mark the end of scope
    // at the start of any other paths (which are dominated).
    if (foundCount > 0)
    {
        // If all successors have result, or are not dominated
        if (foundCount + otherCount == count)
        {
            return _addResult(block, FoundResult::Found);
        }

        // We want to place an end scope in all blocks where it wasn't found
        auto cur = successors.begin();
        for (Index i = 0; i < count; ++i, ++cur)
        {
            auto successor = *cur;
            const auto successorResult = successorResults[i];
            if (successorResult == FoundResult::NotFound)
            {
                _addLiveRangeEndAtBlockStart(successor, m_root);
                _addResult(successor, FoundResult::Found);
            }
        }

        // This block, can now be marked as found 
        return _addResult(block, FoundResult::Found);
    }

    // Search the instructions in this block in reverse order, to find last access
    IRInst* lastAccess = _findLastAccessInBlock(block);

    // Wasn't an access so we are done
    if (lastAccess == nullptr)
    {
        return _addResult(block, FoundResult::NotFound);
    }

    // Can never be a terminator, because logic to find the access instructions does not 
    // include terminators.
    SLANG_ASSERT(as<IRTerminatorInst>(lastAccess) == nullptr);

    // Just add end of scope after the inst 
    m_builder.setInsertLoc(IRInsertLoc::after(lastAccess));

    // Add the live end inst
    m_builder.emitLiveRangeEnd(m_root);
    return _addResult(block, FoundResult::Found);
}

void LivenessContext::processRoot(const RootInfo& rootInfo)
{
    // Clear the work structures
    m_accessSet.Clear();
    m_aliases.clear();
    m_blockResult.Clear();

    auto root = rootInfo.root;

    // Store root information, so don't have to pass around methods
    m_rootBlock = as<IRBlock>(root->parent);
    m_root = root;

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
                    m_accessSet.Add(cur);
                    break;
                }
                default: break;
                }
            }
        }
    }

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
        // Mark the root as visited to stop an infinite loop
        _addResult(m_rootBlock, FoundResult::Visited);

        // Recursively find results
        auto foundResult = processBlock(m_rootBlock);

        if (foundResult == FoundResult::NotFound)
        {
            // Means there is no access to this variable(!)
            // Which means we can end the scope, after the the start scope
            m_builder.setInsertLoc(IRInsertLoc::after(rootInfo.liveStart));
            m_builder.emitLiveRangeEnd(root);
        }
    }
}

void LivenessContext::_processFunction(IRFunc* funcInst)
{
    // If it has no body, then we are done
    if (funcInst->getFirstBlock() == nullptr)
    {
        return;
    }

    List<RootInfo> rootInfos;

    // Iterate through blocks in the function, looking for variables to live track
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        for (auto inst = block->getFirstChild(); inst; inst = inst->getNextInst())
        {
            // We look for var declarations.
            if (auto varInst = as<IRVar>(inst))
            {
                // Add the start location
                m_builder.setInsertLoc(IRInsertLoc::after(varInst));

                // Emit the start
                auto liveStart = m_builder.emitLiveRangeStart(varInst);

                // Add as a root
                RootInfo rootInfo;
                rootInfo.root = varInst;
                rootInfo.liveStart = liveStart;

                rootInfos.add(rootInfo);
            }
        }
    }

    // Create the dominator tree.
    m_dominatorTree = computeDominatorTree(funcInst);

    // Process the roots
    for (const auto& rootInfo : rootInfos)
    {
        processRoot(rootInfo);
    }
}

void LivenessContext::processModule()
{
    // When we process liveness, is prior to output for a target
    // So post specialization

    IRModuleInst* moduleInst = m_module->getModuleInst();

    for (IRInst* child : moduleInst->getChildren())
    {
        // We want to find all of the functions, and process them
        if (auto funcInst = as<IRFunc>(child))
        {
            // Then we want to look through their definition
            // inserting instructions that mark the liveness start/end
            _processFunction(funcInst);
        }
    }
}

} // anonymous

void addLivenessTrackingToModule(IRModule* module)
{
    LivenessContext context(module);

    context.processModule();
}

} // namespace Slang
    