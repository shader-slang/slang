#include "slang-ir-liveness.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

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
