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

static Result _calcNaturalArraySizeAndAlignment(
    TargetRequest*      target,
    IRType*             elementType,
    IRInst*             elementCountInst,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    auto elementCountLit = as<IRIntLit>(elementCountInst);
    if(!elementCountLit)
        return SLANG_FAIL;
    auto elementCount = elementCountLit->getValue();

    if( elementCount == 0 )
    {
        *outSizeAndAlignment = IRSizeAndAlignment(0, 1);
        return SLANG_OK;
    }

    IRSizeAndAlignment elementTypeLayout;
    SLANG_RETURN_ON_FAIL(getNaturalSizeAndAlignment(target, elementType, &elementTypeLayout));

    auto elementStride = elementTypeLayout.getStride();

    *outSizeAndAlignment = IRSizeAndAlignment(
        elementStride * (elementCount - 1) + elementTypeLayout.size,
        elementTypeLayout.alignment);
    return SLANG_OK;
}

IRIntegerValue getIntegerValueFromInst(IRInst* inst)
{
    SLANG_ASSERT(inst->getOp() == kIROp_IntLit);
    return as<IRIntLit>(inst)->value.intVal;
}

static Result _calcNaturalSizeAndAlignment(
    TargetRequest*      target, 
    IRType*             type,
    IRSizeAndAlignment* outSizeAndAlignment)
{
    switch( type->getOp() )
    {

#define CASE(TYPE, SIZE, ALIGNMENT)                                 \
    case kIROp_##TYPE##Type:                                        \
        *outSizeAndAlignment = IRSizeAndAlignment(SIZE, ALIGNMENT); \
        return SLANG_OK                                             \
        /* end */

    // Most base types are "naturally aligned" (meaning alignment and size are the same)
#define BASE(TYPE, SIZE) CASE(TYPE, SIZE, SIZE)

    BASE(Int8,      1);
    BASE(UInt8,     1);

    BASE(Int16,     2);
    BASE(UInt16,    2);
    BASE(Half,      2);

    BASE(Int,       4);
    BASE(UInt,      4);
    BASE(Float,     4);

    BASE(Int64,     8);
    BASE(UInt64,    8);
    BASE(Double,    8);

    // We are currently handling `bool` following the HLSL
    // precednet of storing it in 4 bytes.
    //
    // TODO: It would be good to try to make this follow
    // per-platform conventions, or at least to be able
    // to use a 1-byte encoding where available.
    //
    BASE(Bool,      4);

    // The Slang `void` type is treated as a zero-byte
    // type, so that it does not influence layout at all.
    //
    CASE(Void,      0,  1);

#undef CASE

#undef CASE

    case kIROp_StructType:
        {
            auto structType = cast<IRStructType>(type);
            IRSizeAndAlignment structLayout;
            for( auto field : structType->getFields() )
            {
                IRSizeAndAlignment fieldTypeLayout;
                SLANG_RETURN_ON_FAIL(getNaturalSizeAndAlignment(target, field->getFieldType(), &fieldTypeLayout));

                structLayout.size = align(structLayout.size, fieldTypeLayout.alignment);
                structLayout.alignment = std::max(structLayout.alignment, fieldTypeLayout.alignment);

                IRIntegerValue fieldOffset = structLayout.size;
                if( auto module = type->getModule() )
                {
                    // If we are in a situation where attaching new
                    // decorations is possible, then we want to
                    // cache the field offset on the IR field
                    // instruction.
                    //
                    SharedIRBuilder sharedBuilder(module);

                    IRBuilder builder(sharedBuilder);

                    auto intType = builder.getIntType();
                    builder.addDecoration(
                        field,
                        kIROp_NaturalOffsetDecoration,
                        builder.getIntValue(intType, fieldOffset));
                }

                structLayout.size += fieldTypeLayout.size;
            }
            *outSizeAndAlignment = structLayout;
            return SLANG_OK;
        }
        break;

    case kIROp_ArrayType:
        {
            auto arrayType = cast<IRArrayType>(type);

            return _calcNaturalArraySizeAndAlignment(
                target,
                arrayType->getElementType(),
                arrayType->getElementCount(),
                outSizeAndAlignment);
        }
        break;

    case kIROp_VectorType:
        {
            auto vecType = cast<IRVectorType>(type);

            return _calcNaturalArraySizeAndAlignment(
                target,
                vecType->getElementType(),
                vecType->getElementCount(),
                outSizeAndAlignment);
        }
        break;
    case kIROp_AnyValueType:
        {
            auto anyValType = cast<IRAnyValueType>(type);
            outSizeAndAlignment->size = getIntVal(anyValType->getSize());
            outSizeAndAlignment->alignment = 4;
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
                SLANG_RETURN_ON_FAIL(getNaturalSizeAndAlignment(target, (IRType*)elementType, &fieldTypeLayout));
                resultLayout.size = align(resultLayout.size, fieldTypeLayout.alignment);
                resultLayout.alignment = std::max(resultLayout.alignment, fieldTypeLayout.alignment);
            }
            *outSizeAndAlignment = resultLayout;
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
            *outSizeAndAlignment = resultLayout;
            return SLANG_OK;
        }
        break;
    case kIROp_MatrixType:
        {
            auto matType = cast<IRMatrixType>(type);
            auto rowCount = getIntegerValueFromInst(matType->getRowCount());
            auto colCount = getIntegerValueFromInst(matType->getColumnCount());
            SharedIRBuilder sharedBuilder(type->getModule());
            IRBuilder builder(sharedBuilder);

            return _calcNaturalArraySizeAndAlignment(
                target, matType->getElementType(),
                builder.getIntValue(builder.getUIntType(), rowCount * colCount),
                outSizeAndAlignment);
        }
        break;
    case kIROp_OutType:
    case kIROp_InOutType:
    case kIROp_RefType:
    case kIROp_RawPointerType:
    case kIROp_PtrType:
    case kIROp_NativePtrType:
    case kIROp_ComPtrType:
    case kIROp_NativeStringType:
        {
            *outSizeAndAlignment = IRSizeAndAlignment(sizeof(void*), sizeof(void*));
            return SLANG_OK;
        }
        break;
    default:
        break;
    }

    if( areResourceTypesBindlessOnTarget(target) )
    {
        // TODO: need this to be based on target, instead of hard-coded
        int pointerSize = sizeof(void*);

        if(as<IRTextureType>(type) )
        {
            *outSizeAndAlignment = IRSizeAndAlignment(pointerSize, pointerSize);
            return SLANG_OK;
        }
        else if(as<IRSamplerStateTypeBase>(type) )
        {
            *outSizeAndAlignment = IRSizeAndAlignment(pointerSize, pointerSize);
            return SLANG_OK;
        }
        // TODO: the remaining cases for "bindless" resources on CPU/CUDA targets
    }

    return SLANG_FAIL;
}

Result getNaturalSizeAndAlignment(TargetRequest* target, IRType* type, IRSizeAndAlignment* outSizeAndAlignment)
{
    if( auto decor = type->findDecoration<IRNaturalSizeAndAlignmentDecoration>() )
    {
        *outSizeAndAlignment = IRSizeAndAlignment(decor->getSize(), (int)decor->getAlignment());
        return SLANG_OK;
    }

    IRSizeAndAlignment sizeAndAlignment;
    SLANG_RETURN_ON_FAIL(_calcNaturalSizeAndAlignment(target, type, &sizeAndAlignment));

    if( auto module = type->getModule() )
    {
        SharedIRBuilder sharedBuilder(module);

        IRBuilder builder(sharedBuilder);

        auto intType = builder.getIntType();
        builder.addDecoration(
            type,
            kIROp_NaturalSizeAndAlignmentDecoration,
            builder.getIntValue(intType, sizeAndAlignment.size),
            builder.getIntValue(intType, sizeAndAlignment.alignment));
    }

    *outSizeAndAlignment = sizeAndAlignment;
    return SLANG_OK;
}


Result getNaturalOffset(TargetRequest* target, IRStructField* field, IRIntegerValue* outOffset)
{
    if( auto decor = field->findDecoration<IRNaturalOffsetDecoration>() )
    {
        *outOffset = decor->getOffset();
        return SLANG_OK;
    }

    // Offsets are computed as part of layout out types,
    // so we expect that layout of the "parent" type
    // of the field should add an offset to it if
    // possible.

    auto structType = as<IRStructType>(field->getParent());
    if(!structType)
        return SLANG_FAIL;

    IRSizeAndAlignment structTypeLayout;
    SLANG_RETURN_ON_FAIL(getNaturalSizeAndAlignment(target, structType, &structTypeLayout));

    if( auto decor = field->findDecoration<IRNaturalOffsetDecoration>() )
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

}
