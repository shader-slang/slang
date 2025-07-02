#include "slang-ir-legalize-composite-select.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-legalize-varying-params.h"
#include "slang-ir-specialize-address-space.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
void legalizeASingleNonVectorCompositeSelect(
    IRBuilder& builder,
    IRSelect* selectInst)
{
    SLANG_ASSERT(selectInst);

    auto resultType = selectInst->getFullType();
    auto trueResult = selectInst->getTrueResult();
    auto falseResult = selectInst->getFalseResult();

    IRBlock* trueBlock;
    IRBlock* falseBlock;
    IRBlock* afterBlock;
    builder.emitIfElseWithBlocks(selectInst->getCondition(), trueBlock, falseBlock, afterBlock);

    // Generate if-select-true and else-select-false clause
    builder.setInsertInto(trueBlock);
    builder.emitBranch(afterBlock, 1, &trueResult);

    builder.setInsertInto(falseBlock);
    builder.emitBranch(afterBlock, 1, &falseResult);

    // Move everything after the OpSelect into the "after" block
    List<IRInst*> instsToMove;
    instsToMove.reserve(15);
    IRInst* nextInst = selectInst;
    while (nextInst)
    {
        instsToMove.add(nextInst);
        nextInst = nextInst->getNextInst();
    }
    for (auto i : instsToMove)
        afterBlock->insertAtEnd(i);
    
    // Merge result of branches into param
    builder.setInsertInto(afterBlock);
    auto param = builder.emitParam(resultType);
    selectInst->replaceUsesWith(param);

    // Clean up
    selectInst->removeAndDeallocate();
}
void legalizeNonVectorCompositeSelect(IRModule* module)
{
    IRBuilder builder(module);
    for (auto globalInst : module->getModuleInst()->getChildren())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;
        for (auto block : func->getBlocks())
        {
            auto inst = block->getFirstInst();
            IRInst* next;
            for (; inst; inst = next)
            {
                next = inst->getNextInst();
                switch (inst->getOp())
                {
                case kIROp_Select:
                    // Replace OpSelect with if/else branch (same process as glslang)
                    if (!isScalarOrVectorType(inst->getFullType()))
                        legalizeASingleNonVectorCompositeSelect(builder, as<IRSelect>(inst));
                    continue;
                }
            }
        }
    }
}
} // namespace Slang
