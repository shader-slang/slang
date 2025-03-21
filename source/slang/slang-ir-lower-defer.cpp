// slang-ir-lower-defer.cpp

#include "slang-ir-lower-defer.h"

#include "slang-ir-clone.h"
#include "slang-ir-dominators.h"
#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{

struct DeferLoweringContext : InstPassBase
{
    DiagnosticSink* diagnosticSink;

    DeferLoweringContext(IRModule* inModule)
        : InstPassBase(inModule)
    {
    }

    void processModule()
    {
        List<IRDefer*> unhandledDefers;
        processInstsOfType<IRDefer>(
            kIROp_Defer,
            [&](IRDefer* defer)
            {
                unhandledDefers.add(defer);
            });

        // Iterating over `defer` instructions in reverse order allows us to
        // expand them in the correct order, including nested `defer`s.
        unhandledDefers.reverse();

        IRBuilder builder(module);
        IRCloneEnv env;

        while (unhandledDefers.getCount() != 0)
        {
            IRDefer* defer = unhandledDefers.getLast();
            unhandledDefers.removeLast();

            IRFunc* func = getParentFunc(defer);
            auto dom = module->findOrCreateDominatorTree(func);

            auto parentBlock = as<IRBlock>(defer->getParent());
            auto dominatedBlocks = dom->getProperlyDominatedBlocks(parentBlock);

            // Construct a set from the dominated blocks so that we can quickly
            // check if the terminator of a block jumps to a non-dominated
            // location.
            HashSet<IRBlock*> dominatedBlocksSet;
            dominatedBlocksSet.add(parentBlock);
            for (IRBlock* block : dominatedBlocks)
                dominatedBlocksSet.add(block);

            for (IRBlock* block : dominatedBlocksSet)
            {
                auto terminator = block->getTerminator();
                bool exits = false;
                if (!terminator)
                    exits = true;
                else
                {
                    switch(terminator->getOp())
                    {
                    case kIROp_Return:
                    case kIROp_discard:
                    case kIROp_Throw:
                        exits = true;
                        break;
                    case kIROp_unconditionalBranch:
                    case kIROp_conditionalBranch:
                        // Check through all Block operands and check that all
                        // are dominated.
                        for (UInt index = 0; index < terminator->getOperandCount(); ++index)
                        {
                            auto target = as<IRBlock>(terminator->getOperand(index));
                            if (target && !dominatedBlocksSet.contains(target))
                            {
                                exits = true;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }

                if (exits)
                { // Duplicate child instructions to the end of this block.
                    builder.setInsertBefore(terminator);
                    auto childBlock = as<IRBlock>(defer->getFirstChild());
                    for(IRInst* inst: childBlock->getChildren())
                        cloneInst(&env, &builder, inst);
                }
            }

            defer->removeAndDeallocate();
        }
    }
};

void lowerDefer(IRModule* module, DiagnosticSink* sink)
{
    DeferLoweringContext context(module);
    context.diagnosticSink = sink;
    return context.processModule();
}

} // namespace Slang
