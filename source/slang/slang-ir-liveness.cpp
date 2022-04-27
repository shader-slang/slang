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

        IRInst* nextChild;
        for (IRInst* child = moduleInst->getFirstDecorationOrChild(); child; child = child->getNextInst())
        {
            if (auto funcInst = as<IRFunc>(child))
            {
                // Get the first block, if it doesn't have one we are doen
                if (auto firstBlock = funcInst->getFirstBlock())
                {
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

void addLivenessTracking(IRModule* module)
{
    LivenessContext context(module);

    context.processModule();
}

} // namespace Slang
