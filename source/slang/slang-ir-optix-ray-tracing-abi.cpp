// slang-ir-optix-ray-tracing-abi.cpp

#include "slang-ir-optix-ray-tracing-abi.h"

#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"

namespace Slang
{

// Returns true when preserving a scalar's bits requires a bit cast rather than a numeric cast.
static bool _isOptiXTransportFloatingPointType(IRType* type)
{
    return isFloatingType(type) || isPackedFloatType(type);
}

// Removes source-level attributes without changing the type's physical ABI identity.
static IRType* _unwrapOptiXTransportType(IRType* type)
{
    return as<IRType>(unwrapAttributedType(type));
}

// Selects the unsigned integer type with the same width as one transported scalar.
static Result _getOptiXScalarLayout(
    IRBuilder* builder,
    IRType* type,
    IRType*& outUnsignedType,
    IRIntegerValue& outByteSize)
{
    IRSizeAndAlignment layout;
    SLANG_RETURN_ON_FAIL(getSizeAndAlignment(nullptr, IRTypeLayoutRules::getCUDA(), type, &layout));
    switch (layout.size)
    {
    case 1:
        outUnsignedType = builder->getUInt8Type();
        break;
    case 2:
        outUnsignedType = builder->getUInt16Type();
        break;
    case 4:
        outUnsignedType = builder->getUIntType();
        break;
    case 8:
        outUnsignedType = builder->getUInt64Type();
        break;
    default:
        return SLANG_FAIL;
    }
    outByteSize = layout.size;
    return SLANG_OK;
}

// Converts a source scalar into the same-width unsigned bit pattern used by an OptiX word stream.
static IRInst* _convertOptiXScalarToUnsignedBits(
    IRBuilder* builder,
    IRType* type,
    IRType* unsignedType,
    IRInst* value)
{
    if (as<IRBoolType>(type))
        return builder->emitCast(unsignedType, value);
    if (_isOptiXTransportFloatingPointType(type))
        return builder->emitBitCast(unsignedType, value);
    return builder->emitCast(unsignedType, value);
}

// Reconstructs a source scalar from its same-width unsigned bit pattern.
static IRInst* _convertOptiXUnsignedBitsToScalar(IRBuilder* builder, IRType* type, IRInst* bits)
{
    if (as<IRBoolType>(type))
        return builder->emitNeq(bits, builder->getIntValue(bits->getDataType(), 0));
    if (_isOptiXTransportFloatingPointType(type))
        return builder->emitBitCast(type, bits);
    return builder->emitCast(type, bits);
}

Result getOptiXRayTracingPayloadABIInfo(
    IRBuilder* builder,
    IRType* type,
    OptiXRayTracingPayloadABIInfo* outInfo)
{
    SLANG_RELEASE_ASSERT(builder && type && outInfo);
    *outInfo = {};

    type = _unwrapOptiXTransportType(type);
    if (!type)
        return SLANG_FAIL;

    IRSizeAndAlignment layout;
    SLANG_RETURN_ON_FAIL(getSizeAndAlignment(nullptr, IRTypeLayoutRules::getCUDA(), type, &layout));
    if (layout.size == IRSizeAndAlignment::kIndeterminateSize)
        return SLANG_FAIL;

    // The CUDA prelude uses `PayloadRegisters<T, (sizeof(T) + 3) / 4>`. The compiler's CUDA IR
    // layout describes that emitted native `T`, so this calculation must use its complete
    // allocation size, including aggregate tail padding.
    const IRIntegerValue inlineRegisterCount =
        (layout.size + kOptiXRayTracingRegisterSize - 1) / kOptiXRayTracingRegisterSize;
    if (inlineRegisterCount > kOptiXMaxRayPayloadRegisterCount)
    {
        outInfo->registerCount = kOptiXIndirectPayloadRegisterCount;
        outInfo->isIndirect = true;
    }
    else
    {
        outInfo->registerCount = inlineRegisterCount;
    }
    return SLANG_OK;
}

enum class OptiXPayloadTransportMode
{
    Read,
    Write,
};

// Holds the native words and direction for one recursive payload transport operation.
struct OptiXPayloadTransportContext
{
    IRBuilder* builder = nullptr;
    OptiXPayloadTransportMode mode = OptiXPayloadTransportMode::Read;
    List<IRInst*> words;

    // Returns the current SSA value for a native payload word, fetching it on first use.
    IRInst* getWord(IRIntegerValue index)
    {
        if (index < 0 || index >= words.getCount())
            return nullptr;
        IRInst*& word = words[Index(index)];
        if (!word)
        {
            auto indexValue = builder->getIntValue(builder->getIntType(), index);
            word = builder->emitIntrinsicInst(
                builder->getUIntType(),
                kIROp_GetOptiXPayloadRegister,
                1,
                &indexValue);
        }
        return word;
    }
};

static Result _processOptiXPayloadValue(
    OptiXPayloadTransportContext& context,
    IRType* type,
    IRIntegerValue byteOffset,
    IRInst* sourceValue,
    IRInst*& outReadValue);

// Reads or writes one scalar at its byte offset in the emitted CUDA payload type.
static Result _processOptiXPayloadScalar(
    OptiXPayloadTransportContext& context,
    IRType* type,
    IRIntegerValue byteOffset,
    IRInst* sourceValue,
    IRInst*& outReadValue)
{
    auto builder = context.builder;
    IRType* unsignedType = nullptr;
    IRIntegerValue byteSize = 0;
    SLANG_RETURN_ON_FAIL(_getOptiXScalarLayout(builder, type, unsignedType, byteSize));

    const IRIntegerValue wordIndex = byteOffset / kOptiXRayTracingRegisterSize;
    const IRIntegerValue byteInWord = byteOffset % kOptiXRayTracingRegisterSize;
    if (byteInWord + byteSize > kOptiXRayTracingRegisterSize && byteSize != 8)
        return SLANG_FAIL;

    auto uintType = builder->getUIntType();
    auto uint64Type = builder->getUInt64Type();
    if (context.mode == OptiXPayloadTransportMode::Read)
    {
        IRInst* bits = nullptr;
        if (byteSize <= kOptiXRayTracingRegisterSize)
        {
            bits = context.getWord(wordIndex);
            if (!bits)
                return SLANG_FAIL;
            if (byteInWord)
            {
                bits = builder->emitShr(
                    uintType,
                    bits,
                    builder->getIntValue(uintType, byteInWord * 8));
            }
            if (unsignedType != uintType)
                bits = builder->emitCast(unsignedType, bits);
        }
        else
        {
            if (byteSize != 8 || byteInWord)
                return SLANG_FAIL;
            auto lowBits = context.getWord(wordIndex);
            auto highBits = context.getWord(wordIndex + 1);
            if (!lowBits || !highBits)
                return SLANG_FAIL;
            bits = builder->emitMakeUInt64(lowBits, highBits);
        }
        outReadValue = _convertOptiXUnsignedBitsToScalar(builder, type, bits);
        return SLANG_OK;
    }

    SLANG_RELEASE_ASSERT(sourceValue);
    IRInst* bits = _convertOptiXScalarToUnsignedBits(builder, type, unsignedType, sourceValue);
    if (byteSize == kOptiXRayTracingRegisterSize)
    {
        if (byteInWord)
            return SLANG_FAIL;
        context.words[Index(wordIndex)] = bits;
        return SLANG_OK;
    }
    if (byteSize == 8)
    {
        if (byteInWord || unsignedType != uint64Type)
            return SLANG_FAIL;
        context.words[Index(wordIndex)] = builder->emitCast(uintType, bits);
        auto shift = builder->getIntValue(uint64Type, 32);
        context.words[Index(wordIndex + 1)] =
            builder->emitCast(uintType, builder->emitShr(uint64Type, bits, shift));
        return SLANG_OK;
    }

    // A byte or half-word can share its register with other payload fields. Start from the
    // incoming word and accumulate every replacement in SSA; the caller emits one final set per
    // register after the complete aggregate has been visited.
    IRInst* oldWord = context.getWord(wordIndex);
    if (!oldWord)
        return SLANG_FAIL;
    IRInst* widenedBits = builder->emitCast(uintType, bits);
    const IRIntegerValue bitOffset = byteInWord * 8;
    const IRIntegerValue valueMask = byteSize == 1 ? 0xff : 0xffff;
    widenedBits =
        builder->emitBitAnd(uintType, widenedBits, builder->getIntValue(uintType, valueMask));
    if (bitOffset)
    {
        widenedBits =
            builder->emitShl(uintType, widenedBits, builder->getIntValue(uintType, bitOffset));
    }
    auto clearMask = builder->getIntValue(uintType, ~(valueMask << bitOffset));
    context.words[Index(wordIndex)] = builder->emitBitOr(
        uintType,
        builder->emitBitAnd(uintType, oldWord, clearMask),
        widenedBits);
    return SLANG_OK;
}

// Recursively transports one payload value while preserving its native CUDA aggregate offsets.
static Result _processOptiXPayloadValue(
    OptiXPayloadTransportContext& context,
    IRType* type,
    IRIntegerValue byteOffset,
    IRInst* sourceValue,
    IRInst*& outReadValue)
{
    type = _unwrapOptiXTransportType(type);
    if (!type)
        return SLANG_FAIL;

    if (as<IRVoidType>(type))
    {
        if (context.mode == OptiXPayloadTransportMode::Read)
            outReadValue = context.builder->getVoidValue();
        return SLANG_OK;
    }

    if (auto enumType = as<IREnumType>(type))
    {
        auto tagType = enumType->getTagType();
        IRInst* sourceTag = nullptr;
        if (context.mode == OptiXPayloadTransportMode::Write)
            sourceTag = context.builder->emitCast(tagType, sourceValue);
        IRInst* readTag = nullptr;
        SLANG_RETURN_ON_FAIL(
            _processOptiXPayloadValue(context, tagType, byteOffset, sourceTag, readTag));
        if (context.mode == OptiXPayloadTransportMode::Read)
            outReadValue = context.builder->emitCast(type, readTag);
        return SLANG_OK;
    }

    if (auto structType = as<IRStructType>(type))
    {
        List<IRInst*> readFields;
        for (auto field : structType->getFields())
        {
            IRIntegerValue fieldOffset = 0;
            SLANG_RETURN_ON_FAIL(
                getOffset(nullptr, IRTypeLayoutRules::getCUDA(), field, &fieldOffset));
            IRInst* sourceField = nullptr;
            if (context.mode == OptiXPayloadTransportMode::Write)
            {
                sourceField = context.builder->emitFieldExtract(
                    field->getFieldType(),
                    sourceValue,
                    field->getKey());
            }
            IRInst* readField = nullptr;
            SLANG_RETURN_ON_FAIL(_processOptiXPayloadValue(
                context,
                field->getFieldType(),
                byteOffset + fieldOffset,
                sourceField,
                readField));
            if (context.mode == OptiXPayloadTransportMode::Read)
                readFields.add(readField);
        }
        if (context.mode == OptiXPayloadTransportMode::Read)
            outReadValue = context.builder->emitMakeStruct(type, readFields);
        return SLANG_OK;
    }

    if (auto arrayType = as<IRArrayTypeBase>(type))
    {
        auto elementCount = as<IRIntLit>(arrayType->getElementCount());
        if (!elementCount)
            return SLANG_FAIL;
        IRSizeAndAlignment elementLayout;
        SLANG_RETURN_ON_FAIL(getSizeAndAlignment(
            nullptr,
            IRTypeLayoutRules::getCUDA(),
            arrayType->getElementType(),
            &elementLayout));
        const IRIntegerValue elementStride = elementLayout.getStride();
        List<IRInst*> readElements;
        for (IRIntegerValue i = 0; i < elementCount->getValue(); ++i)
        {
            IRInst* sourceElement = nullptr;
            if (context.mode == OptiXPayloadTransportMode::Write)
                sourceElement = context.builder->emitElementExtract(sourceValue, i);
            IRInst* readElement = nullptr;
            SLANG_RETURN_ON_FAIL(_processOptiXPayloadValue(
                context,
                arrayType->getElementType(),
                byteOffset + i * elementStride,
                sourceElement,
                readElement));
            if (context.mode == OptiXPayloadTransportMode::Read)
                readElements.add(readElement);
        }
        if (context.mode == OptiXPayloadTransportMode::Read)
        {
            outReadValue = context.builder->emitMakeArray(
                type,
                readElements.getCount(),
                readElements.getBuffer());
        }
        return SLANG_OK;
    }

    if (auto matrixType = as<IRMatrixType>(type))
    {
        auto rowCount = as<IRIntLit>(matrixType->getRowCount());
        if (!rowCount)
            return SLANG_FAIL;
        auto rowType = context.builder->getVectorType(
            matrixType->getElementType(),
            matrixType->getColumnCount());
        IRSizeAndAlignment rowLayout;
        SLANG_RETURN_ON_FAIL(
            getSizeAndAlignment(nullptr, IRTypeLayoutRules::getCUDA(), rowType, &rowLayout));
        const IRIntegerValue rowStride = rowLayout.getStride();
        List<IRInst*> readRows;
        for (IRIntegerValue i = 0; i < rowCount->getValue(); ++i)
        {
            IRInst* sourceRow = nullptr;
            if (context.mode == OptiXPayloadTransportMode::Write)
                sourceRow = context.builder->emitElementExtract(sourceValue, i);
            IRInst* readRow = nullptr;
            SLANG_RETURN_ON_FAIL(_processOptiXPayloadValue(
                context,
                rowType,
                byteOffset + i * rowStride,
                sourceRow,
                readRow));
            if (context.mode == OptiXPayloadTransportMode::Read)
                readRows.add(readRow);
        }
        if (context.mode == OptiXPayloadTransportMode::Read)
        {
            outReadValue = context.builder->emitIntrinsicInst(
                type,
                kIROp_MakeMatrix,
                readRows.getCount(),
                readRows.getBuffer());
        }
        return SLANG_OK;
    }

    if (auto vectorType = as<IRVectorType>(type))
    {
        auto elementCount = as<IRIntLit>(vectorType->getElementCount());
        if (!elementCount)
            return SLANG_FAIL;
        IRSizeAndAlignment elementLayout;
        SLANG_RETURN_ON_FAIL(getSizeAndAlignment(
            nullptr,
            IRTypeLayoutRules::getCUDA(),
            vectorType->getElementType(),
            &elementLayout));
        List<IRInst*> readElements;
        for (IRIntegerValue i = 0; i < elementCount->getValue(); ++i)
        {
            IRInst* sourceElement = nullptr;
            if (context.mode == OptiXPayloadTransportMode::Write)
                sourceElement = context.builder->emitElementExtract(sourceValue, i);
            IRInst* readElement = nullptr;
            SLANG_RETURN_ON_FAIL(_processOptiXPayloadValue(
                context,
                vectorType->getElementType(),
                byteOffset + i * elementLayout.size,
                sourceElement,
                readElement));
            if (context.mode == OptiXPayloadTransportMode::Read)
                readElements.add(readElement);
        }
        if (context.mode == OptiXPayloadTransportMode::Read)
        {
            outReadValue = context.builder->emitMakeVector(
                type,
                readElements.getCount(),
                readElements.getBuffer());
        }
        return SLANG_OK;
    }

    if (as<IRBasicType>(type) || isPackedFloatType(type))
    {
        return _processOptiXPayloadScalar(context, type, byteOffset, sourceValue, outReadValue);
    }

    return SLANG_FAIL;
}

// Allocates the payload-word state shared by the native-layout reader and writer.
static Result _initializeOptiXPayloadTransport(
    IRBuilder* builder,
    IRType* type,
    OptiXPayloadTransportContext& context)
{
    OptiXRayTracingPayloadABIInfo abiInfo;
    SLANG_RETURN_ON_FAIL(getOptiXRayTracingPayloadABIInfo(builder, type, &abiInfo));
    if (abiInfo.isIndirect)
        return SLANG_FAIL;
    context.builder = builder;
    context.words.setCount(Index(abiInfo.registerCount));
    for (Index i = 0; i < context.words.getCount(); ++i)
        context.words[i] = nullptr;
    return SLANG_OK;
}

Result emitOptiXRayTracingPayloadRead(IRBuilder* builder, IRType* type, IRInst** outValue)
{
    SLANG_RELEASE_ASSERT(builder && type && outValue);
    OptiXPayloadTransportContext context;
    SLANG_RETURN_ON_FAIL(_initializeOptiXPayloadTransport(builder, type, context));
    IRInst* result = nullptr;
    SLANG_RETURN_ON_FAIL(_processOptiXPayloadValue(context, type, 0, nullptr, result));
    *outValue = result;
    return SLANG_OK;
}

Result emitOptiXRayTracingPayloadWrite(IRBuilder* builder, IRInst* value)
{
    SLANG_RELEASE_ASSERT(builder && value);
    OptiXPayloadTransportContext context;
    context.mode = OptiXPayloadTransportMode::Write;
    SLANG_RETURN_ON_FAIL(_initializeOptiXPayloadTransport(builder, value->getDataType(), context));
    IRInst* unusedReadValue = nullptr;
    SLANG_RETURN_ON_FAIL(
        _processOptiXPayloadValue(context, value->getDataType(), 0, value, unusedReadValue));

    for (Index i = 0; i < context.words.getCount(); ++i)
    {
        auto word = context.words[i];
        if (!word)
            continue;
        IRInst* operands[] = {
            builder->getIntValue(builder->getIntType(), i),
            word,
        };
        builder->emitIntrinsicInst(
            builder->getVoidType(),
            kIROp_SetOptiXPayloadRegister,
            SLANG_COUNT_OF(operands),
            operands);
    }
    return SLANG_OK;
}

enum class OptiXHitAttributeTransportMode
{
    Count,
    Fetch,
    Report,
};

// Holds the cursor and producer/consumer state for one logical attribute-word traversal.
struct OptiXHitAttributeTransportContext
{
    IRBuilder* builder = nullptr;
    OptiXHitAttributeTransportMode mode = OptiXHitAttributeTransportMode::Count;
    IRIntegerValue registerCount = 0;
    List<IRInst*>* reportArguments = nullptr;
};

// Fetches the next physical uint32 attribute word and advances the shared cursor.
static IRInst* _emitOptiXHitAttributeRegisterFetch(OptiXHitAttributeTransportContext& context)
{
    auto builder = context.builder;
    auto index = builder->getIntValue(builder->getIntType(), context.registerCount++);
    auto uintType = builder->getUIntType();
    IRInst* operands[] = {uintType, index};
    return builder->emitIntrinsicInst(
        uintType,
        kIROp_GetOptiXHitAttribute,
        SLANG_COUNT_OF(operands),
        operands);
}

// Counts, fetches, or reports one scalar in the logical OptiX hit-attribute word stream.
static Result _processOptiXHitAttributeScalar(
    OptiXHitAttributeTransportContext& context,
    IRType* type,
    IRInst* sourceValue,
    IRInst*& outFetchedValue)
{
    IRType* unsignedType = nullptr;
    IRIntegerValue byteSize = 0;
    SLANG_RETURN_ON_FAIL(_getOptiXScalarLayout(context.builder, type, unsignedType, byteSize));
    const IRIntegerValue scalarRegisterCount =
        (byteSize + kOptiXRayTracingRegisterSize - 1) / kOptiXRayTracingRegisterSize;

    if (context.mode == OptiXHitAttributeTransportMode::Count)
    {
        context.registerCount += scalarRegisterCount;
        return SLANG_OK;
    }

    auto builder = context.builder;
    auto uintType = builder->getUIntType();
    auto uint64Type = builder->getUInt64Type();
    if (context.mode == OptiXHitAttributeTransportMode::Report)
    {
        SLANG_RELEASE_ASSERT(sourceValue && context.reportArguments);
        IRInst* bits = _convertOptiXScalarToUnsignedBits(builder, type, unsignedType, sourceValue);
        if (scalarRegisterCount == 1)
        {
            context.reportArguments->add(
                bits->getDataType() == uintType ? bits : builder->emitCast(uintType, bits));
            ++context.registerCount;
            return SLANG_OK;
        }

        SLANG_RELEASE_ASSERT(scalarRegisterCount == 2 && bits->getDataType() == uint64Type);
        context.reportArguments->add(builder->emitCast(uintType, bits));
        auto shift = builder->getIntValue(uint64Type, 32);
        context.reportArguments->add(
            builder->emitCast(uintType, builder->emitShr(uint64Type, bits, shift)));
        context.registerCount += 2;
        return SLANG_OK;
    }

    SLANG_RELEASE_ASSERT(context.mode == OptiXHitAttributeTransportMode::Fetch);
    IRInst* bits = nullptr;
    if (scalarRegisterCount == 1)
    {
        bits = _emitOptiXHitAttributeRegisterFetch(context);
        if (unsignedType != uintType)
            bits = builder->emitCast(unsignedType, bits);
    }
    else
    {
        SLANG_RELEASE_ASSERT(scalarRegisterCount == 2 && unsignedType == uint64Type);
        auto lowBits = _emitOptiXHitAttributeRegisterFetch(context);
        auto highBits = _emitOptiXHitAttributeRegisterFetch(context);
        bits = builder->emitMakeUInt64(lowBits, highBits);
    }
    outFetchedValue = _convertOptiXUnsignedBitsToScalar(builder, type, bits);
    return SLANG_OK;
}

// Walks one custom-attribute type for all three OptiX attribute-ABI consumers.
//
// Consider `struct Attributes { float2 uv; uint64_t key; }`. This traversal defines the register
// order as `uv.x`, `uv.y`, `key.low`, `key.high`. Reflection counts those four words, an
// intersection producer extracts them in that order, and closest-hit/any-hit reconstruction
// consumes the same order. Keeping the walk here prevents any consumer from inventing a subtly
// different flattening rule.
static Result _processOptiXHitAttributeValue(
    OptiXHitAttributeTransportContext& context,
    IRType* type,
    IRInst* sourceValue,
    IRInst*& outFetchedValue)
{
    type = _unwrapOptiXTransportType(type);
    if (!type)
        return SLANG_FAIL;

    if (as<IRVoidType>(type))
    {
        if (context.mode == OptiXHitAttributeTransportMode::Fetch)
            outFetchedValue = context.builder->getVoidValue();
        return SLANG_OK;
    }

    if (auto enumType = as<IREnumType>(type))
    {
        auto tagType = enumType->getTagType();
        IRInst* sourceTag = nullptr;
        if (context.mode == OptiXHitAttributeTransportMode::Report)
            sourceTag = context.builder->emitCast(tagType, sourceValue);
        IRInst* fetchedTag = nullptr;
        SLANG_RETURN_ON_FAIL(
            _processOptiXHitAttributeValue(context, tagType, sourceTag, fetchedTag));
        if (context.mode == OptiXHitAttributeTransportMode::Fetch)
            outFetchedValue = context.builder->emitCast(type, fetchedTag);
        return SLANG_OK;
    }

    if (auto structType = as<IRStructType>(type))
    {
        List<IRInst*> fetchedFields;
        for (auto field : structType->getFields())
        {
            IRInst* sourceField = nullptr;
            if (context.mode == OptiXHitAttributeTransportMode::Report)
            {
                sourceField = context.builder->emitFieldExtract(
                    field->getFieldType(),
                    sourceValue,
                    field->getKey());
            }
            IRInst* fetchedField = nullptr;
            SLANG_RETURN_ON_FAIL(_processOptiXHitAttributeValue(
                context,
                field->getFieldType(),
                sourceField,
                fetchedField));
            if (context.mode == OptiXHitAttributeTransportMode::Fetch)
                fetchedFields.add(fetchedField);
        }
        if (context.mode == OptiXHitAttributeTransportMode::Fetch)
            outFetchedValue = context.builder->emitMakeStruct(type, fetchedFields);
        return SLANG_OK;
    }

    if (auto arrayType = as<IRArrayTypeBase>(type))
    {
        auto elementCount = as<IRIntLit>(arrayType->getElementCount());
        if (!elementCount)
            return SLANG_FAIL;
        List<IRInst*> fetchedElements;
        for (IRIntegerValue i = 0; i < elementCount->getValue(); ++i)
        {
            IRInst* sourceElement = nullptr;
            if (context.mode == OptiXHitAttributeTransportMode::Report)
                sourceElement = context.builder->emitElementExtract(sourceValue, i);
            IRInst* fetchedElement = nullptr;
            SLANG_RETURN_ON_FAIL(_processOptiXHitAttributeValue(
                context,
                arrayType->getElementType(),
                sourceElement,
                fetchedElement));
            if (context.mode == OptiXHitAttributeTransportMode::Fetch)
                fetchedElements.add(fetchedElement);
        }
        if (context.mode == OptiXHitAttributeTransportMode::Fetch)
        {
            outFetchedValue = context.builder->emitMakeArray(
                type,
                fetchedElements.getCount(),
                fetchedElements.getBuffer());
        }
        return SLANG_OK;
    }

    if (auto matrixType = as<IRMatrixType>(type))
    {
        auto rowCount = as<IRIntLit>(matrixType->getRowCount());
        if (!rowCount)
            return SLANG_FAIL;
        auto rowType = context.builder->getVectorType(
            matrixType->getElementType(),
            matrixType->getColumnCount());
        List<IRInst*> fetchedRows;
        for (IRIntegerValue i = 0; i < rowCount->getValue(); ++i)
        {
            IRInst* sourceRow = nullptr;
            if (context.mode == OptiXHitAttributeTransportMode::Report)
                sourceRow = context.builder->emitElementExtract(sourceValue, i);
            IRInst* fetchedRow = nullptr;
            SLANG_RETURN_ON_FAIL(
                _processOptiXHitAttributeValue(context, rowType, sourceRow, fetchedRow));
            if (context.mode == OptiXHitAttributeTransportMode::Fetch)
                fetchedRows.add(fetchedRow);
        }
        if (context.mode == OptiXHitAttributeTransportMode::Fetch)
        {
            outFetchedValue = context.builder->emitIntrinsicInst(
                type,
                kIROp_MakeMatrix,
                fetchedRows.getCount(),
                fetchedRows.getBuffer());
        }
        return SLANG_OK;
    }

    if (auto vectorType = as<IRVectorType>(type))
    {
        auto elementCount = as<IRIntLit>(vectorType->getElementCount());
        if (!elementCount)
            return SLANG_FAIL;
        List<IRInst*> fetchedElements;
        for (IRIntegerValue i = 0; i < elementCount->getValue(); ++i)
        {
            IRInst* sourceElement = nullptr;
            if (context.mode == OptiXHitAttributeTransportMode::Report)
                sourceElement = context.builder->emitElementExtract(sourceValue, i);
            IRInst* fetchedElement = nullptr;
            SLANG_RETURN_ON_FAIL(_processOptiXHitAttributeValue(
                context,
                vectorType->getElementType(),
                sourceElement,
                fetchedElement));
            if (context.mode == OptiXHitAttributeTransportMode::Fetch)
                fetchedElements.add(fetchedElement);
        }
        if (context.mode == OptiXHitAttributeTransportMode::Fetch)
        {
            outFetchedValue = context.builder->emitMakeVector(
                type,
                fetchedElements.getCount(),
                fetchedElements.getBuffer());
        }
        return SLANG_OK;
    }

    if (as<IRBasicType>(type) || isPackedFloatType(type))
        return _processOptiXHitAttributeScalar(context, type, sourceValue, outFetchedValue);

    return SLANG_FAIL;
}

Result getOptiXRayTracingHitAttributeRegisterCount(
    IRBuilder* builder,
    IRType* type,
    IRIntegerValue* outRegisterCount)
{
    SLANG_RELEASE_ASSERT(builder && type && outRegisterCount);
    OptiXHitAttributeTransportContext context;
    context.builder = builder;
    IRInst* unusedFetchedValue = nullptr;
    auto result = _processOptiXHitAttributeValue(context, type, nullptr, unusedFetchedValue);
    *outRegisterCount = context.registerCount;
    return result;
}

Result emitOptiXRayTracingHitAttributeFetch(
    IRBuilder* builder,
    IRType* type,
    IRInst** outValue,
    IRIntegerValue* outRegisterCount)
{
    SLANG_RELEASE_ASSERT(builder && type && outValue && outRegisterCount);
    OptiXHitAttributeTransportContext context;
    context.builder = builder;
    context.mode = OptiXHitAttributeTransportMode::Fetch;
    auto result = _processOptiXHitAttributeValue(context, type, nullptr, *outValue);
    *outRegisterCount = context.registerCount;
    return result;
}

Result emitOptiXRayTracingHitAttributeReportArguments(
    IRBuilder* builder,
    IRInst* value,
    List<IRInst*>& outArguments)
{
    SLANG_RELEASE_ASSERT(builder && value);
    OptiXHitAttributeTransportContext context;
    context.builder = builder;
    context.mode = OptiXHitAttributeTransportMode::Report;
    context.reportArguments = &outArguments;
    IRInst* unusedFetchedValue = nullptr;
    return _processOptiXHitAttributeValue(context, value->getDataType(), value, unusedFetchedValue);
}

} // namespace Slang
