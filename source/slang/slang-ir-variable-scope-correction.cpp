#include "slang-ir-insts.h"
#include "slang-ir.h"

#include "slang-ir-dominators.h"
#include "slang-ir-variable-scope-correction.h"
#include "slang-ir-util.h"

namespace Slang
{

namespace { // anonymous
struct VariableScopeCorrectionContext
{
    VariableScopeCorrectionContext(IRModule* module):
        m_module(module), m_builder(module)
    {
    }

    void processModule();

    /// Process a function in the module
    void _processFunction(IRFunc* funcInst);

    IRModule* m_module;
    IRBuilder m_builder;
};

void VariableScopeCorrectionContext::processModule()
{
    IRModuleInst* moduleInst = m_module->getModuleInst();
    for (IRInst* child : moduleInst->getChildren())
    {
        // We want to find all of the functions, and process them
        if (auto funcInst = as<IRFunc>(child))
        {
            _processFunction(funcInst);
        }
    }
}

static void debugPrint(uint32_t indent, IRInst* inst, const char* message)
{
    uint32_t debugid = 0;
    for (uint32_t i = 0; i < indent; i++)
    {
        printf("    ");
    }
#ifdef SLANG_ENABLE_IR_BREAK_ALLOC
        debugid = inst->_debugUID;
#endif
    printf("%s, id: %d\n", message, debugid);
}

void VariableScopeCorrectionContext::_processFunction(IRFunc* funcInst)
{
    IRDominatorTree* dominatorTree = m_module->findOrCreateDominatorTree(funcInst);
    Dictionary<IRInst*, List<IRInst*>> workListMap;
    Dictionary<IRBlock*, IRLoop> loopHeaderMap;

    // traverse all blocks in the function
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        uint32_t indent = 0;
        debugPrint(indent++, block, "processing block");

        // Traverse all the dominators of a given block to check whether this given block is in a loop region.
        // Loop region blocks are the blocks that are dominated by the loop header block
        // but not dominated by the loop break block.
        auto dominatorBlock = dominatorTree->getImmediateDominator(block);
        for (; dominatorBlock; dominatorBlock = dominatorTree->getImmediateDominator(dominatorBlock))
        {
            debugPrint(indent++, dominatorBlock, "dominator block");

            // Find if the block is loop header block
            if (auto loopHeader = as<IRLoop>(dominatorBlock->getTerminator()))
            {
                debugPrint(indent, loopHeader, "loop header");
                // Get the break block of the loop and check if such block
                auto breakBlock = loopHeader->getBreakBlock();
                debugPrint(indent++, breakBlock, "break block");

                // Check if the current block is dominated by the break block. If so, it means that the block is in the loop region.
                if (!dominatorTree->dominates(breakBlock, block))
                {
                    debugPrint(indent++, block, "block is not dominated by break block");
                    loopHeaderMap.add(block, *loopHeader);
                }
            }
        }
    }

    printf("\n");
    // Traverse all the instructions in function.
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        if(auto loopHeader = loopHeaderMap.tryGetValue(block))
        {
            for (auto inst = block->getFirstChild(); inst; inst = inst->getNextInst())
            {
                List<IRInst*> instList;
                uint32_t indent = 0;
                auto breakBlock = loopHeader->getBreakBlock();
                // traverse all uses of this instruction
                for (auto use = inst->firstUse; use; use=use->nextUse)
                {
                    debugPrint(indent++, getBlock(use->getUser()), "inst's use block is");
                    if (auto userBlock = getBlock(use->getUser()))
                    {
                        // If the use site of this instruction is dominated by the break block, it means that the
                        // instruction is used after the break block, so we need to make that instruction available globally.
                        // By doing so, we record all the users of this instructions.
                        if (dominatorTree->dominates(breakBlock, userBlock))
                        {
                            debugPrint(indent, inst, "inst is defined in loop but used outside of loop");
                            debugPrint(indent++, use->getUser(), "The user is");
                            instList.add(use->getUser());
                        }
                    }
                }
                if (instList.getCount() > 0)
                {
                    workListMap.add(inst, instList);
                }
            }
        }
    }

    printf("\n");
    // By duplicating the instructions right before their users, we can make the instructions available globally.
    for(auto it = workListMap.begin(); it != workListMap.end(); it++)
    {
        auto inst = it->first;
        auto list = it->second;
        debugPrint(0, inst, "inst");
        for(auto user : list)
        {
            debugPrint(1, user, "The user");
            inst->insertAt(IRInsertLoc::before(user));
        }
    }
}

} // anonymous

void applyVariableScopeCorrection(IRModule* module)
{
    VariableScopeCorrectionContext context(module);

    context.processModule();
}

} // namespace Slang

