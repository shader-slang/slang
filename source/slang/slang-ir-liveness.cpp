#include "slang-ir-liveness.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

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
                            m_builder.emitLiveStart(varInst);
                        }
                    }
                }
            }
        }
    }

    LivenessContext(IRModule* module):
        m_module(module)
    {
        m_sharedBuilder.init(module);
        m_builder.init(m_sharedBuilder);
    }

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
