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

    FoundResult processBlock(IRBlock* parent, IRInst* rootInst)
    {
        SLANG_ASSERT(rootInst == nullptr || rootInst->parent == parent);

        // I guess I could use the 'Use' mechanism to do this.
        // I find all of the uses of the root. Then we find all the uses of that, and so on 
        // 
        // We'd probably end up storing for each block all of the use sites.
        // 
        
        





     

        IRInst* lastAccess = nullptr;

        {
            IRInst* cur = rootInst ? rootInst : parent->getFirstChild();

            for (; cur; cur = cur->getNextInst())
            {
                // We want to find instructions that access the root
                switch (cur->getOp())
                {
                    case kIROp_getElement:
                    {
                        IRInst* base = static_cast<IRGetElement*>(cur)->getBase();
                        if (m_referencesToRoot.Contains(base))
                        {
                            lastAccess = cur;
                        }
                        break;
                    }
                    case kIROp_getElementPtr:
                    {
                        IRInst* base = static_cast<IRGetElementPtr*>(cur)->getBase();
                        if (m_referencesToRoot.Contains(base))
                        {
                            m_referencesToRoot.Add(base);
                        }
                        break;
                    }
                    case kIROp_FieldAddress:
                    {
                        IRInst* base = static_cast<IRFieldAddress*>(cur)->getBase();
                        if (m_referencesToRoot.Contains(base))
                        {
                            m_referencesToRoot.Add(base);
                        }
                        break;
                    }
                    case kIROp_FieldExtract:
                    {
                        IRInst* base = static_cast<IRFieldExtract*>(cur)->getBase();

                        if (m_referencesToRoot.Contains(base))
                        {
                            lastAccess = cur;
                        }
                        break;
                    }
                    case kIROp_Load:
                    {
                        IRLoad* load = static_cast<IRLoad*>(cur);
                        if (m_referencesToRoot.Contains(load->getPtr()))
                        {
                            lastAccess = load;
                        }
                        break;
                    }
                    case kIROp_Store:
                    {
                        IRStore* store = static_cast<IRStore*>(cur);
                        if (m_referencesToRoot.Contains(store->getPtr()))
                        {
                            lastAccess = store;
                        }
                        break;
                    }
                    case kIROp_getAddr:
                    {
                        IRGetAddress* getAddr = static_cast<IRGetAddress*>(cur);
                        IRInst* target = getAddr->getOperand(0);

                        if (m_referencesToRoot.Contains(target))
                        {
                            m_referencesToRoot.Add(target);
                        }
                        break;
                    }
                    default:
                    {
                        if (as<IRLiveBase>(cur) || 
                            as<IRAttr>(cur) ||
                            as<IRDecoration>(cur))
                        {
                            // Don't need to do anything
                        }
                        else
                        {
                            // Go through the args. 
                            // If used 
                        }
                    }
                }
            }
        }

        // When we look at children there can be these scenarios
        // 1) None of the children access. 
        // 2) One or more of the children access
        // 
        // If none access, then we can look at the last access in this block. If we find one
        // we set the scope as after this

        auto childBlocks = m_dominatorTree->getImmediatelyDominatedBlocks(parent);

        const Index count = childBlocks.getCount();

        List<FoundResult> childResults;
        childResults.setCount(count);

        // We need to record what the result is for all of the child blocks
        
        auto cur = childBlocks.begin();
        const auto end = childBlocks.end();

        Index foundCount = 0;

        for (Index i = 0; cur != end; ++cur, ++i)
        {
            const FoundResult childResult = processBlock(*cur, nullptr);
            // Save the result
            childResults[i] = childResult;

            foundCount += Index(childResult == FoundResult::Found);
        }


            // If it's passed as a parameter
            //


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

        // If we go depth first, if a child doesn't identify 

        // Okay, lets go through each block again and look for accesses. 
        // Any access must be built on other accesses
        for (const auto& rootInfo : rootInfos)
        {
            m_referencesToRoot.Clear();

            auto root = rootInfo.root;

            // An access to the root itself qualifies
            m_referencesToRoot.Add(root);

            IRBlock* parent = as<IRBlock>(root->parent);

            auto foundResult = processBlock(parent, root);
            
            if (foundResult == FoundResult::NotFound)
            {
                // Means there is no access to this variable(!)
                // Which means we can end the scope, after the the start scope
                m_builder.setInsertLoc(IRInsertLoc::after(rootInfo.liveStart));
                m_builder.emitLiveEnd(root);
            }
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

    HashSet<IRInst*> m_referencesToRoot;
    RefPtr<IRDominatorTree> m_dominatorTree;

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
