#include "slang-ir-extract-value-from-type.h"

#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#define CHECK(x) SLANG_RELEASE_ASSERT((x) == SLANG_OK)

namespace Slang
{

// Represents the result of finding the leaf-level value in a type that contains the
// the entirety or the first half of the requested value at the specified offset.
struct FindLeafValueResult
{
    IRInst* leafValue = nullptr; // The leaf-level value.
    uint32_t valueSize = 0;      // The size of the leaf-level value.
    uint32_t offsetInValue =
        0; // The offset in bytes within `leafValue` that contains the requested value.
};

// bitcast the leaf value to the same size as leaf value's type.
// For type that has size smaller than 4 bytes, we will need to cast them
// to 32-bit unsigned int first, and then cast to the target type.
IRInst* bitCastLeafValue(IRBuilder& builder, FindLeafValueResult& leaf)
{
    auto resultValue = leaf.leafValue;

    IRType* intermediateUintType = nullptr;
    IRType* targetUintType = nullptr;
    switch (leaf.valueSize)
    {
    case 1:
        intermediateUintType = builder.getUInt8Type();
        targetUintType = builder.getUIntType();
        break;
    case 2:
        intermediateUintType = builder.getUInt16Type();
        targetUintType = builder.getUIntType();
        break;
    case 4:
        intermediateUintType = builder.getUIntType();
        targetUintType = intermediateUintType;
        break;
    case 8:
        intermediateUintType = builder.getUInt64Type();
        targetUintType = intermediateUintType;
        break;
    default:
        SLANG_UNEXPECTED("Unsupported value size");
        break;
    }
    resultValue = builder.emitBitCast(intermediateUintType, resultValue);

    // In case of 1-byte or 2-byte value, we need to cast it to 32-bit unsigned int first
    // because we don't allow bitCast from 1-byte or 2-byte type to 32-bit type.
    if (intermediateUintType != targetUintType)
    {
        resultValue = builder.emitCast(targetUintType, resultValue);
        resultValue = builder.emitBitCast(targetUintType, resultValue);
    }

    return resultValue;
}

FindLeafValueResult findLeafValueAtOffset(
    TargetProgram* targetProgram,
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
                CHECK(getNaturalSizeAndAlignment(
                    targetProgram->getTargetReq(),
                    field->getFieldType(),
                    &fieldLayout));
                CHECK(getNaturalOffset(targetProgram->getTargetReq(), field, &fieldOffset));
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
                        targetProgram,
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
            CHECK(getNaturalSizeAndAlignment(
                targetProgram->getTargetReq(),
                elementType,
                &elementLayout));
            if (elementLayout.getStride() == 0)
            {
                result.leafValue = builder.getIntValue(builder.getUIntType(), 0);
                result.valueSize = 4;
                result.offsetInValue = 0;
                return result;
            }
            uint32_t index = offset / (uint32_t)elementLayout.getStride();
            auto elementValue = builder.emitElementExtract(
                elementType,
                src,
                builder.getIntValue(builder.getUIntType(), index));
            return findLeafValueAtOffset(
                targetProgram,
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
            CHECK(getNaturalSizeAndAlignment(
                targetProgram->getTargetReq(),
                elementType,
                &elementLayout));
            uint32_t index =
                elementLayout.getStride() == 0 ? 0 : (uint32_t)(offset / elementLayout.getStride());
            auto elementValue = builder.emitElementExtract(
                elementType,
                src,
                builder.getIntValue(builder.getUIntType(), index));
            return findLeafValueAtOffset(
                targetProgram,
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
            CHECK(getNaturalSizeAndAlignment(targetProgram->getTargetReq(), rowType, &rowLayout));
            uint32_t rowIndex = rowLayout.getStride() == 0
                                    ? 0
                                    : (uint32_t)(offset / (columnCount * rowLayout.getStride()));
            auto rowValue = builder.emitElementExtract(
                rowType,
                src,
                builder.getIntValue(builder.getUIntType(), rowIndex));
            return findLeafValueAtOffset(
                targetProgram,
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
    TargetProgram* targetProgram,
    IRType* dataType,
    IRSizeAndAlignment& layout,
    IRInst* src,
    uint32_t offset)
{
    auto leaf = findLeafValueAtOffset(targetProgram, builder, dataType, layout, src, offset);
    auto resultValue = bitCastLeafValue(builder, leaf);
    auto uintType = resultValue->getDataType();

    if (leaf.offsetInValue != 0)
    {
        uint32_t shift = leaf.offsetInValue * 8;
        resultValue = builder.emitShr(uintType, resultValue, builder.getIntValue(uintType, shift));

        resultValue = builder.emitBitAnd(
            builder.getUIntType(),
            resultValue,
            builder.getIntValue(builder.getUIntType(), 0xFF));
    }
    return resultValue;
}

IRInst* extractMultiByteValueAtOffset(
    IRBuilder& builder,
    TargetProgram* targetProgram,
    IRType* dataType,
    IRSizeAndAlignment& layout,
    IRInst* src,
    uint32_t size,
    uint32_t offset)
{
    if (size == 1)
        return extractByteAtOffset(builder, targetProgram, dataType, layout, src, offset);

    auto leaf = findLeafValueAtOffset(targetProgram, builder, dataType, layout, src, offset);
    if (leaf.valueSize - leaf.offsetInValue >= size)
    {
        // The request value is fully contained in the found leaf element.
        // We can proceed to extract the requested bits from the element.
        auto resultValue = bitCastLeafValue(builder, leaf);
        auto uintType = resultValue->getDataType();

        uint32_t shift = leaf.offsetInValue * 8;
        if (shift > 0)
            resultValue =
                builder.emitShr(uintType, resultValue, builder.getIntValue(uintType, shift));
        uint32_t bitMask = 0;
        switch (size)
        {
        case 1:
            bitMask = 0xFF;
            break;
        case 2:
            bitMask = 0xFFFF;
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
        auto firstHalfSize = leaf.valueSize - leaf.offsetInValue;
        auto firstHalf = extractMultiByteValueAtOffset(
            builder,
            targetProgram,
            dataType,
            layout,
            src,
            firstHalfSize,
            offset);

        auto uintType = firstHalf->getDataType();
        switch (firstHalfSize)
        {
        case 1:
            firstHalf =
                builder.emitBitAnd(uintType, firstHalf, builder.getIntValue(uintType, 0xFF));
            break;
        case 2:
            firstHalf =
                builder.emitBitAnd(uintType, firstHalf, builder.getIntValue(uintType, 0xFFFF));
            break;
        case 3:
            firstHalf =
                builder.emitBitAnd(uintType, firstHalf, builder.getIntValue(uintType, 0xFFFFFF));
            break;
        default:
            break;
        }
        auto restSize = size - firstHalfSize;
        auto secondHalf = extractMultiByteValueAtOffset(
            builder,
            targetProgram,
            dataType,
            layout,
            src,
            restSize,
            offset + firstHalfSize);

        IRType* resultType = builder.getUIntType();
        if (size > 4)
        {
            resultType = builder.getUInt64Type();
            firstHalf = builder.emitCast(resultType, firstHalf);
            secondHalf = builder.emitCast(resultType, secondHalf);
        }

        uint32_t shift = firstHalfSize * 8;
        auto resultValue = builder.emitBitOr(
            resultType,
            firstHalf,
            builder.emitShl(
                resultType,
                secondHalf,
                builder.getIntValue(builder.getUIntType(), shift)));
        return resultValue;
    }
}

IRInst* extractValueAtOffset(
    IRBuilder& builder,
    TargetProgram* targetProgram,
    IRInst* src,
    uint32_t offset,
    uint32_t size)
{
    auto dataType = src->getDataType();
    IRSizeAndAlignment typeLayout;
    SLANG_RETURN_NULL_ON_FAIL(
        getNaturalSizeAndAlignment(targetProgram->getTargetReq(), dataType, &typeLayout));
    if (offset + size > typeLayout.size)
    {
        return builder.getIntValue(builder.getIntType(), 0);
    }
    return extractMultiByteValueAtOffset(
        builder,
        targetProgram,
        dataType,
        typeLayout,
        src,
        size,
        offset);
}

} // namespace Slang
