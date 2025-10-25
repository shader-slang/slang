#include "slang-ir-lower-copy-logical.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
struct LowerCopyLogicalContext
{
    List<IRCopyLogical*> copyLogicalInsts;
    void findCopyLogicalInsts(IRInst* inst)
    {
        if (auto copyLogicalInst = as<IRCopyLogical>(inst))
        {
            copyLogicalInsts.add(copyLogicalInst);
            return;
        }
        for (auto child : inst->getChildren())
        {
            findCopyLogicalInsts(child);
        }
    }
    void processModule(IRModule* module)
    {
        findCopyLogicalInsts(module->getModuleInst());
        for (auto copyLogicalInst : copyLogicalInsts)
        {
            processCopyLogicalInst(copyLogicalInst);
        }
    }

    void processCopyLogicalInst(IRCopyLogical* copyLogicalInst)
    {
        IRBuilder builder(copyLogicalInst);
        builder.setInsertBefore(copyLogicalInst);

        IRInst* srcPtr = copyLogicalInst->getVal();
        IRInst* destPtr = copyLogicalInst->getPtr();
        lowerCopyLogicalWithDestImpl(builder, destPtr, srcPtr);
        copyLogicalInst->removeAndDeallocate();
    }

    List<IRStructField*> getFieldList(IRStructType* structType)
    {
        List<IRStructField*> fields;
        for (auto field : structType->getFields())
        {
            fields.add(field);
        }
        return fields;
    }

    IRBlock* splitBlockAt(IRInsertLoc insertLoc)
    {
        IRBuilder builder(insertLoc.getInst());
        builder.setInsertBefore(insertLoc.getInst());
        if (insertLoc.getMode() == IRInsertLoc::Mode::Before)
        {
            auto newBlock = builder.emitBlock();
            for (auto inst = insertLoc.getInst(); inst;)
            {
                auto nextInst = inst->getNextInst();
                inst->insertAtEnd(newBlock);
                inst = nextInst;
            }
            return newBlock;
        }
        if (insertLoc.getMode() == IRInsertLoc::Mode::AtEnd)
        {
            // Nothing to split, just return a new block.
            return builder.emitBlock();
        }
        else if (insertLoc.getMode() == IRInsertLoc::Mode::AtStart)
        {
            return insertLoc.getBlock();
        }
        else
        {
            SLANG_UNIMPLEMENTED_X("splitBlockAt: unsupported insert loc mode");
        }
    }

    void lowerCopyLogicalWithDestImpl(IRBuilder& builder, IRInst* destPtr, IRInst* srcPtr)
    {
        auto destValType = tryGetPointedToType(&builder, destPtr->getDataType());
        auto srcValType = tryGetPointedToType(&builder, srcPtr->getDataType());
        // Generate code to copy each field from source to destination.
        if (auto srcStructType = as<IRStructType>(srcValType))
        {
            auto dstStructType = as<IRStructType>(destValType);
            SLANG_RELEASE_ASSERT(dstStructType && "Mismatched types in copy-logical inst");
            auto srcFields = getFieldList(srcStructType);
            auto dstFields = getFieldList(dstStructType);
            SLANG_RELEASE_ASSERT(
                srcFields.getCount() == dstFields.getCount() &&
                "Mismatched field count in copy-logical operand struct types.");
            for (Index i = 0; i < srcFields.getCount(); i++)
            {
                auto srcFieldKey = srcFields[i]->getKey();
                auto dstFieldKey = dstFields[i]->getKey();
                auto srcFieldValue = builder.emitFieldAddress(srcPtr, srcFieldKey);
                auto dstFieldPtr = builder.emitFieldAddress(destPtr, dstFieldKey);
                lowerCopyLogicalWithDestImpl(builder, dstFieldPtr, srcFieldValue);
            }
        }
        else if (auto srcArrayType = as<IRArrayType>(srcValType))
        {
            auto elementCount = srcArrayType->getElementCount();
            IRIntegerValue elementCountIntLit = 0xFFFFFFFF;
            if (as<IRIntLit>(elementCount))
            {
                elementCountIntLit = getIntVal(elementCount);
            }
            if (elementCountIntLit <= 16)
            {
                // If array is small, just unroll the copy for each element.
                for (IRIntegerValue i = 0; i < elementCountIntLit; i++)
                {
                    auto dstArrayType = as<IRArrayTypeBase>(destValType);
                    SLANG_RELEASE_ASSERT(dstArrayType && "Mismatched types in copy-logical inst");
                    auto srcElement = builder.emitElementAddress(
                        srcPtr,
                        builder.getIntValue(builder.getIntType(), i));
                    auto dstElementPtr = builder.emitElementAddress(
                        destPtr,
                        builder.getIntValue(builder.getIntType(), i));
                    lowerCopyLogicalWithDestImpl(builder, dstElementPtr, srcElement);
                }
            }
            else
            {
                // For bigger arrays, emit a loop to do the copy.
                IRBlock* loopBodyBlock;
                IRBlock* loopBreakBlock;
                auto loopParam = emitLoopBlocks(
                    &builder,
                    builder.getIntValue(builder.getIntType(), 0),
                    builder.emitCast(builder.getIntType(), elementCount),
                    loopBodyBlock,
                    loopBreakBlock);
                auto afterBlock = splitBlockAt(builder.getInsertLoc());
                builder.setInsertBefore(loopBodyBlock->getFirstOrdinaryInst());
                auto srcElement = builder.emitElementAddress(srcPtr, loopParam);
                auto dstElementPtr = builder.emitElementAddress(destPtr, loopParam);
                lowerCopyLogicalWithDestImpl(builder, dstElementPtr, srcElement);
                builder.setInsertInto(loopBreakBlock);
                builder.emitBranch(afterBlock);
                builder.setInsertBefore(afterBlock->getFirstChild());
            }
        }
        else
        {
            // Base case: just do a store.
            auto srcValue = builder.emitLoad(srcPtr);
            if (srcValType != destValType)
                srcValue = builder.emitCast(destValType, srcValue);
            builder.emitStore(destPtr, srcValue);
        }
    }
};

void lowerCopyLogical(IRModule* module)
{
    LowerCopyLogicalContext context;
    context.processModule(module);
}
} // namespace Slang
