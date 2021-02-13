#include "slang-ir-extract-value-from-type.h"
#include "slang-ir-layout.h"
#include "slang-ir-insts.h"
#define CHECK(x) SLANG_RELEASE_ASSERT((x) == SLANG_OK)

namespace Slang
{

// Represents the result of finding the leaf-level value in a type that contains the
// the entirety or the first half of the requested value at the specified offset.
struct FindLeafValueResult
{
    IRInst* leafValue = nullptr; // The leaf-level value.
    uint32_t valueSize = 0; // The size of the leaf-level value.
    uint32_t offsetInValue = 0; // The offset in bytes within `leafValue` that contains the requested value.
};

FindLeafValueResult findLeafValueAtOffset(
    TargetRequest* targetReq,
    IRBuilder& builder,
    IRType* dataType,
    IRSizeAndAlignment& layout,
    IRInst* src,
    uint32_t offset)
{
    FindLeafValueResult result;
    if (offset >= layout.size && offset < layout.getStride())
    {
        // We are extracting bits beyond the type size but within the stride boundary,
        // return a 0 value in this case.
        result.leafValue = builder.getIntValue(builder.getUIntType(), 0);
        result.valueSize = 4;
        result.offsetInValue = (uint32_t)(offset - layout.size);
        return result;
    }
    switch (dataType->getOp())
    {
    case kIROp_StructType:
        {
            auto structType = as<IRStructType>(dataType);
            for (auto field : structType->getFields())
            {
                IRIntegerValue fieldOffset = 0;
                IRSizeAndAlignment fieldLayout;
                CHECK(getNaturalSizeAndAlignment(targetReq, field->getFieldType(), &fieldLayout));
                CHECK(getNaturalOffset(targetReq, field, &fieldOffset));
                if (fieldOffset + fieldLayout.size > offset)
                {
                    if (fieldOffset > offset)
                    {
                        // This field is starting after the requested offset,
                        // therefore the requested value is located at the "gap"
                        // between aligned fields, in this case the requested value
                        // is 0.
                        result.leafValue = builder.getIntValue(builder.getUIntType(), 0);
                        result.valueSize = 4;
                        result.offsetInValue = (uint32_t)(fieldOffset - offset);
                        return result;
                    }
                    // The field contains requested value. We want to recursively
                    // traverse the field type to reach a leaf case.
                    auto fieldValue =
                        builder.emitFieldExtract(field->getFieldType(), src, field->getKey());
                    return findLeafValueAtOffset(
                        targetReq,
                        builder,
                        field->getFieldType(),
                        fieldLayout,
                        fieldValue,
                        (uint32_t)(offset - fieldOffset));
                }
            }
            result.leafValue = builder.getIntValue(builder.getUIntType(), 0);
            result.valueSize = 4;
            result.offsetInValue = (uint32_t)(offset - layout.size);
            return result;
        }
        break;
    case kIROp_ArrayType:
        {
            auto arrayType = as<IRArrayType>(dataType);
            auto elementType = arrayType->getElementType();
            IRSizeAndAlignment elementLayout;
            CHECK(getNaturalSizeAndAlignment(targetReq, elementType, &elementLayout));
            if (elementLayout.getStride() == 0)
            {
                result.leafValue = builder.getIntValue(builder.getUIntType(), 0);
                result.valueSize = 4;
                result.offsetInValue = 0;
                return result;
            }
            uint32_t index = offset / (uint32_t)elementLayout.getStride();
            auto elementValue = builder.emitElementExtract(
                elementType, src, builder.getIntValue(builder.getUIntType(), index));
            return findLeafValueAtOffset(
                targetReq,
                builder,
                elementType,
                elementLayout,
                elementValue,
                (uint32_t)(offset - elementLayout.getStride() * index));
        }
        break;
    case kIROp_VectorType:
        {
            auto vectorType = as<IRVectorType>(dataType);
            auto elementType = vectorType->getElementType();
            IRSizeAndAlignment elementLayout;
            CHECK(getNaturalSizeAndAlignment(targetReq, elementType, &elementLayout));
            uint32_t index =
                elementLayout.getStride() == 0 ? 0 : (uint32_t)(offset / elementLayout.getStride());
            auto elementValue = builder.emitElementExtract(
                elementType, src, builder.getIntValue(builder.getUIntType(), index));
            return findLeafValueAtOffset(
                targetReq,
                builder,
                elementType,
                elementLayout,
                elementValue,
                (uint32_t)(offset - elementLayout.getStride() * index));
        }
        break;
    case kIROp_MatrixType:
        {
            // Note: this code is assuming row major odering.
            auto matrixType = as<IRMatrixType>(dataType);
            auto elementType = matrixType->getElementType();
            SLANG_RELEASE_ASSERT(matrixType->getColumnCount()->getOp() == kIROp_IntLit);
            auto columnCount = as<IRIntLit>(matrixType->getColumnCount())->value.intVal;
            auto rowType = builder.getVectorType(elementType, matrixType->getColumnCount());
            IRSizeAndAlignment rowLayout;
            CHECK(getNaturalSizeAndAlignment(targetReq, rowType, &rowLayout));
            uint32_t rowIndex = rowLayout.getStride() == 0
                                    ? 0
                                    : (uint32_t)(offset / (columnCount * rowLayout.getStride()));
            auto rowValue = builder.emitElementExtract(
                rowType, src, builder.getIntValue(builder.getUIntType(), rowIndex));
            return findLeafValueAtOffset(
                targetReq,
                builder,
                rowType,
                rowLayout,
                rowValue,
                (uint32_t)(offset - rowLayout.getStride() * rowIndex));
        }
        break;
    default:
        {
            result.leafValue = src;
            result.offsetInValue = offset;
            result.valueSize = (uint32_t)layout.size;
            return result;
        }
        break;
    }
}

IRInst* extractByteAtOffset(
    IRBuilder& builder,
    TargetRequest* targetReq,
    IRType* dataType,
    IRSizeAndAlignment& layout,
    IRInst* src,
    uint32_t offset)
{
    auto leaf = findLeafValueAtOffset(targetReq, builder, dataType, layout, src, offset);
    IRType* uintType = nullptr;
    if (leaf.valueSize <= 4)
    {
        uintType = builder.getUIntType();
    }
    else
    {
        uintType = builder.getUInt64Type();
    }
    auto resultValue = builder.emitBitCast(uintType, leaf.leafValue);
    if (leaf.offsetInValue != 0)
    {
        uint32_t shift = leaf.offsetInValue * 8;
        resultValue = builder.emitShr(uintType, resultValue, builder.getIntValue(uintType, shift));

        resultValue = builder.emitBitAnd(
            builder.getUIntType(),
            resultValue, builder.getIntValue(builder.getUIntType(), 0xFF));
    }
    return resultValue;
}

IRInst* extractMultiByteValueAtOffset(
    IRBuilder& builder,
    TargetRequest* targetReq,
    IRType* dataType,
    IRSizeAndAlignment& layout,
    IRInst* src,
    uint32_t size,
    uint32_t offset)
{
    if (size == 1)
        return extractByteAtOffset(builder, targetReq, dataType, layout, src, offset);

    auto leaf = findLeafValueAtOffset(targetReq, builder, dataType, layout, src, offset);
    auto resultValue = leaf.leafValue;
    IRType* uintType = nullptr;
    if (leaf.valueSize <= 4)
    {
        uintType = builder.getUIntType();
    }
    else
    {
        uintType = builder.getUInt64Type();
    }
    if (leaf.valueSize - leaf.offsetInValue >= size)
    {
        // The request value is fully contained in the found leaf element.
        // We can proceed to extract the requested bits from the element.
        uint32_t shift = leaf.offsetInValue * 8;
        if (shift > 0)
            resultValue = builder.emitShr(uintType, resultValue, builder.getIntValue(uintType, shift));
        uint32_t bitMask = 0;
        switch (size)
        {
        case 1:
            bitMask = 0xFF;
            break;
        case 2:
            bitMask = 0xFFFFF;
            break;
        case 3:
            bitMask = 0xFFFFFF;
            break;
        case 4:
            bitMask = 0xFFFFFFFF;
            break;
        default:
            break;
        }
        if (leaf.valueSize != size)
        {
            resultValue =
                builder.emitBitAnd(uintType, resultValue, builder.getIntValue(uintType, bitMask));
        }
        return resultValue;
    }
    else
    {
        // The requested value crosses the boundaries of different fields.
        // We need to extract first and second half separately, and combine them together.
        auto firstHalf = extractMultiByteValueAtOffset(
            builder, targetReq, dataType, layout, src, size / 2, offset);
        auto secondHalf = extractMultiByteValueAtOffset(
            builder, targetReq, dataType, layout, src, size / 2, offset + size / 2);
        uint32_t shift = (size / 2) * 8;
        resultValue = builder.emitAdd(
            builder.getUIntType(),
            firstHalf,
            builder.emitShl(
                builder.getUIntType(),
                secondHalf,
                builder.getIntValue(builder.getUIntType(), shift)));
        return resultValue;
    }
}

IRInst* extractValueAtOffset(
    IRBuilder& builder, TargetRequest* targetReq, IRInst* src, uint32_t offset, uint32_t size)
{
    auto dataType = src->getDataType();
    IRSizeAndAlignment typeLayout;
    SLANG_RETURN_NULL_ON_FAIL(getNaturalSizeAndAlignment(targetReq, dataType, &typeLayout));
    if (offset + size > typeLayout.size)
    {
        return builder.getIntValue(builder.getIntType(), 0);
    }
    return extractMultiByteValueAtOffset(
        builder, targetReq, dataType, typeLayout, src, size, offset);
}

} // namespace Slang
