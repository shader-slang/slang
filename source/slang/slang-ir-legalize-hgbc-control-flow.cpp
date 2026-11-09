#include "slang-ir-legalize-hgbc-control-flow.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
void legalizeControlFlowForHGBC(IRModule* module)
{
    List<IRInst*> terminators;
    InstPassBase context(module);
    context.processAllInsts(
        [&](IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_Loop:
            case kIROp_IfElse:
            case kIROp_Switch:
                terminators.add(inst);
                break;
            }
        });

    for (auto terminator : terminators)
    {
        IRBuilder builder(terminator);
        IRBuilderSourceLocRAII sourceLoc(&builder, terminator->sourceLoc);
        builder.setInsertBefore(terminator);

        // Consider `for (;;) { if (ready()) return result; }`. Its structured loop needs
        // a break block even though execution never reaches that block. HGBC uses ordinary
        // branches, so discard these structural references and let the following DCE remove
        // unreachable blocks. Real break, continue, and back edges remain unchanged.
        if (auto loop = as<IRLoop>(terminator))
        {
            // Phi elimination has already moved branch arguments into stores.
            SLANG_RELEASE_ASSERT(loop->getArgCount() == 0);
            auto branch = builder.emitBranch(loop->getTargetBlock());
            copyNameHintAndDebugDecorations(branch, loop);
            loop->removeAndDeallocate();
            continue;
        }
        if (auto ifElse = as<IRIfElse>(terminator))
        {
            auto branch = builder.emitBranch(
                ifElse->getCondition(),
                ifElse->getTrueBlock(),
                ifElse->getFalseBlock());
            copyNameHintAndDebugDecorations(branch, ifElse);
            ifElse->removeAndDeallocate();
            continue;
        }

        auto inst = cast<IRSwitch>(terminator);
        auto testBlock = cast<IRBlock>(inst->getParent());

        // Keep case bodies and their outgoing edges intact, including source-level fallthrough.
        // Only the selector dispatch is expanded. The bytecode uses unstructured branches, so
        // these new test blocks do not need synthetic structured merge regions.
        for (UInt index = 0; index < inst->getCaseCount(); ++index)
        {
            auto nextBlock = inst->getDefaultLabel();
            if (index + 1 < inst->getCaseCount())
            {
                nextBlock = builder.createBlock();
                nextBlock->insertAfter(testBlock);
            }
            auto condition = builder.emitEql(inst->getCondition(), inst->getCaseValue(index));
            builder.emitBranch(condition, inst->getCaseLabel(index), nextBlock);
            testBlock = nextBlock;
            builder.setInsertInto(nextBlock);
        }
        if (inst->getCaseCount() == 0)
            builder.emitBranch(inst->getDefaultLabel());
        inst->removeAndDeallocate();
    }
}
} // namespace Slang
