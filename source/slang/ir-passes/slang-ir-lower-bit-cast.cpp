#include "slang-ir-lower-bit-cast.h"

#include "slang-ir-extract-value-from-type.h"
#include "slang/ir/slang-ir-insts.h"
#include "slang/ir/slang-ir-layout.h"
#include "slang/ir/slang-ir-util.h"
#include "slang/ir/slang-ir.h"
#include "slang/slang-capability.h"
#include "slang/slang-rich-diagnostics.h"
#include "slang/slang-target.h"

namespace Slang
{

// Is `halfType` a `vec2<f16>` and `otherType` a 32-bit scalar, or `halfType` a `vec4<f16>` and
// `otherType` a `vec2` of 32-bit values? WGSL's `bitcast` converts directly between those shapes
// in both directions, so the caller checks the bitcast's types in both orders.
static bool isWGSLHalfVectorBitCast(IRType* halfType, IRType* otherType)
{
    auto halfVectorType = as<IRVectorType>(halfType);
    if (!halfVectorType || halfVectorType->getElementType()->getOp() != kIROp_HalfType)
        return false;
    auto halfCount = as<IRIntLit>(halfVectorType->getElementCount());
    if (!halfCount)
        return false;

    auto is32BitScalar = [](IRType* type)
    {
        switch (type->getOp())
        {
        case kIROp_IntType:
        case kIROp_UIntType:
        case kIROp_FloatType:
            return true;
        default:
            return false;
        }
    };

    if (halfCount->getValue() == 2)
        return is32BitScalar(otherType);
    if (halfCount->getValue() == 4)
    {
        auto otherVectorType = as<IRVectorType>(otherType);
        auto otherCount =
            otherVectorType ? as<IRIntLit>(otherVectorType->getElementCount()) : nullptr;
        return otherCount && otherCount->getValue() == 2 &&
               is32BitScalar(otherVectorType->getElementType());
    }
    return false;
}

// On the C++ (CPU) and CUDA targets a `DescriptorHandle<T>` is the native resource `T` itself, and
// every resource there is plain data: a pointer, a CUDA texture or surface object, or a pointer and
// an element count for a structured buffer. A bit cast to or from a handle is therefore a bitwise
// copy of those bytes. Other targets represent a handle differently (an index pair, or a type that
// cannot be built from an integer at all), so this pass cannot rebuild one from bits there.
static bool isDescriptorHandlePlainData(TargetRequest* targetReq)
{
    return isCPUTarget(targetReq) || isCUDATarget(targetReq);
}

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


    // The size of `handleType` when `readObject` can build it from one integer leaf, or 0 when it
    // cannot. That is the case for a plain-data handle whose resource is 4 or 8 bytes, such as a
    // texture, a sampler or a CUDA texture object; a CPU structured buffer handle is 16 bytes.
    uint32_t getDescriptorHandleLeafSize(IRDescriptorHandleType* handleType)
    {
        auto targetReq = targetProgram->getTargetReq();
        if (!isDescriptorHandlePlainData(targetReq))
            return 0;
        IRSizeAndAlignment layout;
        if (SLANG_FAILED(getNaturalSizeAndAlignment(targetReq, handleType, &layout)))
            return 0;
        if (layout.size != 4 && layout.size != 8)
            return 0;
        return (uint32_t)layout.size;
    }

    // Find a `DescriptorHandle<T>` in `type`, either `type` itself or one nested in its struct
    // fields or array elements, for which `isUnsupported` returns true.
    template<typename IsUnsupported>
    IRDescriptorHandleType* findDescriptorHandle(IRType* type, const IsUnsupported& isUnsupported)
    {
        switch (type->getOp())
        {
        case kIROp_DescriptorHandleType:
            {
                auto handleType = as<IRDescriptorHandleType>(type);
                return isUnsupported(handleType) ? handleType : nullptr;
            }
        case kIROp_StructType:
            for (auto field : as<IRStructType>(type)->getFields())
            {
                if (auto handleType = findDescriptorHandle(field->getFieldType(), isUnsupported))
                    return handleType;
            }
            return nullptr;
        case kIROp_ArrayType:
            return findDescriptorHandle(as<IRArrayType>(type)->getElementType(), isUnsupported);
        default:
            return nullptr;
        }
    }

    // Find a `DescriptorHandle<T>` in `type` that `readObject` cannot build from bits on this
    // target.
    IRDescriptorHandleType* findUnbuildableDescriptorHandle(IRType* type)
    {
        return findDescriptorHandle(
            type,
            [&](IRDescriptorHandleType* handleType)
            { return getDescriptorHandleLeafSize(handleType) == 0; });
    }

    // Find a `DescriptorHandle<T>` in `type` that `extractValueAtOffset` cannot read. It reads a
    // leaf of 1, 2, 4 or 8 bytes, and a handle is 4 or 8 bytes where it is readable, so a handle
    // of another size, such as a 16-byte C++ or CUDA structured-buffer handle, is not.
    IRDescriptorHandleType* findUnreadableDescriptorHandle(IRType* type)
    {
        return findDescriptorHandle(
            type,
            [&](IRDescriptorHandleType* handleType)
            {
                IRSizeAndAlignment layout;
                if (SLANG_FAILED(getNaturalSizeAndAlignment(
                        targetProgram->getTargetReq(),
                        handleType,
                        &layout)))
                    return true;
                return layout.size != 4 && layout.size != 8;
            });
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
                        getNaturalOffset(targetProgram->getTargetReq(), field, &fieldOffset) ==
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
                        targetProgram->getTargetReq(),
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
                        targetProgram->getTargetReq(),
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
                        targetProgram->getTargetReq(),
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
        case kIROp_BFloat16Type:
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
            {
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, 4);
                object = builder.emitCast(builder.getUIntType(), object);
                return builder.emitBitCast(type, object);
            }
            break;
        case kIROp_DoubleType:
        case kIROp_Int64Type:
        case kIROp_UInt64Type:
            {
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, 8);
                object = builder.emitCast(builder.getUInt64Type(), object);
                return builder.emitBitCast(type, object);
            }
            break;
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
        case kIROp_RawPointerType:
        case kIROp_PtrType:
        case kIROp_FuncType:
            {
                IRInst* object;
                auto ptrSize = getPointerSize(targetProgram->getTargetReq());
                object =
                    extractValueAtOffset(builder, targetProgram, src, offset, uint32_t(ptrSize));
                object = builder.emitCast(
                    ptrSize == sizeof(uint64_t) ? (IRType*)builder.getUInt64Type()
                                                : (IRType*)builder.getUIntType(),
                    object);
                return builder.emitBitCast(type, object);
            }
            break;
        case kIROp_UInt8Type:
        case kIROp_Int8Type:
        case kIROp_FloatE4M3Type:
        case kIROp_FloatE5M2Type:
            {
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, 1);
                object = builder.emitCast(builder.getUInt8Type(), object);
                return builder.emitBitCast(type, object);
            }
            break;
        case kIROp_DescriptorHandleType:
            {
                // `findUnbuildableDescriptorHandle` has rejected every handle this case cannot
                // build, so the handle is plain data and has the size of a 4- or 8-byte leaf.
                auto objectSize = getDescriptorHandleLeafSize(as<IRDescriptorHandleType>(type));
                SLANG_RELEASE_ASSERT(objectSize != 0);
                auto object = extractValueAtOffset(builder, targetProgram, src, offset, objectSize);
                object = builder.emitCast(
                    objectSize == sizeof(uint64_t) ? (IRType*)builder.getUInt64Type()
                                                   : (IRType*)builder.getUIntType(),
                    object);
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
        getNaturalSizeAndAlignment(targetProgram->getTargetReq(), toType, &toTypeSize);
        IRSizeAndAlignment fromTypeSize;
        getNaturalSizeAndAlignment(targetProgram->getTargetReq(), fromType, &fromTypeSize);

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
            {
                sink->diagnose(Diagnostics::NotEqualBitCastSize{
                    .fromType = fromType,
                    .fromSize = fromTypeSize.size,
                    .toType = toType,
                    .toSize = toTypeSize.size,
                    .location = inst->sourceLoc,
                });
            }
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
                    auto intInfo = getIntTypeInfo(targetProgram->getTargetReq(), elementType);
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
                    auto intInfo = getIntTypeInfo(targetProgram->getTargetReq(), elementType);
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

        // WGSL's `bitcast` converts directly between `vec2<f16>` and a 32-bit scalar, and
        // between `vec4<f16>` and a two-element vector of 32-bit values. Keep those as they are:
        // lowering them would go through 16-bit integers, which WGSL does not have.
        if (isWGPUTarget(targetProgram->getTargetReq()) &&
            (isWGSLHalfVectorBitCast(fromType, toType) ||
             isWGSLHalfVectorBitCast(toType, fromType)))
            return;

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
        if (as<IRHLSLStructuredBufferTypeBase>(fromType) ||
            as<IRHLSLStructuredBufferTypeBase>(toType))
        {
            return;
        }

        // A cast between a bare handle and other plain data of the same size is a bitwise copy,
        // which the C++ and CUDA emitters spell as `slang_bit_cast`. Leave it to them, as for the
        // handle casts that AnyValue marshalling creates for `reinterpret`.
        if ((as<IRDescriptorHandleType>(fromType) || as<IRDescriptorHandleType>(toType)) &&
            isDescriptorHandlePlainData(targetProgram->getTargetReq()))
        {
            if (fromTypeSize.size != toTypeSize.size)
            {
                sink->diagnose(Diagnostics::NotEqualBitCastSize{
                    .fromType = fromType,
                    .fromSize = fromTypeSize.size,
                    .toType = toType,
                    .toSize = toTypeSize.size,
                    .location = inst->sourceLoc,
                });
            }
            return;
        }

        // `readObject` builds the destination from integer leaves, which works for a handle only
        // where the handle is plain data of a leaf size. Report the others at the cast.
        if (auto handleType = findUnbuildableDescriptorHandle(toType))
        {
            sink->diagnose(Diagnostics::BitCastToUnsupportedDescriptorHandle{
                .fromType = fromType,
                .toType = toType,
                .handleType = handleType,
                .location = inst->sourceLoc,
            });
            return;
        }

        // `readObject` reads every byte of the source as integer leaves. A handle in the source
        // that is not a readable leaf would abort that read, so report it at the cast.
        if (auto handleType = findUnreadableDescriptorHandle(fromType))
        {
            sink->diagnose(Diagnostics::BitCastFromUnsupportedDescriptorHandle{
                .fromType = fromType,
                .toType = toType,
                .handleType = handleType,
                .location = inst->sourceLoc,
            });
            return;
        }

        if (fromTypeSize.size != toTypeSize.size)
        {
            sink->diagnose(Diagnostics::NotEqualBitCastSize{
                .fromType = fromType,
                .fromSize = fromTypeSize.size,
                .toType = toType,
                .toSize = toTypeSize.size,
                .location = inst->sourceLoc,
            });
        }

        // Enumerate all fields in to-type and obtain its value from operand object.
        IRBuilder builder(module);
        builder.setInsertBefore(inst);
        auto finalObject = readObject(builder, operand, toType, 0);
        inst->replaceUsesWith(finalObject);
        inst->removeAndDeallocate();
    }
};

void lowerBitCast(IRModule* module, TargetProgram* targetProgram, DiagnosticSink* sink)
{
    BitCastLoweringContext context;
    context.module = module;
    context.targetProgram = targetProgram;
    context.sink = sink;
    context.processModule();
}

} // namespace Slang
