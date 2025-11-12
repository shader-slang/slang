#include "slang-ir-lower-bit-cast.h"

#include "slang-capability.h"
#include "slang-ir-extract-value-from-type.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct BitCastLoweringContext
{
    TargetProgram* targetProgram;
    IRModule* module;
    OrderedHashSet<IRInst*> workList;
    DiagnosticSink* sink;

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
                        getNaturalOffset(targetProgram->getOptionSet(), field, &fieldOffset) ==
                        SLANG_OK);
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
                        targetProgram->getOptionSet(),
                        arrayType->getElementType(),
                        &elementLayout) == SLANG_OK);
                for (IRIntegerValue i = 0; i < arrayCount->value.intVal; i++)
                {
                    elements.add(readObject(
                        builder,
                        src,
                        arrayType->getElementType(),
                        (uint32_t)(offset + elementLayout.getStride() * i)));
                }
                return builder.emitMakeArray(
                    arrayType,
                    (UInt)arrayCount->value.intVal,
                    elements.getBuffer());
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
                        targetProgram->getOptionSet(),
                        vectorType->getElementType(),
                        &elementLayout) == SLANG_OK);
                for (IRIntegerValue i = 0; i < elementCount->value.intVal; i++)
                {
                    elements.add(readObject(
                        builder,
                        src,
                        vectorType->getElementType(),
                        (uint32_t)(offset + elementLayout.getStride() * i)));
                }
                return builder.emitMakeVector(
                    vectorType,
                    (UInt)elementCount->value.intVal,
                    elements.getBuffer());
            }
            break;
        case kIROp_MatrixType:
            {
                // Assuming row-major order
                auto matrixType = as<IRMatrixType>(type);
                auto elementCount = as<IRIntLit>(matrixType->getRowCount());
                SLANG_RELEASE_ASSERT(elementCount && "bit_cast: vector size must be int literal.");
                List<IRInst*> elements;
                auto elementType = builder.getVectorType(
                    matrixType->getElementType(),
                    matrixType->getColumnCount());
                IRSizeAndAlignment elementLayout;
                SLANG_RELEASE_ASSERT(
                    getNaturalSizeAndAlignment(
                        targetProgram->getOptionSet(),
                        elementType,
                        &elementLayout) == SLANG_OK);
                for (IRIntegerValue i = 0; i < elementCount->value.intVal; i++)
                {
                    elements.add(readObject(
                        builder,
                        src,
                        elementType,
                        (uint32_t)(offset + elementLayout.getStride() * i)));
                }
                return builder.emitMakeMatrix(
                    matrixType,
                    (UInt)elementCount->value.intVal,
                    elements.getBuffer());
            }
            break;
        case kIROp_HalfType:
        case kIROp_Int16Type:
        case kIROp_UInt16Type:
            {
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, 2);
                object = builder.emitCast(builder.getUInt16Type(), object);
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
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, 4);
                object = builder.emitCast(builder.getUIntType(), object);
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
        case kIROp_PtrType:
        case kIROp_FuncType:
            {
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, 8);
                object = builder.emitCast(builder.getUInt64Type(), object);
                return builder.emitBitCast(type, object);
            }
            break;
        case kIROp_UInt8Type:
        case kIROp_Int8Type:
            {
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, 1);
                object = builder.emitCast(builder.getUInt8Type(), object);
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

        IRSizeAndAlignment toTypeSize;
        getNaturalSizeAndAlignment(targetProgram->getOptionSet(), toType, &toTypeSize);
        IRSizeAndAlignment fromTypeSize;
        getNaturalSizeAndAlignment(targetProgram->getOptionSet(), fromType, &fromTypeSize);

        // Check if the target is directly emitted SPIRV and if the target is SPIRV 1.5 or later
        bool isDirectSpirv = false;
        bool isSpirv15OrLater = false;
        if (auto targetReq = targetProgram->getTargetReq())
        {
            auto target = targetReq->getTarget();
            isDirectSpirv =
                (target == CodeGenTarget::SPIRV || target == CodeGenTarget::SPIRVAssembly) &&
                targetProgram->shouldEmitSPIRVDirectly();
            isSpirv15OrLater = targetReq->getTargetCaps().implies(CapabilityAtom::_spirv_1_5);
        }

        auto fromBasicType = as<IRBasicType>(fromType);
        auto toBasicType = as<IRBasicType>(toType);
        if (fromBasicType && toBasicType)
        {
            if (fromTypeSize.size != toTypeSize.size)
                sink->diagnose(
                    inst->sourceLoc,
                    Diagnostics::notEqualBitCastSize,
                    fromType,
                    fromTypeSize.size,
                    toType,
                    toTypeSize.size);
            // Both fromType and toType are basic types, no processing needed.
            return;
        }

        // Skip lowering bitcasts that can be directly handled by SPIR-V OpBitcast
        // The SPIR-V spec requires that OpBitcast's operand and result have the same size and
        // different types
        if (isDirectSpirv && fromTypeSize.size == toTypeSize.size)
        {
            auto fromPtrType = as<IRPtrTypeBase>(fromType);
            auto toPtrType = as<IRPtrTypeBase>(toType);

            // OpBitcast can handle pointer <-> pointer bitcasts directly,
            // but both pointers must have same storage class and different types.
            if (fromPtrType && toPtrType &&
                fromPtrType->getAddressSpace() == toPtrType->getAddressSpace() &&
                !isTypeEqual(fromPtrType, toPtrType))
            {
                auto fromValueType = fromPtrType->getValueType();
                auto toValueType = toPtrType->getValueType();

                // Unwrap atomic pointers, as they are emitted as the same type as non-atomic
                // pointers in SPIR-V, but have different types from non-atomic pointers in IR
                auto fromUnwrappedType = as<IRAtomicType>(fromValueType)
                                             ? as<IRAtomicType>(fromValueType)->getElementType()
                                             : fromValueType;
                auto toUnwrappedType = as<IRAtomicType>(toValueType)
                                           ? as<IRAtomicType>(toValueType)->getElementType()
                                           : toValueType;

                // If the unwrapped types are different, we can use OpBitcast directly
                if (!isTypeEqual(fromUnwrappedType, toUnwrappedType))
                    return;
            }

            // OpBitcast can handle pointer -> scalar integer bitcasts directly
            if (fromPtrType && toBasicType && isIntegralType(toType))
                return;

            // OpBitcast can handle scalar integer -> pointer bitcasts directly
            if (fromBasicType && toPtrType && isIntegralType(fromType))
                return;

            auto fromVectorType = as<IRVectorType>(fromType);
            auto toVectorType = as<IRVectorType>(toType);

            // OpBitcast can handle pointer -> integer vector bitcasts directly,
            // but those integers need to be 32-bit and SPIR-V 1.5+ is required
            if (fromPtrType && toVectorType && isSpirv15OrLater)
            {
                auto elementType = toVectorType->getElementType();
                if (isIntegralType(elementType))
                {
                    auto intInfo = getIntTypeInfo(elementType);
                    if (intInfo.width == 32)
                        return;
                }
            }

            // OpBitcast can handle integer vector -> pointer bitcasts directly,
            // but those integers need to be 32-bit and SPIR-V 1.5+ is required
            if (toPtrType && fromVectorType && isSpirv15OrLater)
            {
                auto elementType = fromVectorType->getElementType();
                if (isIntegralType(elementType))
                {
                    auto intInfo = getIntTypeInfo(elementType);
                    if (intInfo.width == 32)
                        return;
                }
            }

            // OpBitcast can handle vector <-> scalar bitcasts directly
            // OpBitcast can also handle vector <-> vector bitcasts directly,
            // but only if the larger element count is an integer multiple of the smaller element
            // count, and if the types are different (SPIR-V spec requires different operand/result
            // types)
            auto fromElementCount = getIRVectorElementSize(fromType);
            auto toElementCount = getIRVectorElementSize(toType);
            if ((fromVectorType || fromBasicType) && (toVectorType || toBasicType) &&
                (fromElementCount % toElementCount == 0 ||
                 toElementCount % fromElementCount == 0) &&
                !isTypeEqual(fromType, toType))
                return;
        }

        // Ignore cases we cannot handle yet.
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

        if (fromTypeSize.size != toTypeSize.size)
            sink->diagnose(
                inst->sourceLoc,
                Diagnostics::notEqualBitCastSize,
                fromType,
                fromTypeSize.size,
                toType,
                toTypeSize.size);

        // Enumerate all fields in to-type and obtain its value from operand object.
        IRBuilder builder(module);
        builder.setInsertBefore(inst);
        auto finalObject = readObject(builder, operand, toType, 0);
        inst->replaceUsesWith(finalObject);
        inst->removeAndDeallocate();
    }
};

void lowerBitCast(TargetProgram* targetProgram, IRModule* module, DiagnosticSink* sink)
{
    BitCastLoweringContext context;
    context.module = module;
    context.targetProgram = targetProgram;
    context.sink = sink;
    context.processModule();
}

} // namespace Slang
