#include "slang-ir-wrap-cbuffer-element.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

// This pass implements a simple translation that wraps the element type T in a ConstantBuffer<T>
// (or ParameterBlock<T>) type in `struct S { T inner; }`, and replace the ConstantBuffer<T> type
// with ConstantBuffer<S>. This is needed because some backends do not allow certain types to be
// used directly as the element type of a constant buffer.
// For example, Metal does not allow `ParameterBlock<StructuredBuffer<int>>` as that will create
// a double pointer that Metal compiler does not like. We can easily work around this limitation
// by wrapping the `StructuredBuffer<int>` in a struct.

namespace Slang
{

void maybeProvideNameHint(
    IRBuilder& builder,
    IRStructType* wrappedStructType,
    IRParameterGroupType* originalParamGroupType)
{
    StringBuilder sb;
    sb << "wrapper_";
    getTypeNameHint(sb, originalParamGroupType->getElementType());
    builder.addNameHintDecoration(wrappedStructType, sb.produceString().getUnownedSlice());
}

void wrapCBufferElements(IRModule* module, WrapCBufferElementPolicy* policy)
{
    struct WorkItem
    {
        IRStructKey* wrappedFieldKey;
        IRInst* inst;
        IRInst* newParameterGroupType;
    };

    IRBuilder builder(module);

    List<WorkItem> workList;
    for (auto globalInst : module->getGlobalInsts())
    {
        // Discover all insts whose type is a parameter group type.
        if (auto paramGroupType = as<IRParameterGroupType>(globalInst))
        {
            if (!policy->shouldWrapBufferElementInStruct(paramGroupType))
                continue;

            // Create the wrapper struct.
            builder.setInsertBefore(paramGroupType);
            auto structType = builder.createStructType();
            maybeProvideNameHint(builder, structType, paramGroupType);
            auto fieldKey = builder.createStructKey();
            builder.addNameHintDecoration(fieldKey, toSlice("inner"));
            builder.createStructField(structType, fieldKey, paramGroupType->getElementType());

            // Create the new parameter group type whose element is the wrapper struct.
            List<IRInst*> bufferTypeOperands;
            bufferTypeOperands.add(structType);
            for (UInt i = 1; i < paramGroupType->getOperandCount(); ++i)
            {
                bufferTypeOperands.add(paramGroupType->getOperand(i));
            }
            auto newParameterGroupType = builder.getType(
                paramGroupType->getOp(),
                (UInt)bufferTypeOperands.getCount(),
                bufferTypeOperands.getArrayView().getBuffer());

            // Traverse all uses of the parameter group type, and add them to the work list
            // for further processing.
            traverseUses(
                paramGroupType,
                [&](IRUse* use)
                {
                    if (use->getUser()->getFullType() != paramGroupType)
                        return;
                    WorkItem item;
                    item.wrappedFieldKey = fieldKey;
                    item.inst = use->getUser();
                    workList.add(item);
                });
            paramGroupType->replaceUsesWith(newParameterGroupType);
        }
    }

    // Now we have a work list of all instructions that uses a parameter group.
    // We need to update all uses of parameter group x with `x.inner` instead.
    for (auto item : workList)
    {
        traverseUses(
            item.inst,
            [&](IRUse* use)
            {
                auto user = use->getUser();
                IRBuilder builder(user);
                builder.setInsertBefore(user);

                // Note that we insert the field address instruction right before each use, instead
                // of immediately after the original parameter group inst, because the parameter
                // group inst may be defined in a scope that does not allow field address
                // instructions.
                auto unwrapped = builder.emitFieldAddress(item.inst, item.wrappedFieldKey);
                builder.replaceOperand(use, unwrapped);
            });
    }
}

class MetalWrapCBufferElementPolicy : public WrapCBufferElementPolicy
{
public:
    virtual bool shouldWrapBufferElementInStruct(IRParameterGroupType* cbufferType) override
    {
        // Metal allows structs, scalars, vectors and matrices directly as buffer elements.
        if (as<IRStructType>(cbufferType->getElementType()))
            return false;
        if (as<IRBasicType>(cbufferType->getElementType()))
            return false;
        if (as<IRMatrixType>(cbufferType->getElementType()))
            return false;
        if (as<IRVectorType>(cbufferType->getElementType()))
            return false;

        // Wrap everything else in a struct.
        return true;
    }
};

void wrapCBufferElementsForMetal(IRModule* module)
{
    MetalWrapCBufferElementPolicy policy = {};
    wrapCBufferElements(module, &policy);
}

} // namespace Slang
