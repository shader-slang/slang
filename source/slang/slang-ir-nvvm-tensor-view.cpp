#include "slang-ir-nvvm-tensor-view.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
void lowerNVVMTensorViews(IRModule* module)
{
    List<IRTensorViewType*> types;
    List<IRInst*> queries;
    List<IRInst*> workList;
    workList.add(module->getModuleInst());
    for (Index i = 0; i < workList.getCount(); ++i)
    {
        auto inst = workList[i];
        if (auto type = as<IRTensorViewType>(inst))
            types.add(type);
        switch (inst->getOp())
        {
        case kIROp_GetTensorViewData:
        case kIROp_GetTensorViewStride:
        case kIROp_GetTensorViewSize:
        case kIROp_GetTensorViewDimensionCount:
            queries.add(inst);
            break;
        }
        for (auto child : inst->getChildren())
            workList.add(child);
    }
    if (!types.getCount())
        return;

    // The host CUDA descriptor is independent of T: device-address word, five byte strides, five
    // extents and rank. Its offsets are 0, 8, 28 and 48, with size 56 and alignment 8. Keep
    // this representation at the type-lowering boundary, so ordinary entry decoding and
    // helper transport remain the sole owners of their respective ABIs.
    IRBuilder builder(module);
    builder.setInsertInto(module);
    auto record = builder.createStructType();
    builder.addNameHintDecoration(record, UnownedStringSlice("TensorViewStorage"));
    auto count = builder.getIntValue(builder.getIntType(), 5);
    auto uintArray = builder.getArrayType(builder.getUIntType(), count);
    auto data =
        builder.createStructField(record, builder.createStructKey(), builder.getUInt64Type());
    auto strides = builder.createStructField(record, builder.createStructKey(), uintArray);
    auto sizes = builder.createStructField(record, builder.createStructKey(), uintArray);
    auto rank = builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
    // The host ABI pads the final word. Represent it explicitly: natural-layout
    // records otherwise report an unrounded size of 52, which changes enclosing array strides.
    builder.createStructField(record, builder.createStructKey(), builder.getUIntType());
    for (auto type : types)
        type->replaceUsesWith(record);

    for (auto query : queries)
    {
        builder.setInsertBefore(query);
        IRBuilderSourceLocRAII sourceLoc(&builder, query->sourceLoc);
        IRStructField* field = nullptr;
        switch (query->getOp())
        {
        case kIROp_GetTensorViewData:
            field = data;
            break;
        case kIROp_GetTensorViewStride:
            field = strides;
            break;
        case kIROp_GetTensorViewSize:
            field = sizes;
            break;
        case kIROp_GetTensorViewDimensionCount:
            field = rank;
            break;
        default:
            SLANG_UNEXPECTED("unexpected tensor view query");
        }
        auto value =
            builder.emitFieldExtract(field->getFieldType(), query->getOperand(0), field->getKey());
        if (field == strides || field == sizes)
            value = builder.emitElementExtract(value, query->getOperand(1));
        query->replaceUsesWith(value);
        query->removeAndDeallocate();
    }
    for (auto type : types)
        type->removeAndDeallocate();
}
} // namespace Slang
