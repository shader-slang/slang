#include "slang-ir-lower-copy-logical.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
struct LowerCopyLogicalContext
{
    bool onlyUntypedPtrOperand = false;

    List<IRCopyLogical*> copyLogicalInsts;

    // A copyLogical through an untyped SPIR-V pointer must be lowered element-wise even on
    // SPIR-V 1.4+ (see the header comment / #13022); this identifies that case. In the motivating
    // case the source (`getVal`, the descriptor-heap `ConstantBuffer`) is untyped and the
    // destination is a function-local. The `getPtr` (destination) disjunct is forward-looking: no
    // current path produces an untyped copyLogical destination (descriptor-heap RW structured
    // buffers use typed `StorageBuffer` pointers on 1.4+), but checking both operands keeps the
    // predicate honest to its name should an untyped destination ever arise.
    static bool hasUntypedPtrOperand(IRCopyLogical* copyLogicalInst)
    {
        return as<IRSPIRVUntypedPtrType>(copyLogicalInst->getVal()->getDataType()) ||
               as<IRSPIRVUntypedPtrType>(copyLogicalInst->getPtr()->getDataType());
    }

    void findCopyLogicalInsts(IRInst* inst)
    {
        if (auto copyLogicalInst = as<IRCopyLogical>(inst))
        {
            if (!onlyUntypedPtrOperand || hasUntypedPtrOperand(copyLogicalInst))
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

    // Build a field/element address that keeps an untyped `SPIRVUntypedPtr` base's flavor. The
    // auto-deducing `emitFieldAddress`/`emitElementAddress` overloads always build a typed
    // `IRPtrType`; using them on an untyped descriptor-heap pointer would drop the untyped flavor,
    // so a nested field/element access derived from it would carry a typed IR pointer and emit a
    // typed access chain on an untyped base -- invalid SPIR-V (#13022). Only untyped bases are
    // special-cased; every other pointer flavor keeps the original auto-deduced result.
    //
    // This is the same "a field/element pointer of an untyped base is itself untyped" rule that
    // `processFieldAddress`/`processGetElementPtrImpl` apply in spirv-legalize. It must be
    // reapplied here because this lowering runs after both `processWorkList()` drains in
    // `SPIRVLegalizationContext::processModule`, so the addresses it creates are never revisited by
    // those retype passes and have to carry the untyped flavor up front.
    static IRInst* emitFieldAddressKeepingFlavor(
        IRBuilder& builder,
        IRInst* basePtr,
        IRStructField* field)
    {
        if (auto untypedBase = as<IRSPIRVUntypedPtrType>(basePtr->getDataType()))
            return builder.emitFieldAddress(
                builder.getPtrType(field->getFieldType(), untypedBase),
                basePtr,
                field->getKey());
        return builder.emitFieldAddress(basePtr, field->getKey());
    }

    static IRInst* emitElementAddressKeepingFlavor(
        IRBuilder& builder,
        IRInst* basePtr,
        IRInst* index,
        IRType* elementType)
    {
        if (auto untypedBase = as<IRSPIRVUntypedPtrType>(basePtr->getDataType()))
            return builder.emitElementAddress(
                builder.getPtrType(elementType, untypedBase),
                basePtr,
                index);
        return builder.emitElementAddress(basePtr, index);
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
                auto srcFieldValue = emitFieldAddressKeepingFlavor(builder, srcPtr, srcFields[i]);
                auto dstFieldPtr = emitFieldAddressKeepingFlavor(builder, destPtr, dstFields[i]);
                lowerCopyLogicalWithDestImpl(builder, dstFieldPtr, srcFieldValue);
            }
        }
        else if (auto srcArrayType = as<IRArrayType>(srcValType))
        {
            auto dstArrayType = as<IRArrayTypeBase>(destValType);
            SLANG_RELEASE_ASSERT(dstArrayType && "Mismatched types in copy-logical inst");
            auto srcElementType = srcArrayType->getElementType();
            auto dstElementType = dstArrayType->getElementType();
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
                    auto index = builder.getIntValue(builder.getIntType(), i);
                    auto srcElement =
                        emitElementAddressKeepingFlavor(builder, srcPtr, index, srcElementType);
                    auto dstElementPtr =
                        emitElementAddressKeepingFlavor(builder, destPtr, index, dstElementType);
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
                auto srcElement =
                    emitElementAddressKeepingFlavor(builder, srcPtr, loopParam, srcElementType);
                auto dstElementPtr =
                    emitElementAddressKeepingFlavor(builder, destPtr, loopParam, dstElementType);
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

void lowerCopyLogical(IRModule* module, bool onlyUntypedPtrOperand)
{
    LowerCopyLogicalContext context;
    context.onlyUntypedPtrOperand = onlyUntypedPtrOperand;
    context.processModule(module);
}
} // namespace Slang
