#include "slang-ir-liveness.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

#include "slang-ir-dominators.h"

namespace Slang
{

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

struct LivenessContext
{
    enum class FoundResult
    {
        Found,
        NotFound,
    };

    struct RootInfo
    {
        IRInst* root;
        IRLiveStart* liveStart;
    };

    void _addAccess(IRInst* inst)
    {
        if (as<IRLiveBase>(inst) ||
            as<IRAttr>(inst) ||
            as<IRDecoration>(inst))
        {
            return;
        }

        m_accessSet.Add(inst);
    }

    void _addLiveEndAtBlockStart(IRBlock* block, IRInst* root)
    {
        // Insert before the first ordinary inst
        auto inst = block->getFirstOrdinaryInst();
        SLANG_ASSERT(inst);

        m_builder.setInsertLoc(IRInsertLoc::before(inst));

        // Add the live end inst
        m_builder.emitLiveEnd(root);
    }
    IRInst* _findLastAccessInBlock(IRBlock* block)
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

    FoundResult _addResult(IRBlock* block, FoundResult result)
    {
        m_blockResult.AddIfNotExists(block, result);
        return result;
    }

    FoundResult processSuccessor(IRBlock* block)
    {
        auto res = m_blockResult.TryGetValue(block);
        if (res)
        {
            return *res;
        }

        SLANG_ASSERT(m_dominatorTree->properlyDominates(m_rootBlock, block));
        return processBlock(block);
    }

    FoundResult processBlock(IRBlock* block)
    {
        List<IRBlock*> dominatedSuccessors;
        {
            // Find result for all successors
            for (auto succ : block->getSuccessors())
            {
                if (m_dominatorTree->properlyDominates(m_rootBlock, succ))
                {
                    dominatedSuccessors.add(succ);
                }
            }
        }

        const Index count = dominatedSuccessors.getCount();
        List<FoundResult> successorResults;
        successorResults.setCount(count);

        Index foundCount = 0;

        for (Index i = 0; i < count; ++i)
        {
            auto succ = dominatedSuccessors[i];

            auto successorResult = processSuccessor(succ);
            successorResults.add(successorResult);
            foundCount += Index(successorResult == FoundResult::Found);
        }

        if (count > 0)
        {
            // If all successors have result, we are done
            if (foundCount == count)
            {
                return _addResult(block, FoundResult::Found);
            }

            // We want to place an end scope in all blocks where it wasn't found
            for (Index i = 0; i < count; ++i)
            {
                auto successor = dominatedSuccessors[i];
                const auto successorResult = successorResults[i];

                if (successorResult == FoundResult::NotFound)
                {
                    _addLiveEndAtBlockStart(successor, m_root);
                    _addResult(successor, FoundResult::Found);
                }
            }

            return _addResult(block, FoundResult::Found);
        }

        // Search the instructions in this block in reverse order, to find first access
        IRInst* lastAccess = _findLastAccessInBlock(block);

        // Wasn't an access so we are done
        if (lastAccess == nullptr)
        {
            return _addResult(block, FoundResult::NotFound);
        }

        // We need to specially handle last accesses that are terminators
        // * Return       - do we mark the scope end? It can't be after the return. Doing before seems wrong
        //                - So perhaps we don't end scope if it's a return?
        // * Conditional  - We want to place the access at the start of the targets
        // * Discard      - You could argue the ending scope before a discard is reasonable

        if (as<IRTerminatorInst>(lastAccess))
        {
            switch (lastAccess->getOp())
            {
                case kIROp_ReturnVal:
                case kIROp_ReturnVoid:
                {
                    // It's arguable what to do here. We can't add a scope end afterwards, there is no afterwards
                    // We don't want to add before, because that isn't correct. 
                    // For now we ignore
                    return _addResult(block, FoundResult::Found);
                }
                case kIROp_discard:
                {
                    // Can end scope before the discard
                    m_builder.setInsertLoc(IRInsertLoc::before(lastAccess));

                    // Add the live end inst
                    m_builder.emitLiveEnd(m_root);
                    return _addResult(block, FoundResult::Found);
                      
                }
                default: break;
            }

            // For others terminators, we just add to all the successors
            for (auto successor : dominatedSuccessors)
            {
                _addLiveEndAtBlockStart(successor, m_root);
            }

            return _addResult(block, FoundResult::Found);
        }

        // Just add end of scope after the inst 
        m_builder.setInsertLoc(IRInsertLoc::after(lastAccess));

        // Add the live end inst
        m_builder.emitLiveEnd(m_root);
        return _addResult(block, FoundResult::Found);
    }

    void processRoot(const RootInfo& rootInfo)
    {
        m_accessSet.Clear();
        m_aliases.clear();

        m_blockResult.Clear();

        auto root = rootInfo.root;

        m_aliases.add(root);

        // If we had a way to determine the index (or relative ordering) of two instructions in a block, 
        // then we wouldn't need to store 'access' information, just the the last one found for a block.
        // 
        // A way to do this is to just start from one, and see if the other is hit, but the more accesses 
        // from within the block there is the more inefficient this becomes
        // 
        // A more mundane way would be to work out the set of all blocks that have an access. 
        // We could then iterate in order over the instructions in order. The last one that is a an access must be the last
        // the problem is that it's not trivial to work out which instructions are an access, without traversing the *args*
        //
        // So perhaps the best you can do is
        // 1) Find the set of blocks that have accesses.
        // 2) Make a set of all the accessed instructions.
        // 
        // When we traverse the blocks from the root, we can determine quickly
        // 1) If a block contains any accesses (> 0 instructions that access it
        // 2) If it does, we go through the instructions to find the last access
        //
        // A small improvement would be to *count* the amount of instructions that access for a block, which makes it easier to work out termination
        //
        // If we use the dominator tree to map blocks to indices, we can do this relatively easily.

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

                // 
                bool isAlias = false;

                // We want to find instructions that access the root
                switch (cur->getOp())
                {
                    case kIROp_getElement:
                    {
                        base = static_cast<IRGetElement*>(cur)->getBase();
                        break;
                    }
                    case kIROp_getElementPtr:
                    {
                        base = static_cast<IRGetElementPtr*>(cur)->getBase();
                        isAlias = true;
                        break;
                    }
                    case kIROp_FieldAddress:
                    {
                        base = static_cast<IRFieldAddress*>(cur)->getBase();
                        isAlias = true;
                        break;
                    }
                    case kIROp_FieldExtract:
                    {
                        base = static_cast<IRFieldExtract*>(cur)->getBase();
                        break;
                    }
                    case kIROp_Load:
                    {
                        base = static_cast<IRLoad*>(cur)->getPtr();
                        break;
                    }
                    case kIROp_Store:
                    {
                        base = static_cast<IRStore*>(cur)->getPtr();
                        break;
                    }
                    case kIROp_getAddr:
                    {
                        IRGetAddress* getAddr = static_cast<IRGetAddress*>(cur);
                        base = getAddr->getOperand(0);
                        isAlias = true;
                        break;
                    }
                    default: break;
                }

                // If the instruction is making an alias like reference,
                // we add to the list of things that indirectly *reference* items
                if (base == alias && isAlias)
                {
                    // Add this instruction to the aliases to the root.
                    m_aliases.add(cur);
                }

                _addAccess(cur);
            }
        }

        {
            m_rootBlock = as<IRBlock>(root->parent);
            m_root = root;

            auto foundResult = processBlock(m_rootBlock);

            if (foundResult == FoundResult::NotFound)
            {
                // Means there is no access to this variable(!)
                // Which means we can end the scope, after the the start scope
                m_builder.setInsertLoc(IRInsertLoc::after(rootInfo.liveStart));
                m_builder.emitLiveEnd(root);
            }
        }
    }

    void processFunction(IRFunc* funcInst)
    {
        List<RootInfo> rootInfos;

        // Iterate through blocks 
        for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
        {
            for (auto inst = block->getFirstChild(); inst; inst = inst->getNextInst())
            {
                // Instructions appear as 'let' if they have some used result.
                // 
                // Instructions in general are their result (so producing a value), and therefore a variable.

                // We look for var declarations.
                if (auto varInst = as<IRVar>(inst))
                {
                    // TODO(JS): 
                    // For now we'll just add start liveness information, just to check out this path
                    // all works
                    m_builder.setInsertLoc(IRInsertLoc::after(varInst));

                    // Emit the start
                    auto liveStart = m_builder.emitLiveStart(varInst);

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

    void processModule()
    {
        // We want to find all of the functions
        // Then we want to look through their definition
        // inserting instructions that mark the liveness start/end

        // When we process liveness, is prior to output for a target
        // So post specialization

        IRModuleInst* moduleInst = m_module->getModuleInst();
       
        for (IRInst* child = moduleInst->getFirstDecorationOrChild(); child; child = child->getNextInst())
        {
            if (auto funcInst = as<IRFunc>(child))
            {
                processFunction(funcInst);
            }
        }
    }

    LivenessContext(IRModule* module):
        m_module(module)
    {
        m_sharedBuilder.init(module);
        m_builder.init(m_sharedBuilder);
    }

    // The dominator tree for the current function
    RefPtr<IRDominatorTree> m_dominatorTree;
    IRBlock* m_rootBlock;
    IRInst* m_root;

    // Holds a set of all the functions that in some way access the root.
    HashSet<IRInst*> m_accessSet;

    // A list of instructions that alias to a root
    List<IRInst*> m_aliases;

    Dictionary<IRBlock*, FoundResult> m_blockResult;

    IRModule* m_module;
    SharedIRBuilder m_sharedBuilder;
    IRBuilder m_builder;
};

} // anonymous

void addLivenessTrackingToModule(IRModule* module)
{
    LivenessContext context(module);

    context.processModule();
}

} // namespace Slang
