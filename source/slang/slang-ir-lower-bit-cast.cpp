#include "slang-ir-lower-bit-cast.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-extract-value-from-type.h"
#include "slang-ir-layout.h"

namespace Slang
{

struct BitCastLoweringContext
{
    TargetRequest* targetReq;
    IRModule* module;
    OrderedHashSet<IRInst*> workList;

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workList.contains(inst))
            return;

        workList.add(inst);
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_BitCast:
            processBitCast(inst);
            break;
        default:
            break;
        }
    }

    void processModule()
    {
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }
    }


    // Extract an object of `type` from `offset` in `src`.
    IRInst* readObject(IRBuilder& builder, IRInst* src, IRType* type, uint32_t offset)
    {
        switch (type->getOp())
        {
        case kIROp_StructType:
            {
                auto structType = as<IRStructType>(type);
                List<IRInst*> fieldValues;
                for (auto field : structType->getFields())
                {
                    IRIntegerValue fieldOffset = 0;
                    SLANG_RELEASE_ASSERT(
                        getNaturalOffset(targetReq, field, &fieldOffset) == SLANG_OK);
                    auto fieldType = field->getFieldType();
                    auto fieldValue =
                        readObject(builder, src, fieldType, (uint32_t)(fieldOffset + offset));
                    fieldValues.add(fieldValue); 
                }
                return builder.emitMakeStruct(structType, fieldValues);
            }
            break;
        case kIROp_ArrayType:
            {
                auto arrayType = as<IRArrayType>(type);
                auto arrayCount = as<IRIntLit>(arrayType->getElementCount());
                SLANG_RELEASE_ASSERT(arrayCount && "bit_cast: array size must be fixed.");
                List<IRInst*> elements;
                IRSizeAndAlignment elementLayout;
                SLANG_RELEASE_ASSERT(
                    getNaturalSizeAndAlignment(
                        targetReq, arrayType->getElementType(), &elementLayout) == SLANG_OK);
                for (IRIntegerValue i = 0; i < arrayCount->value.intVal; i++)
                {
                    elements.add(readObject(
                        builder,
                        src,
                        arrayType->getElementType(),
                        (uint32_t)(offset + elementLayout.getStride() * i)));
                }
                return builder.emitMakeArray(arrayType, (UInt)arrayCount->value.intVal, elements.getBuffer());
            }
            break;
        case kIROp_VectorType:
            {
                auto vectorType = as<IRVectorType>(type);
                auto elementCount = as<IRIntLit>(vectorType->getElementCount());
                SLANG_RELEASE_ASSERT(elementCount && "bit_cast: vector size must be int literal.");
                List<IRInst*> elements;
                IRSizeAndAlignment elementLayout;
                SLANG_RELEASE_ASSERT(
                    getNaturalSizeAndAlignment(
                        targetReq, vectorType->getElementType(), &elementLayout) == SLANG_OK);
                for (IRIntegerValue i = 0; i < elementCount->value.intVal; i++)
                {
                    elements.add(readObject(
                        builder,
                        src,
                        vectorType->getElementType(),
                        (uint32_t)(offset + elementLayout.getStride() * i)));
                }
                return builder.emitMakeVector(
                    vectorType, (UInt)elementCount->value.intVal, elements.getBuffer());
            }
            break;
        case kIROp_MatrixType:
            {
                // Assuming row-major order
                auto matrixType = as<IRMatrixType>(type);
                auto elementCount = as<IRIntLit>(matrixType->getRowCount());
                SLANG_RELEASE_ASSERT(
                    elementCount && "bit_cast: vector size must be int literal.");
                List<IRInst*> elements;
                auto elementType = builder.getVectorType(
                    matrixType->getElementType(), matrixType->getColumnCount());
                IRSizeAndAlignment elementLayout;
                SLANG_RELEASE_ASSERT(
                    getNaturalSizeAndAlignment(targetReq, elementType, &elementLayout) == SLANG_OK);
                for (IRIntegerValue i = 0; i < elementCount->value.intVal; i++)
                {
                    elements.add(readObject(
                        builder,
                        src,
                        elementType,
                        (uint32_t)(offset + elementLayout.getStride() * i)));
                }
                return builder.emitMakeMatrix(
                    matrixType, (UInt)elementCount->value.intVal, elements.getBuffer());
            }
            break;
        case kIROp_HalfType:
        case kIROp_Int16Type:
        case kIROp_UInt16Type:
            {
                auto object = extractValueAtOffset(builder, targetReq, src, offset, 2);
                return builder.emitBitCast(type, object);
            }
            break;
        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_FloatType:
        case kIROp_BoolType:
#if SLANG_PTR_IS_32
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
#endif
            {
                auto object = extractValueAtOffset(builder, targetReq, src, offset, 4);
                return builder.emitBitCast(type, object);
            }
            break;
        case kIROp_DoubleType:
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
#if SLANG_PTR_IS_64
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
#endif
        case kIROp_RawPointerType:
            {
                auto low = extractValueAtOffset(builder, targetReq, src, offset, 4);
                auto high = extractValueAtOffset(builder, targetReq, src, offset + 4, 4);
                auto combined = builder.emitAdd(builder.getUInt64Type(),
                    low,
                    builder.emitShl(
                        builder.getUInt64Type(),
                        high,
                        builder.getIntValue(builder.getUIntType(), 32)));
                if (type->getOp() == kIROp_UInt64Type)
                    return combined;
                return builder.emitBitCast(type, combined);
            }
            break;
        case kIROp_UInt8Type:
        case kIROp_Int8Type:
            {
                auto object = extractValueAtOffset(builder, targetReq, src, offset, 1);
                return builder.emitBitCast(type, object);
            }
            break;
        default:
            {
                SLANG_UNEXPECTED("Unable to generate bit_cast code for the given type");
            }
            break;
        }
    }

    void processBitCast(IRInst* inst)
    {
        auto operand = inst->getOperand(0);
        auto fromType = operand->getDataType();
        auto toType = inst->getDataType();
        if (as<IRBasicType>(fromType) != nullptr && as<IRBasicType>(toType) != nullptr)
        {
            // Both fromType and toType are basic types, no processing needed.
            return;
        }
        // Ignore cases we cannot handle yet.
        if (as<IRPtrType>(fromType) || as<IRPtrType>(toType))
        {
            return;
        }
        if (as<IRRawPointerType>(fromType) || as<IRRawPointerType>(toType))
        {
            return;
        }
        if (as<IRResourceTypeBase>(fromType) || as<IRResourceTypeBase>(toType))
        {
            return;
        }
        if (as<IRPointerLikeType>(fromType) || as<IRPointerLikeType>(toType))
        {
            return;
        }
        if (as<IRSamplerStateTypeBase>(fromType) || as<IRSamplerStateTypeBase>(toType))
        {
            return;
        }
        // Enumerate all fields in to-type and obtain its value from operand object.
        IRBuilder builder(module);
        builder.setInsertBefore(inst);
        auto finalObject = readObject(builder, operand, toType, 0);
        inst->replaceUsesWith(finalObject);
        inst->removeAndDeallocate();
    }
};

void lowerBitCast(TargetRequest* targetReq, IRModule* module)
{
    BitCastLoweringContext context;
    context.module = module;
    context.targetReq = targetReq;
    context.processModule();
}

}
