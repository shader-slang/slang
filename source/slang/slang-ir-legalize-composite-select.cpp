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
    TargetRequest* target,
    IRBuilder& builder,
    IRSelect* selectInst,
    DiagnosticSink* sink)
{
    SLANG_UNUSED(sink)
    SLANG_UNUSED(target)
    SLANG_ASSERT(selectInst);

    // result var
    builder.setInsertBefore(selectInst);
    auto resultVar = builder.emitVar(selectInst->getFullType());

    // emit {if(condition) result = selectTrueBlock; else result = selectFalseBlock;}
    auto trueBlock = builder.createBlock();
    auto falseBlock = builder.createBlock();
    auto afterBlock = builder.createBlock();
    builder.emitIfElseWithBlocks(selectInst->getCondition(), trueBlock, falseBlock, afterBlock);

    builder.setInsertInto(trueBlock);
    builder.emitStore(
        builder.emitGetAddress(resultVar->getFullType(), resultVar),
        selectInst->getTrueResult());

    builder.setInsertInto(falseBlock);
    builder.emitStore(
        builder.emitGetAddress(resultVar->getFullType(), resultVar),
        selectInst->getFalseResult());

    // Move everything after the if/else into the "after" block
    builder.setInsertInto(afterBlock);
    IRInst* nextInst = selectInst;
    while (nextInst)
    {
        afterBlock->insertAtEnd(nextInst);
        nextInst = nextInst->getNextInst();
    }
    
    // clean up
    selectInst->replaceUsesWith(resultVar);
    selectInst->removeAndDeallocate();
}
void legalizeNonVectorCompositeSelect(TargetRequest* target, IRModule* module, DiagnosticSink* sink)
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
                    // We will legalize the same way glslang legalizes, replace the OpSelect with an OpBranch and OpStore.
                    if (!isScalarOrVectorType(inst->getFullType()))
                        legalizeASingleNonVectorCompositeSelect(target, builder, as<IRSelect>(inst), sink);
                    continue;
                }
            }
        }
    }
}
} // namespace Slang
