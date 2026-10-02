#include "slang-ir-lower-array-builtin-cast.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct ArrayBuiltinCastLoweringContext
{
    IRModule* module;

    List<IRInst*> workList;

    void collectCasts(IRInst* inst)
    {
        if (isArrayBuiltinCast(inst))
            workList.add(inst);
        for (auto child : inst->getDecorationsAndChildren())
            collectCasts(child);
    }

    /// Replace the array cast `cast` with a conversion of each element, adding any nested array
    /// cast this creates to the work list. If the conversion is a loop, return the function or
    /// global variable whose blocks were split for it, which needs its blocks re-sorted;
    /// otherwise return null.
    IRGlobalValueWithCode* lowerCast(IRInst* cast)
    {
        auto toType = as<IRArrayType>(cast->getDataType());
        auto toElementType = toType->getElementType();
        auto value = cast->getOperand(0);

        // Specializing a call for a buffer-load argument leaves the caller's cast unused, and
        // expanding it would leave behind a loop that dead-code elimination does not remove.
        if (!cast->hasUses())
        {
            cast->removeAndDeallocate();
            return nullptr;
        }

        // Type legalization can map both array types to one type, e.g. when it lowers integer
        // matrices of either layout to the same array of vectors.
        if (isTypeEqual(toType, value->getDataType()))
        {
            cast->replaceUsesWith(value);
            cast->removeAndDeallocate();
            return nullptr;
        }

        IRBuilder builder(module);
        IRBuilderSourceLocRAII srcLocRAII(&builder, cast->sourceLoc);
        builder.setInsertBefore(cast);

        IRGlobalValueWithCode* splitCode = nullptr;
        IRInst* result = nullptr;
        // A loop needs a control-flow graph, so a cast directly in module scope, such as one in
        // a `static const` initializer, is unrolled whatever its length.
        auto count = as<IRIntLit>(toType->getElementCount());
        auto block = as<IRBlock>(cast->getParent());
        if (count && (count->getValue() <= kMaxUnrolledArrayElementCount || !block))
        {
            List<IRInst*> elements;
            for (IRIntegerValue i = 0; i < count->getValue(); i++)
            {
                auto element =
                    builder.emitCast(toElementType, builder.emitElementExtract(value, i));
                if (isArrayBuiltinCast(element))
                    workList.add(element);
                elements.add(element);
            }
            result = builder.emitMakeArray(toType, elements.getCount(), elements.getBuffer());
        }
        else
        {
            // A module-scope array whose length is a specialization constant cannot be emitted
            // even without a conversion, so we do not try to convert one.
            if (!block)
                SLANG_UNIMPLEMENTED_X("array layout conversion of a module-scope array whose "
                                      "length is not a literal");
            splitCode = as<IRGlobalValueWithCode>(block->getParent());

            builder.setInsertBefore(splitCode->getFirstBlock()->getFirstOrdinaryInst());
            auto resultVar = builder.emitVar(toType);

            auto tailBlock = splitBlockBefore(builder, cast);
            builder.setInsertInto(block);
            IRBlock* loopBodyBlock = nullptr;
            IRBlock* loopBreakBlock = nullptr;
            auto index = emitLoopBlocks(
                &builder,
                builder.getIntValue(builder.getIntType(), 0),
                builder.emitCast(builder.getIntType(), toType->getElementCount()),
                loopBodyBlock,
                loopBreakBlock);

            builder.setInsertBefore(loopBodyBlock->getTerminator());
            auto element =
                builder.emitCast(toElementType, builder.emitElementExtract(value, index));
            if (isArrayBuiltinCast(element))
                workList.add(element);
            builder.emitStore(builder.emitElementAddress(resultVar, index), element);

            builder.setInsertInto(loopBreakBlock);
            builder.emitBranch(tailBlock);

            builder.setInsertBefore(cast);
            result = builder.emitLoad(resultVar);
        }

        cast->replaceUsesWith(result);
        cast->removeAndDeallocate();
        return splitCode;
    }

    void processModule()
    {
        collectCasts(module->getModuleInst());

        HashSet<IRGlobalValueWithCode*> splitCodes;
        while (workList.getCount())
        {
            auto cast = workList.getLast();
            workList.removeLast();
            if (auto splitCode = lowerCast(cast))
                splitCodes.add(splitCode);
        }

        // The new blocks are appended at the end, so we restore an order in which each block
        // comes after the blocks that dominate it.
        for (auto code : splitCodes)
            sortBlocksInFunc(code);
    }
};

void lowerArrayBuiltinCasts(IRModule* module)
{
    ArrayBuiltinCastLoweringContext context;
    context.module = module;
    context.processModule();
}

} // namespace Slang
