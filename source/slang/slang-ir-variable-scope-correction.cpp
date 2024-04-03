#include "slang-ir-insts.h"
#include "slang-ir.h"

#include "slang-ir-dominators.h"
#include "slang-ir-variable-scope-correction.h"

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

    // traverse each instruction in the function
    for (auto block = funcInst->getFirstBlock(); block; block = block->getNextBlock())
    {
        for (auto inst = block->getFirstChild(); inst; inst = inst->getNextInst())
        {
            uint32_t indent = 1;
            printf("process inst = %d\n", inst->getOp());
            // traverse the dominator tree, theoretically, the variables in each of dominator should be accessible in the current block,
            // except the loop block, so we have to find out if there is a loop block in the dominator chain.
            auto dominatorBlock = dominatorTree->getImmediateDominator(block);
            for (; dominatorBlock; dominatorBlock = dominatorTree->getImmediateDominator(dominatorBlock))
            {
                debugPrint(indent++, dominatorBlock, "dominator block");
                if (auto loop = as<IRLoop>(dominatorBlock->getTerminator()))
                {
                    debugPrint(indent, loop, "loop block");
                    // Get the break block of the loop and check if such block
                    auto breakBlock = loop->getBreakBlock();
                    debugPrint(indent++, breakBlock, "break block");

                    // If a block is not dominated by the break block, but dominated by the loop header, then the
                    // instructions in this block are considered to be defined in the loop. We need to search
                    // the instruction's use sites to see if there is any uses are out of the loop.
                    if (!dominatorTree->dominates(breakBlock, block))
                    {
                        debugPrint(indent++, block, "block is not dominated by break block");
                        // traverse all uses of this instruction
                        for (auto use = inst->firstUse; use; use=use->nextUse)
                        {
                            debugPrint(indent++, use->getUser()->getParent(), "inst's use block is");
                            if (auto userBlock = as<IRBlock>(use->getUser()->getParent()))
                            {
                                // If the use site of this instruction is dominated by the break block, it means that the
                                // instruction is used after the break block, so we need to make that instruction available globally.
                                // By doing so, we record all the users of this instructions.
                                if (dominatorTree->dominates(breakBlock, userBlock))
                                {
                                    debugPrint(indent, inst, "inst is defined in loop but used outside of loop");
                                    debugPrint(indent++, use->getUser(), "The user is");

                                    auto list = workListMap.tryGetValue(inst);
                                    if (!list)
                                    {
                                        workListMap.add(inst, List<IRInst*>());
                                        list = workListMap.tryGetValue(inst);
                                    }
                                    list->add(use->getUser());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

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

