// slang-ir-layout.cpp
#include "slang-ir-layout.h"

#include "slang-ir-insts.h"

#include "slang-ir-generics-lowering-context.h"

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
    TargetRequest* target,
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
    SLANG_RETURN_ON_FAIL(getSizeAndAlignment(target, rules, elementType, &elementTypeLayout));

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

static Result _calcSizeAndAlignment(
    TargetRequest* target,
    IRTypeLayoutRules* rules,
    IRType* type,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    int kPointerSize = 8;
    switch (target->getTarget())
    {
    case CodeGenTarget::HostCPPSource:
    case CodeGenTarget::HostHostCallable:
    case CodeGenTarget::HostExecutable:
        kPointerSize = (int)sizeof(void*);
        break;
    }

    switch (type->getOp())
    {

#define CASE(TYPE, SIZE, ALIGNMENT)                                 \
case kIROp_##TYPE##Type:                                        \
    *outSizeAndAlignment = IRSizeAndAlignment(SIZE, ALIGNMENT); \
    return SLANG_OK                                             \
    /* end */

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
        BASE(IntPtr, kPointerSize);
        BASE(UIntPtr, kPointerSize);
        BASE(Double, 8);

        // We are currently handling `bool` following the HLSL
        // precednet of storing it in 4 bytes.
        //
        // TODO: It would be good to try to make this follow
        // per-platform conventions, or at least to be able
        // to use a 1-byte encoding where available.
        //
        BASE(Bool, 4);

        // The Slang `void` type is treated as a zero-byte
        // type, so that it does not influence layout at all.
        //
        CASE(Void, 0, 1);

#undef CASE

#undef CASE

    case kIROp_StructType:
    {
        auto structType = cast<IRStructType>(type);
        IRSizeAndAlignment structLayout;
        IRIntegerValue offset = 0;
        for (auto field : structType->getFields())
        {
            IRSizeAndAlignment fieldTypeLayout;
            SLANG_RETURN_ON_FAIL(getSizeAndAlignment(target, rules, field->getFieldType(), &fieldTypeLayout));
            structLayout.size = align(offset, fieldTypeLayout.alignment);
            structLayout.alignment = std::max(structLayout.alignment, fieldTypeLayout.alignment);

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
                    builder.getIntValue(intType, (IRIntegerValue)rules->ruleName),
                    builder.getIntValue(intType, fieldOffset));
            }

            structLayout.size += fieldTypeLayout.size;
            offset = structLayout.size;
            if (as<IRMatrixType>(field->getFieldType()) || as<IRArrayTypeBase>(field->getFieldType()) || as<IRStructType>(field->getFieldType()))
            {
                offset = rules->adjustOffsetForNextAggregateMember(offset, fieldTypeLayout.alignment);
            }
        }
        *outSizeAndAlignment = rules->alignCompositeElement(structLayout);
        return SLANG_OK;
    }
    break;

    case kIROp_ArrayType:
    {
        auto arrayType = cast<IRArrayType>(type);

        return _calcArraySizeAndAlignment(
            target,
            rules ,
            arrayType->getElementType(),
            arrayType->getElementCount(),
            outSizeAndAlignment);
    }
    break;

    case kIROp_VectorType:
    {
        auto vecType = cast<IRVectorType>(type);
        IRSizeAndAlignment elementTypeLayout;
        getSizeAndAlignment(target, rules, vecType->getElementType(), &elementTypeLayout);
        *outSizeAndAlignment = rules->getVectorSizeAndAlignment(elementTypeLayout, getIntegerValueFromInst(vecType->getElementCount()));
        return SLANG_OK;
    }
    break;
    case kIROp_AnyValueType:
    {
        auto anyValType = cast<IRAnyValueType>(type);
        outSizeAndAlignment->size = getIntVal(anyValType->getSize());
        outSizeAndAlignment->alignment = 4;
        *outSizeAndAlignment = rules->alignCompositeElement(*outSizeAndAlignment);
        return SLANG_OK;
    }
    break;
    case kIROp_TupleType:
    {
        auto tupleType = cast<IRTupleType>(type);
        IRSizeAndAlignment resultLayout;
        for (UInt i = 0; i < tupleType->getOperandCount(); i++)
        {
            auto elementType = tupleType->getOperand(i);
            IRSizeAndAlignment fieldTypeLayout;
            SLANG_RETURN_ON_FAIL(getSizeAndAlignment(target, rules, (IRType*)elementType, &fieldTypeLayout));
            resultLayout.size = align(resultLayout.size, fieldTypeLayout.alignment);
            resultLayout.alignment = std::max(resultLayout.alignment, fieldTypeLayout.alignment);
        }
        *outSizeAndAlignment = rules->alignCompositeElement(resultLayout);
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
    case kIROp_InterfaceType:
    {
        auto interfaceType = cast<IRInterfaceType>(type);
        auto size = SharedGenericsLoweringContext::getInterfaceAnyValueSize(interfaceType, interfaceType->sourceLoc);
        size += kRTTIHeaderSize;
        size = align(size, 4);
        IRSizeAndAlignment resultLayout;
        resultLayout.size = size;
        resultLayout.alignment = 4;
        *outSizeAndAlignment = rules->alignCompositeElement(resultLayout);
        return SLANG_OK;
    }
    break;
    case kIROp_MatrixType:
    {
        auto matType = cast<IRMatrixType>(type);
        IRBuilder builder(type->getModule());
        if (getIntegerValueFromInst(matType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
        {
            auto colVector = builder.getVectorType(matType->getElementType(), matType->getRowCount());
            return _calcArraySizeAndAlignment(
                target,
                rules,
                colVector,
                matType->getColumnCount(),
                outSizeAndAlignment);
        }
        else
        {
            auto rowVector = builder.getVectorType(matType->getElementType(), matType->getColumnCount());
            return _calcArraySizeAndAlignment(
                target,
                rules,
                rowVector,
                matType->getRowCount(),
                outSizeAndAlignment);
        }
    }
    break;
    case kIROp_OutType:
    case kIROp_InOutType:
    case kIROp_RefType:
    case kIROp_ConstRefType:
    case kIROp_RawPointerType:
    case kIROp_PtrType:
    case kIROp_NativePtrType:
    case kIROp_ComPtrType:
    case kIROp_NativeStringType:
    case kIROp_HLSLConstBufferPointerType:
    {
        *outSizeAndAlignment = IRSizeAndAlignment(kPointerSize, kPointerSize);
        return SLANG_OK;
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

IRSizeAndAlignmentDecoration* findSizeAndAlignmentDecorationForLayout(IRType* type, IRTypeLayoutRuleName layoutName)
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

Result getSizeAndAlignment(TargetRequest* target, IRTypeLayoutRules* rules, IRType* type, IRSizeAndAlignment* outSizeAndAlignment)
{
    if (auto decor = findSizeAndAlignmentDecorationForLayout(type, rules->ruleName))
    {
        *outSizeAndAlignment = IRSizeAndAlignment(decor->getSize(), (int)decor->getAlignment());
        return SLANG_OK;
    }

    IRSizeAndAlignment sizeAndAlignment;
    SLANG_RETURN_ON_FAIL(_calcSizeAndAlignment(target, rules, type, &sizeAndAlignment));

    if (auto module = type->getModule())
    {
        IRBuilder builder(module);

        auto intType = builder.getIntType();
        builder.addDecoration(
            type,
            kIROp_SizeAndAlignmentDecoration,
            builder.getIntValue(intType, (IRIntegerValue)rules->ruleName),
            builder.getIntValue(intType, sizeAndAlignment.size),
            builder.getIntValue(intType, sizeAndAlignment.alignment));
    }

    *outSizeAndAlignment = sizeAndAlignment;
    return SLANG_OK;
}
IROffsetDecoration* findOffsetDecorationForLayout(IRStructField* field, IRTypeLayoutRuleName layoutName)
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

Result getOffset(TargetRequest* target, IRTypeLayoutRules* rules, IRStructField* field, IRIntegerValue* outOffset)
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
    SLANG_RETURN_ON_FAIL(getSizeAndAlignment(target, rules, structType, &structTypeLayout));

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
    NaturalLayoutRules()
    {
        ruleName = IRTypeLayoutRuleName::Natural;
    }
    virtual IRIntegerValue adjustOffsetForNextAggregateMember(IRIntegerValue currentSize, IRIntegerValue lastElementAlignment)
    {
        SLANG_UNUSED(lastElementAlignment);
        return currentSize;
    }
    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        return elementSize;
    }
    virtual IRSizeAndAlignment getVectorSizeAndAlignment(IRSizeAndAlignment element, IRIntegerValue count)
    {
        return IRSizeAndAlignment(element.size * count, element.alignment);
    }
};

struct Std430LayoutRules : IRTypeLayoutRules
{
    Std430LayoutRules()
    {
        ruleName = IRTypeLayoutRuleName::Std430;
    }

    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        return elementSize;
    }
    virtual IRIntegerValue adjustOffsetForNextAggregateMember(IRIntegerValue currentSize, IRIntegerValue lastElementAlignment)
    {
        return align(currentSize, (int)lastElementAlignment);
    }

    virtual IRSizeAndAlignment getVectorSizeAndAlignment(IRSizeAndAlignment element, IRIntegerValue count)
    {
        IRIntegerValue countForAlignment = count;
        if (count == 3)
            countForAlignment = 4;
        return IRSizeAndAlignment((int)(element.size * count), (int)(element.size * countForAlignment));
    }
};

struct Std140LayoutRules : IRTypeLayoutRules
{
    Std140LayoutRules()
    {
        ruleName = IRTypeLayoutRuleName::Std140;
    }

    virtual IRIntegerValue adjustOffsetForNextAggregateMember(IRIntegerValue currentSize, IRIntegerValue lastElementAlignment)
    {
        return align(currentSize, (int)lastElementAlignment);
    }
    virtual IRSizeAndAlignment alignCompositeElement(IRSizeAndAlignment elementSize)
    {
        elementSize.alignment = (int)align(elementSize.alignment, 16);
        elementSize.size = align(elementSize.size, elementSize.alignment);
        return elementSize;
    }
    virtual IRSizeAndAlignment getVectorSizeAndAlignment(IRSizeAndAlignment element, IRIntegerValue count)
    {
        if (count == 3)
            count = 4;
        return IRSizeAndAlignment((int)(element.size * count), (int)(element.size * count));
    }
};

Result getNaturalSizeAndAlignment(TargetRequest* target, IRType* type, IRSizeAndAlignment* outSizeAndAlignment)
{
    return getSizeAndAlignment(target, IRTypeLayoutRules::getNatural(), type, outSizeAndAlignment);

}

Result getNaturalOffset(TargetRequest* target, IRStructField* field, IRIntegerValue* outOffset)
{
    return getOffset(target, IRTypeLayoutRules::getNatural(), field, outOffset);
}


//////////////////////////
// Std430 Layout
//////////////////////////

Result getStd430SizeAndAlignment(TargetRequest* target, IRType* type, IRSizeAndAlignment* outSizeAndAlignment)
{
    return getSizeAndAlignment(target, IRTypeLayoutRules::getStd430(), type, outSizeAndAlignment);
}

Result getStd430Offset(TargetRequest* target, IRStructField* field, IRIntegerValue* outOffset)
{
    return getOffset(target, IRTypeLayoutRules::getStd430(), field, outOffset);
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
IRTypeLayoutRules* IRTypeLayoutRules::get(IRTypeLayoutRuleName name)
{
    switch (name)
    {
        case IRTypeLayoutRuleName::Std430: return getStd430();
        case IRTypeLayoutRuleName::Std140: return getStd140();
        case IRTypeLayoutRuleName::Natural: return getNatural();
        default: return nullptr;
    }
}

}
