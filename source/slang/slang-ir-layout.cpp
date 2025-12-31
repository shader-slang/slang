// slang-ir-layout.cpp
#include "slang-ir-layout.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"

// This file implements facilities for computing and caching layout
// information on IR types.
//
// Unlike the AST-level layout system, this code currently only
// handles the notion of "natural" layout for IR types, which is
// the layout they use when stored in general-purpose memory
// without additional constraints.
//
// In general, "natural" layout for all targets is assumed to follow
// the same basic rules:
//
// * Scalars are all naturally aligned and have the "obvious" size
//
// * Arrays are laid out by separating elements by their "stride" (size rounded up to alignment)
//
// * Vectors are laid out as arrays of elements
//
// * Matrices are laid out as arrays of rows
//
// * Structures are laid out by packing fields in order, placing each field on the "next"
//   suitably aligned offset. The alignment of a structure is the maximum alignment of
//   its fields.
//
// Right now this file implements a one-size-fits-all version of natural
// layout that might not be a perfect fit for all targets. In particular
// this code currently assumes:
//
// * The `bool` type is laid out as 4 bytes (equivalent to an `int`)
//
// * The size of a structure or array type is *not* rounded up to a multiple
//   of its alignment. This means that fields may be laid out in
//   the "tail padding" of previous fields in the same structure. This is
//   correct behavior for VK/D3D, but does not match the behavior of typical
//   C/C++ compilers.
//
// * All matrices are laid out in row-major order, regardless of any
//   settings in user code.
//
// TODO: Addressing the above issues would require extending this file to somehow
// get target-specific layout information as an input. One option would be
// to attach information about "natural" layout on the target to the `IRModuleInst`
// as a decoration, similar to how an LLVM IR module stores a "layout string."

namespace Slang
{
static Result _calcArraySizeAndAlignment(
    TargetRequest* targetReq,
    IRTypeLayoutRules* rules,
    IRType* elementType,
    IRInst* elementCountInst,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    auto elementCountLit = as<IRIntLit>(elementCountInst);
    if (!elementCountLit)
        return SLANG_FAIL;
    auto elementCount = elementCountLit->getValue();

    if (elementCount == 0)
    {
        *outSizeAndAlignment = IRSizeAndAlignment(0, 1);
        return SLANG_OK;
    }

    IRSizeAndAlignment elementTypeLayout;
    SLANG_RETURN_ON_FAIL(getSizeAndAlignment(targetReq, rules, elementType, &elementTypeLayout));

    elementTypeLayout = rules->alignCompositeElement(elementTypeLayout);
    *outSizeAndAlignment = IRSizeAndAlignment(
        elementTypeLayout.getStride() * (elementCount - 1) + elementTypeLayout.size,
        elementTypeLayout.alignment);
    return SLANG_OK;
}

IRIntegerValue getIntegerValueFromInst(IRInst* inst)
{
    SLANG_ASSERT(inst->getOp() == kIROp_IntLit);
    return as<IRIntLit>(inst)->value.intVal;
}

Result IRTypeLayoutRules::calcSizeAndAlignment(
    TargetRequest* targetReq,
    IRType* type,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    TargetBuiltinTypeLayoutInfo builtinTypeInfo = getBuiltinTypeLayoutInfo(targetReq);

    switch (type->getOp())
    {

#define CASE(TYPE, SIZE, ALIGNMENT)                                 \
    case kIROp_##TYPE##Type:                                        \
        *outSizeAndAlignment = IRSizeAndAlignment(SIZE, ALIGNMENT); \
        return SLANG_OK /* end */

        // Most base types are "naturally aligned" (meaning alignment and size are the same)
#define BASE(TYPE, SIZE) CASE(TYPE, SIZE, SIZE)

        BASE(Int8, 1);
        BASE(UInt8, 1);

        BASE(Int16, 2);
        BASE(UInt16, 2);
        BASE(Half, 2);

        BASE(Int, 4);
        BASE(UInt, 4);
        BASE(Float, 4);

        BASE(Int64, 8);
        BASE(UInt64, 8);
        BASE(Double, 8);

        // We are currently handling `bool` following the HLSL
        // precednet of storing it in 4 bytes.
        //
        BASE(Bool, 4);

        // The Slang `void` type is treated as a zero-byte
        // type, so that it does not influence layout at all.
        //
        CASE(Void, 0, 1);

#undef BASE

#undef CASE

    case kIROp_StructType:
        {
            auto structType = cast<IRStructType>(type);
            IRSizeAndAlignment structLayout;
            IRIntegerValue offset = 0;
            IRIntegerValue lastFieldAlignment = 0;
            IRType* lastFieldType = NULL;
            bool seenFinalUnsizedArrayField = false;
            for (auto field : structType->getFields())
            {
                // If we failed to catch an unsized array earlier in the pipeline,
                // this will pick it up before generating nonsense results for
                // subsequent offsets
                SLANG_ASSERT(!seenFinalUnsizedArrayField);

                IRSizeAndAlignment fieldTypeLayout;
                SLANG_RETURN_ON_FAIL(
                    getSizeAndAlignment(targetReq, this, field->getFieldType(), &fieldTypeLayout));
                seenFinalUnsizedArrayField =
                    fieldTypeLayout.size == IRSizeAndAlignment::kIndeterminateSize;

                if (auto offsetDecor =
                        field->getKey()->findDecoration<IRVkStructOffsetDecoration>())
                {
                    offset = offsetDecor->getOffset()->getValue();
                }
                else
                {
                    offset = adjustOffset(
                        offset,
                        fieldTypeLayout.size,
                        lastFieldType,
                        lastFieldAlignment);
                }

                structLayout.size = align(offset, fieldTypeLayout.alignment);
                structLayout.alignment =
                    std::max(structLayout.alignment, fieldTypeLayout.alignment);

                IRIntegerValue fieldOffset = structLayout.size;
                if (auto module = type->getModule())
                {
                    // If we are in a situation where attaching new
                    // decorations is possible, then we want to
                    // cache the field offset on the IR field
                    // instruction.
                    //
                    IRBuilder builder(module);

                    auto intType = builder.getIntType();
                    builder.addDecoration(
                        field,
                        kIROp_OffsetDecoration,
                        builder.getIntValue(intType, (IRIntegerValue)ruleName),
                        builder.getIntValue(intType, fieldOffset));
                }
                if (!seenFinalUnsizedArrayField)
                    structLayout.size += fieldTypeLayout.size;
                offset = structLayout.size;
                lastFieldType = field->getFieldType();
                lastFieldAlignment = fieldTypeLayout.alignment;
            }
            *outSizeAndAlignment = alignCompositeElement(structLayout);
            return SLANG_OK;
        }
        break;

    case kIROp_ArrayType:
        {
            auto arrayType = cast<IRArrayType>(type);

            return _calcArraySizeAndAlignment(
                targetReq,
                this,
                arrayType->getElementType(),
                arrayType->getElementCount(),
                outSizeAndAlignment);
        }
        break;

    case kIROp_AtomicType:
        {
            auto atomicType = cast<IRAtomicType>(type);
            calcSizeAndAlignment(targetReq, atomicType->getElementType(), outSizeAndAlignment);
            return SLANG_OK;
        }
        break;

    case kIROp_UnsizedArrayType:
        {
            auto unsizedArrayType = cast<IRUnsizedArrayType>(type);
            getSizeAndAlignment(
                targetReq,
                this,
                unsizedArrayType->getElementType(),
                outSizeAndAlignment);
            outSizeAndAlignment->size = IRSizeAndAlignment::kIndeterminateSize;
            return SLANG_OK;
        }
        break;

    case kIROp_VectorType:
        {
            auto vecType = cast<IRVectorType>(type);
            IRSizeAndAlignment elementTypeLayout;
            getSizeAndAlignment(targetReq, this, vecType->getElementType(), &elementTypeLayout);
            *outSizeAndAlignment = getVectorSizeAndAlignment(
                elementTypeLayout,
                getIntegerValueFromInst(vecType->getElementCount()));
            return SLANG_OK;
        }
        break;
    case kIROp_AnyValueType:
        {
            auto anyValType = cast<IRAnyValueType>(type);
            outSizeAndAlignment->size = getIntVal(anyValType->getSize());
            outSizeAndAlignment->alignment = 4;
            *outSizeAndAlignment = alignCompositeElement(*outSizeAndAlignment);
            return SLANG_OK;
        }
        break;
    case kIROp_TupleType:
        {
            auto tupleType = cast<IRTupleType>(type);
            IRSizeAndAlignment resultLayout;
            IRIntegerValue lastFieldAlignment = 0;
            IRType* lastFieldType = NULL;
            for (UInt i = 0; i < tupleType->getOperandCount(); i++)
            {
                auto elementType = tupleType->getOperand(i);
                IRSizeAndAlignment fieldTypeLayout;
                SLANG_RETURN_ON_FAIL(
                    getSizeAndAlignment(targetReq, this, (IRType*)elementType, &fieldTypeLayout));
                resultLayout.size = adjustOffset(
                    resultLayout.size,
                    fieldTypeLayout.size,
                    lastFieldType,
                    lastFieldAlignment);
                resultLayout.size = align(resultLayout.size, fieldTypeLayout.alignment);
                resultLayout.alignment =
                    std::max(resultLayout.alignment, fieldTypeLayout.alignment);

                resultLayout.size += fieldTypeLayout.size;
                lastFieldType = as<IRType>(elementType);
                lastFieldAlignment = fieldTypeLayout.alignment;
            }
            *outSizeAndAlignment = alignCompositeElement(resultLayout);
            return SLANG_OK;
        }
        break;
    case kIROp_WitnessTableType:
    case kIROp_WitnessTableIDType:
    case kIROp_RTTIHandleType:
        {
            outSizeAndAlignment->size = kRTTIHandleSize;
            outSizeAndAlignment->alignment = 4;
            return SLANG_OK;
        }
        break;
    case kIROp_SetTagType:
        {
            outSizeAndAlignment->size = 4;
            outSizeAndAlignment->alignment = 4;
            return SLANG_OK;
        }
        break;
    case kIROp_InterfaceType:
        {
            auto interfaceType = cast<IRInterfaceType>(type);
            auto size = getInterfaceAnyValueSize(interfaceType, interfaceType->sourceLoc);
            size += kRTTIHeaderSize;
            size = align(size, 4);
            IRSizeAndAlignment resultLayout;
            resultLayout.size = size;
            resultLayout.alignment = 4;
            *outSizeAndAlignment = alignCompositeElement(resultLayout);
            return SLANG_OK;
        }
        break;
    case kIROp_MatrixType:
        {
            auto matType = cast<IRMatrixType>(type);
            IRBuilder builder(type->getModule());
            if (getIntegerValueFromInst(matType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
            {
                auto colVector =
                    builder.getVectorType(matType->getElementType(), matType->getRowCount());
                return _calcArraySizeAndAlignment(
                    targetReq,
                    this,
                    colVector,
                    matType->getColumnCount(),
                    outSizeAndAlignment);
            }
            else
            {
                auto rowVector =
                    builder.getVectorType(matType->getElementType(), matType->getColumnCount());
                return _calcArraySizeAndAlignment(
                    targetReq,
                    this,
                    rowVector,
                    matType->getRowCount(),
                    outSizeAndAlignment);
            }
        }
        break;
    case kIROp_IntPtrType:
    case kIROp_UIntPtrType:
    case kIROp_OutParamType:
    case kIROp_BorrowInOutParamType:
    case kIROp_RefParamType:
    case kIROp_BorrowInParamType:
    case kIROp_RawPointerType:
    case kIROp_PtrType:
    case kIROp_NativePtrType:
    case kIROp_ComPtrType:
    case kIROp_NativeStringType:
    case kIROp_RaytracingAccelerationStructureType:
    case kIROp_FuncType:
        // If we don't know the target, we can't tell the size of these types
        // yet as it is target-dependent.
        if (targetReq)
        {
            *outSizeAndAlignment = IRSizeAndAlignment(
                builtinTypeInfo.genericPointerSize,
                builtinTypeInfo.genericPointerSize);
            return SLANG_OK;
        }
        break;
    case kIROp_ScalarBufferLayoutType:
    case kIROp_CBufferLayoutType:
    case kIROp_Std140BufferLayoutType:
    case kIROp_Std430BufferLayoutType:
    case kIROp_DefaultBufferLayoutType:
        *outSizeAndAlignment = IRSizeAndAlignment(0, 4);
        return SLANG_OK;
    case kIROp_DescriptorHandleType:
        {
            IRBuilder builder(type);
            builder.setInsertBefore(type);
            auto uintType = builder.getUIntType();
            auto uint2Type = builder.getVectorType(uintType, 2);
            return getSizeAndAlignment(targetReq, this, uint2Type, outSizeAndAlignment);
        }
    case kIROp_AttributedType:
        {
            auto attributedType = cast<IRAttributedType>(type);
            SLANG_ASSERT(attributedType->getAttr()->getOp() == kIROp_NoDiffAttr);
            return getSizeAndAlignment(
                targetReq,
                this,
                attributedType->getBaseType(),
                outSizeAndAlignment);
        }
    case kIROp_EnumType:
        {
            auto enumType = cast<IREnumType>(type);
            auto tagType = enumType->getTagType();
            return calcSizeAndAlignment(targetReq, tagType, outSizeAndAlignment);
        }
        break;
    default:
        break;
    }
    if (as<IRResourceTypeBase>(type) || as<IRSamplerStateTypeBase>(type))
    {
        *outSizeAndAlignment = IRSizeAndAlignment(8, 8);
        return SLANG_OK;
    }

    return SLANG_FAIL;
}

IRSizeAndAlignmentDecoration* findSizeAndAlignmentDecorationForLayout(
    IRType* type,
    IRTypeLayoutRuleName layoutName)
{
    for (auto decorInst : type->getDecorations())
    {
        if (auto decor = as<IRSizeAndAlignmentDecoration>(decorInst))
        {
            if (decor->getLayoutName() == layoutName)
                return decor;
        }
    }
    return nullptr;
}

Result getSizeAndAlignment(
    TargetRequest* targetReq,
    IRTypeLayoutRules* rules,
    IRType* type,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    if (auto decor = findSizeAndAlignmentDecorationForLayout(type, rules->ruleName))
    {
        *outSizeAndAlignment = IRSizeAndAlignment(decor->getSize(), (int)decor->getAlignment());
        return SLANG_OK;
    }

    IRSizeAndAlignment sizeAndAlignment;
    SLANG_RETURN_ON_FAIL(rules->calcSizeAndAlignment(targetReq, type, &sizeAndAlignment));

    if (auto module = type->getModule())
    {
        IRBuilder builder(module);

        auto intType = builder.getIntType();
        auto int64Type = builder.getInt64Type();
        builder.addDecoration(
            type,
            kIROp_SizeAndAlignmentDecoration,
            builder.getIntValue(intType, (IRIntegerValue)rules->ruleName),
            builder.getIntValue(int64Type, sizeAndAlignment.size),
            builder.getIntValue(intType, sizeAndAlignment.alignment));
    }

    *outSizeAndAlignment = sizeAndAlignment;
    return SLANG_OK;
}
IROffsetDecoration* findOffsetDecorationForLayout(
    IRStructField* field,
    IRTypeLayoutRuleName layoutName)
{
    for (auto decorInst : field->getDecorations())
    {
        if (auto decor = as<IROffsetDecoration>(decorInst))
        {
            if (decor->getLayoutName() == layoutName)
                return decor;
        }
    }
    return nullptr;
}

Result getOffset(
    TargetRequest* targetReq,
    IRTypeLayoutRules* rules,
    IRStructField* field,
    IRIntegerValue* outOffset)
{
    if (auto decor = findOffsetDecorationForLayout(field, rules->ruleName))
    {
        *outOffset = decor->getOffset();
        return SLANG_OK;
    }

    // Offsets are computed as part of layout out types,
    // so we expect that layout of the "parent" type
    // of the field should add an offset to it if
    // possible.

    auto structType = as<IRStructType>(field->getParent());
    if (!structType)
        return SLANG_FAIL;

    IRSizeAndAlignment structTypeLayout;
    SLANG_RETURN_ON_FAIL(getSizeAndAlignment(targetReq, rules, structType, &structTypeLayout));

    if (auto decor = findOffsetDecorationForLayout(field, rules->ruleName))
    {
        *outOffset = decor->getOffset();
        return SLANG_OK;
    }

    // If attempting to lay out the parent type didn't
    // cause the field to get an offset, then we are
    // in an unexpected case with no easy answer.
    //
    return SLANG_FAIL;
}

struct NaturalLayoutRules : IRTypeLayoutRules
{
    NaturalLayoutRules() { ruleName = IRTypeLayoutRuleName::Natural; }
    virtual IRIntegerValue adjustOffset(
        IRIntegerValue offset,
        IRIntegerValue elementSize,
        IRType* lastFieldType,
        IRIntegerValue lastFieldAlignment)
    {
        SLANG_UNUSED(elementSize);
        SLANG_UNUSED(lastFieldType);
        SLANG_UNUSED(lastFieldAlignment);
        return offset;
    }

    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        return elementSize;
    }

    virtual IRSizeAndAlignment getVectorSizeAndAlignment(
        IRSizeAndAlignment element,
        IRIntegerValue count)
    {
        return IRSizeAndAlignment(element.size * count, element.alignment);
    }
};

struct CLayoutRules : IRTypeLayoutRules
{
    CLayoutRules() { ruleName = IRTypeLayoutRuleName::C; }

    virtual Result calcSizeAndAlignment(
        TargetRequest* targetReq,
        IRType* type,
        IRSizeAndAlignment* outSizeAndAlignment)
    {
        if (type->getOp() == kIROp_BoolType)
        {
            *outSizeAndAlignment = IRSizeAndAlignment(1, 1);
            return SLANG_OK;
        }
        return IRTypeLayoutRules::calcSizeAndAlignment(targetReq, type, outSizeAndAlignment);
    }

    virtual IRIntegerValue adjustOffset(
        IRIntegerValue offset,
        IRIntegerValue elementSize,
        IRType* lastFieldType,
        IRIntegerValue lastFieldAlignment)
    {
        SLANG_UNUSED(elementSize);
        SLANG_UNUSED(lastFieldType);
        return align(offset, (int)lastFieldAlignment);
    }

    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        IRSizeAndAlignment alignedSize = elementSize;
        alignedSize.size = align(alignedSize.size, alignedSize.alignment);
        return alignedSize;
    }

    virtual IRSizeAndAlignment getVectorSizeAndAlignment(
        IRSizeAndAlignment element,
        IRIntegerValue count)
    {
        return IRSizeAndAlignment(element.size * count, element.alignment);
    }
};

struct ConstantBufferLayoutRules : IRTypeLayoutRules
{
    ConstantBufferLayoutRules() { ruleName = IRTypeLayoutRuleName::D3DConstantBuffer; }

    /// Next member only aligns to 16 if the next member is an array/matrix/struct
    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment currentSize)
    {
        // Matrix/Array/Struct should be aligned on a new register
        return IRSizeAndAlignment(currentSize.size, 16);
    }

    virtual IRIntegerValue adjustOffset(
        IRIntegerValue offset,
        IRIntegerValue elementSize,
        IRType* lastFieldType,
        IRIntegerValue lastFieldAlignment)
    {
        SLANG_UNUSED(lastFieldType);
        SLANG_UNUSED(lastFieldAlignment);

        // If the element would cross a 16-byte boundary, align to the next boundary
        auto currentChunk = offset / 16;
        auto endChunk = (offset + elementSize - 1) / 16;
        if (currentChunk != endChunk)
        {
            return align(offset, 16);
        }
        return offset;
    }

    virtual IRSizeAndAlignment getVectorSizeAndAlignment(
        IRSizeAndAlignment element,
        IRIntegerValue count)
    {
        return IRSizeAndAlignment(element.size * count, element.alignment);
    }
};

struct Std430LayoutRules : IRTypeLayoutRules
{
    Std430LayoutRules() { ruleName = IRTypeLayoutRuleName::Std430; }

    virtual IRIntegerValue adjustOffset(
        IRIntegerValue offset,
        IRIntegerValue elementSize,
        IRType* lastFieldType,
        IRIntegerValue lastFieldAlignment)
    {
        SLANG_UNUSED(elementSize);
        if (as<IRMatrixType>(lastFieldType) || as<IRArrayTypeBase>(lastFieldType) ||
            as<IRStructType>(lastFieldType))
        {
            return align(offset, (int)lastFieldAlignment);
        }
        return offset;
    }

    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        return elementSize;
    }

    virtual IRSizeAndAlignment getVectorSizeAndAlignment(
        IRSizeAndAlignment element,
        IRIntegerValue count)
    {
        IRIntegerValue countForAlignment = count;
        if (count == 3)
            countForAlignment = 4;
        return IRSizeAndAlignment(
            (int)(element.size * count),
            (int)(element.size * countForAlignment));
    }
};

struct Std140LayoutRules : IRTypeLayoutRules
{
    Std140LayoutRules() { ruleName = IRTypeLayoutRuleName::Std140; }

    virtual IRIntegerValue adjustOffset(
        IRIntegerValue offset,
        IRIntegerValue elementSize,
        IRType* lastFieldType,
        IRIntegerValue lastFieldAlignment)
    {
        SLANG_UNUSED(elementSize);
        if (as<IRMatrixType>(lastFieldType) || as<IRArrayTypeBase>(lastFieldType) ||
            as<IRStructType>(lastFieldType))
        {
            return align(offset, (int)lastFieldAlignment);
        }
        return offset;
    }

    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        elementSize.alignment = (int)align(elementSize.alignment, 16);
        elementSize.size = align(elementSize.size, elementSize.alignment);
        return elementSize;
    }

    virtual IRSizeAndAlignment getVectorSizeAndAlignment(
        IRSizeAndAlignment element,
        IRIntegerValue count)
    {
        IRIntegerValue alignmentCount = count;
        if (count == 3)
            alignmentCount = 4;
        return IRSizeAndAlignment(
            (int)(element.size * count),
            (int)(element.size * alignmentCount));
    }
};

struct LLVMLayoutRules : IRTypeLayoutRules
{
    LLVMLayoutRules() { ruleName = IRTypeLayoutRuleName::LLVM; }

    virtual Result calcSizeAndAlignment(
        TargetRequest* targetReq,
        IRType* type,
        IRSizeAndAlignment* outSizeAndAlignment)
    {
        if (type->getOp() == kIROp_BoolType)
        {
            *outSizeAndAlignment = IRSizeAndAlignment(1, 1);
            return SLANG_OK;
        }
        return IRTypeLayoutRules::calcSizeAndAlignment(targetReq, type, outSizeAndAlignment);
    }

    virtual IRIntegerValue adjustOffset(
        IRIntegerValue offset,
        IRIntegerValue elementSize,
        IRType* lastFieldType,
        IRIntegerValue lastFieldAlignment)
    {
        SLANG_UNUSED(elementSize);
        SLANG_UNUSED(lastFieldType);
        return align(offset, (int)lastFieldAlignment);
    }

    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        IRSizeAndAlignment alignedSize = elementSize;
        alignedSize.size = align(alignedSize.size, alignedSize.alignment);
        return alignedSize;
    }

    virtual IRSizeAndAlignment getVectorSizeAndAlignment(
        IRSizeAndAlignment element,
        IRIntegerValue count)
    {
        // Round alignment to next power of two
        IRIntegerValue alignment = element.alignment;
        while (alignment < element.size * count)
            alignment *= 2;
        return IRSizeAndAlignment(element.size * count, (int)alignment);
    }
};

Result getNaturalSizeAndAlignment(
    TargetRequest* targetReq,
    IRType* type,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    return getSizeAndAlignment(
        targetReq,
        IRTypeLayoutRules::getNatural(),
        type,
        outSizeAndAlignment);
}

Result getNaturalOffset(TargetRequest* targetReq, IRStructField* field, IRIntegerValue* outOffset)
{
    return getOffset(targetReq, IRTypeLayoutRules::getNatural(), field, outOffset);
}


//////////////////////////
// Std430 Layout
//////////////////////////

Result getStd430SizeAndAlignment(
    TargetRequest* targetReq,
    IRType* type,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    return getSizeAndAlignment(
        targetReq,
        IRTypeLayoutRules::getStd430(),
        type,
        outSizeAndAlignment);
}

Result getStd430Offset(TargetRequest* targetReq, IRStructField* field, IRIntegerValue* outOffset)
{
    return getOffset(targetReq, IRTypeLayoutRules::getStd430(), field, outOffset);
}

IRTypeLayoutRules* IRTypeLayoutRules::getStd430()
{
    static Std430LayoutRules rules;
    return &rules;
}
IRTypeLayoutRules* IRTypeLayoutRules::getStd140()
{
    static Std140LayoutRules rules;
    return &rules;
}
IRTypeLayoutRules* IRTypeLayoutRules::getNatural()
{
    static NaturalLayoutRules rules;
    return &rules;
}

IRTypeLayoutRules* IRTypeLayoutRules::getC()
{
    static CLayoutRules rules;
    return &rules;
}

IRTypeLayoutRules* IRTypeLayoutRules::getLLVM()
{
    static LLVMLayoutRules rules;
    return &rules;
}

IRTypeLayoutRules* IRTypeLayoutRules::getConstantBuffer()
{
    static ConstantBufferLayoutRules rules;
    return &rules;
}

IRTypeLayoutRules* IRTypeLayoutRules::get(IRTypeLayoutRuleName name)
{
    switch (name)
    {
    case IRTypeLayoutRuleName::Std430:
        return getStd430();
    case IRTypeLayoutRuleName::Std140:
        return getStd140();
    case IRTypeLayoutRuleName::Natural:
    case IRTypeLayoutRuleName::MetalParameterBlock:
        return getNatural();
    case IRTypeLayoutRuleName::C:
        return getC();
    case IRTypeLayoutRuleName::D3DConstantBuffer:
        return getConstantBuffer();
    case IRTypeLayoutRuleName::LLVM:
        return getLLVM();
    default:
        return nullptr;
    }
}

} // namespace Slang
