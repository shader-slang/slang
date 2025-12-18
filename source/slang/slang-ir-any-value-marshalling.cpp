#include "slang-ir-any-value-marshalling.h"

#include "../core/slang-math.h"
#include "slang-compiler-options.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-legalize-types.h"
#include "slang-target-program.h"
#include "slang-target.h"

namespace Slang
{

// This is a subpass of generics lowering IR transformation.
// This pass generates packing/unpacking functions for `AnyValue`s,
// and replaces all `IRPackAnyValue` and `IRUnpackAnyValue` with calls to these
// functions.
struct AnyValueMarshallingContext
{
    IRModule* module;

    // We will use a single work list of instructions that need
    // to be considered for lowering.
    //
    InstWorkList workList;
    InstHashSet workListSet;
    TargetProgram* targetProgram;

    AnyValueMarshallingContext(IRModule* module, TargetProgram* targetProgram)
        : module(module), targetProgram(targetProgram), workList(module), workListSet(module)
    {
    }

    void addToWorkList(IRInst* inst)
    {
        if (!inst)
            return;

        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    // Stores information about generated `AnyValue` struct types.
    struct AnyValueTypeInfo : RefObject
    {
        IRType* type;                 // The generated IR value for the `AnyValue<N>` struct type.
        List<IRStructKey*> fieldKeys; // `IRStructKey`s for the fields of the generated type.
    };

    Dictionary<IRIntegerValue, RefPtr<AnyValueTypeInfo>> generatedAnyValueTypes;

    struct MarshallingFunctionKey
    {
        IRType* originalType;
        IRIntegerValue anyValueSize;
        bool operator==(MarshallingFunctionKey other) const
        {
            return originalType == other.originalType && anyValueSize == other.anyValueSize;
        }
        HashCode getHashCode() const
        {
            return combineHash(Slang::getHashCode(originalType), Slang::getHashCode(anyValueSize));
        }
    };

    struct MarshallingFunctionSet
    {
        IRFunc* packFunc;
        IRFunc* unpackFunc;
    };

    // Stores the generated packing/unpacking functions for lookup.
    Dictionary<MarshallingFunctionKey, MarshallingFunctionSet> mapTypeMarshalingFunctions;

    AnyValueTypeInfo* ensureAnyValueType(IRAnyValueType* type)
    {
        auto size = getIntVal(type->getSize());
        if (auto typeInfo = generatedAnyValueTypes.tryGetValue(size))
            return typeInfo->Ptr();
        RefPtr<AnyValueTypeInfo> info = new AnyValueTypeInfo();
        IRBuilder builder(module);
        builder.setInsertBefore(type);
        auto structType = builder.createStructType();
        info->type = structType;
        StringBuilder nameSb;
        nameSb << "AnyValue" << size;
        builder.addExportDecoration(structType, nameSb.getUnownedSlice());
        auto fieldCount = (size + sizeof(uint32_t) - 1) / sizeof(uint32_t);
        for (decltype(fieldCount) i = 0; i < fieldCount; i++)
        {
            auto key = builder.createStructKey();
            nameSb.clear();
            nameSb << "field" << i;
            builder.addNameHintDecoration(key, nameSb.getUnownedSlice());
            nameSb << "_anyVal" << size;
            builder.addExportDecoration(key, nameSb.getUnownedSlice());
            builder.createStructField(structType, key, builder.getUIntType());
            info->fieldKeys.add(key);
        }
        generatedAnyValueTypes[size] = info;
        return info.Ptr();
    }

    struct TypeMarshallingContext
    {
        AnyValueTypeInfo* anyValInfo;
        uint32_t fieldOffset;
        uint32_t intraFieldOffset;
        IRType* uintPtrType;
        IRInst* anyValueVar;
        TargetProgram* targetProgram;

        // Defines what to do with basic typed data elements.
        virtual void marshalBasicType(
            IRBuilder* builder,
            IRType* dataType,
            IRInst* concreteTypedVar) = 0;
        // Defines what to do with resource handle elements.
        virtual void marshalResourceHandle(
            IRBuilder* builder,
            IRType* dataType,
            IRInst* concreteTypedVar) = 0;

        // Defines what to do with DescriptorHandle types.
        // For non-bindless targets, uses CastDescriptorHandleToUInt2.
        // For bindless targets, marshals based on actual byte size.
        virtual void marshalDescriptorHandle(
            IRBuilder* builder,
            IRType* dataType,
            IRInst* concreteTypedVar,
            bool isBindless,
            IRIntegerValue sizeInBytes) = 0;

        void ensureOffsetAt4ByteBoundary()
        {
            if (intraFieldOffset)
            {
                fieldOffset++;
                intraFieldOffset = 0;
            }
        }
        void ensureOffsetAt8ByteBoundary()
        {
            ensureOffsetAt4ByteBoundary();
            if ((fieldOffset & 1) != 0)
                fieldOffset++;
        }
        void ensureOffsetAt2ByteBoundary()
        {
            if (intraFieldOffset == 0)
                return;
            if (intraFieldOffset <= 2)
            {
                intraFieldOffset = 2;
                return;
            }
            fieldOffset++;
            intraFieldOffset = 0;
            return;
        }
        void advanceOffset(uint32_t bytes)
        {
            intraFieldOffset += bytes;
            fieldOffset += intraFieldOffset / 4;
            intraFieldOffset = intraFieldOffset % 4;
        }
    };

    void emitMarshallingCode(
        IRBuilder* builder,
        TypeMarshallingContext* context,
        IRInst* concreteTypedVar)
    {
        auto dataType = cast<IRPtrTypeBase>(concreteTypedVar->getDataType())->getValueType();
        switch (dataType->getOp())
        {
        case kIROp_IntType:
        case kIROp_FloatType:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
        case kIROp_Int64Type:
        case kIROp_DoubleType:
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_HalfType:
        case kIROp_BoolType:
        case kIROp_IntPtrType:
        case kIROp_UIntPtrType:
        case kIROp_PtrType:
            context->marshalBasicType(builder, dataType, concreteTypedVar);
            break;
        case kIROp_EnumType:
            {
                auto enumType = static_cast<IREnumType*>(dataType);
                auto tagType = enumType->getTagType();
                context->marshalBasicType(builder, tagType, concreteTypedVar);
                break;
            }
        case kIROp_VectorType:
            {
                auto vectorType = static_cast<IRVectorType*>(dataType);
                auto elementCount = getIntVal(vectorType->getElementCount());
                for (IRIntegerValue i = 0; i < elementCount; i++)
                {
                    auto elementAddr = builder->emitElementAddress(
                        concreteTypedVar,
                        builder->getIntValue(builder->getIntType(), i));
                    emitMarshallingCode(builder, context, elementAddr);
                }
                break;
            }
        case kIROp_MatrixType:
            {
                auto matrixType = static_cast<IRMatrixType*>(dataType);
                auto colCount = getIntVal(matrixType->getColumnCount());
                auto rowCount = getIntVal(matrixType->getRowCount());
                if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                {
                    for (IRIntegerValue i = 0; i < colCount; i++)
                    {
                        for (IRIntegerValue j = 0; j < rowCount; j++)
                        {
                            auto row = builder->emitElementAddress(
                                concreteTypedVar,
                                builder->getIntValue(builder->getIntType(), j));
                            auto element = builder->emitElementAddress(
                                row,
                                builder->getIntValue(builder->getIntType(), i));
                            emitMarshallingCode(builder, context, element);
                        }
                    }
                }
                else
                {
                    for (IRIntegerValue i = 0; i < rowCount; i++)
                    {
                        auto row = builder->emitElementAddress(
                            concreteTypedVar,
                            builder->getIntValue(builder->getIntType(), i));
                        for (IRIntegerValue j = 0; j < colCount; j++)
                        {
                            auto element = builder->emitElementAddress(
                                row,
                                builder->getIntValue(builder->getIntType(), j));
                            emitMarshallingCode(builder, context, element);
                        }
                    }
                }
                break;
            }
        case kIROp_StructType:
            {
                auto structType = cast<IRStructType>(dataType);
                for (auto field : structType->getFields())
                {
                    auto fieldAddr = builder->emitFieldAddress(
                        builder->getPtrType(field->getFieldType()),
                        concreteTypedVar,
                        field->getKey());
                    emitMarshallingCode(builder, context, fieldAddr);
                }
                break;
            }
        case kIROp_ArrayType:
            {
                auto arrayType = cast<IRArrayType>(dataType);
                for (IRIntegerValue i = 0; i < getIntVal(arrayType->getElementCount()); i++)
                {
                    auto fieldAddr = builder->emitElementAddress(
                        concreteTypedVar,
                        builder->getIntValue(builder->getIntType(), i));
                    emitMarshallingCode(builder, context, fieldAddr);
                }
                break;
            }
        case kIROp_AnyValueType:
            {
                auto anyValType = cast<IRAnyValueType>(dataType);
                auto info = ensureAnyValueType(anyValType);
                for (auto field : info->fieldKeys)
                {
                    auto fieldAddr = builder->emitFieldAddress(
                        builder->getPtrType(builder->getUIntType()),
                        concreteTypedVar,
                        field);
                    emitMarshallingCode(builder, context, fieldAddr);
                }
                break;
            }
        case kIROp_DescriptorHandleType:
            {
                // DescriptorHandle<T> representation depends on target:
                // - Non-bindless (HLSL, GLSL/SPIRV): uint2 (8 bytes)
                // - Bindless (CUDA, CPU, Metal): same as T (variable size)
                auto target = context->targetProgram->getOptionSet().getTarget();
                bool bindless = areResourceTypesBindlessOnTarget(target);

                IRSizeAndAlignment sizeAndAlign;
                auto result = getNaturalSizeAndAlignment(
                    context->targetProgram->getOptionSet(),
                    dataType,
                    &sizeAndAlign);

                IRIntegerValue size = 8; // Default to 8 bytes for non-bindless
                if (SLANG_SUCCEEDED(result) && sizeAndAlign.size > 0)
                {
                    size = sizeAndAlign.size;
                }

                context
                    ->marshalDescriptorHandle(builder, dataType, concreteTypedVar, bindless, size);
                break;
            }
        default:
            if (isResourceType(dataType))
            {
                context->marshalResourceHandle(builder, dataType, concreteTypedVar);
                return;
            }
            SLANG_UNIMPLEMENTED_X("Unimplemented type packing");
            break;
        }
    }

    struct TypePackingContext : TypeMarshallingContext
    {
        virtual void marshalBasicType(IRBuilder* builder, IRType* dataType, IRInst* concreteVar)
            override
        {
            switch (dataType->getOp())
            {
            case kIROp_IntType:
            case kIROp_FloatType:
#if SLANG_PTR_IS_32
            case kIROp_IntPtrType:
#endif
                {
                    ensureOffsetAt4ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcVal = builder->emitLoad(concreteVar);
                        auto dstVal = builder->emitBitCast(builder->getUIntType(), srcVal);
                        auto dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        builder->emitStore(dstAddr, dstVal);
                    }
                    advanceOffset(4);
                    break;
                }
            case kIROp_BoolType:
                {
                    ensureOffsetAt4ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcVal = builder->emitLoad(concreteVar);
                        IRInst* args[] = {
                            srcVal,
                            builder->getIntValue(builder->getUIntType(), 1),
                            builder->getIntValue(builder->getUIntType(), 0)};
                        auto dstVal = builder->emitIntrinsicInst(
                            builder->getUIntType(),
                            kIROp_Select,
                            3,
                            args);
                        auto dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        builder->emitStore(dstAddr, dstVal);
                    }
                    advanceOffset(4);
                    break;
                }
            case kIROp_UIntType:
#if SLANG_PTR_IS_32
            case kIROp_UIntPtrType:
#endif
                {
                    ensureOffsetAt4ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcVal = builder->emitLoad(concreteVar);
                        auto dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        builder->emitStore(dstAddr, srcVal);
                    }
                    advanceOffset(4);
                    break;
                }
            case kIROp_Int16Type:
            case kIROp_UInt16Type:
            case kIROp_HalfType:
                {
                    ensureOffsetAt2ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcVal = builder->emitLoad(concreteVar);
                        if (dataType->getOp() == kIROp_HalfType)
                        {
                            srcVal =
                                builder->emitBitCast(builder->getType(kIROp_UInt16Type), srcVal);
                        }
                        srcVal = builder->emitCast(builder->getType(kIROp_UIntType), srcVal);
                        auto dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        auto dstVal = builder->emitLoad(dstAddr);
                        if (intraFieldOffset == 0)
                        {
                            dstVal = builder->emitBitAnd(
                                dstVal->getFullType(),
                                dstVal,
                                builder->getIntValue(builder->getUIntType(), 0xFFFF0000));
                        }
                        else
                        {
                            srcVal = builder->emitShl(
                                srcVal->getFullType(),
                                srcVal,
                                builder->getIntValue(builder->getUIntType(), 16));
                            dstVal = builder->emitBitAnd(
                                dstVal->getFullType(),
                                dstVal,
                                builder->getIntValue(builder->getUIntType(), 0xFFFF));
                        }
                        dstVal = builder->emitBitOr(dstVal->getFullType(), dstVal, srcVal);
                        builder->emitStore(dstAddr, dstVal);
                    }
                    advanceOffset(2);
                    break;
                }
            case kIROp_Int8Type:
            case kIROp_UInt8Type:
                if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcVal = builder->emitLoad(concreteVar);
                    srcVal = builder->emitCast(builder->getType(kIROp_UIntType), srcVal);
                    auto dstAddr = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    auto dstVal = builder->emitLoad(dstAddr);
                    dstVal = builder->emitBitfieldInsert(
                        dstVal->getFullType(),
                        dstVal,
                        srcVal,
                        builder->getIntValue(builder->getUIntType(), 8 * intraFieldOffset),
                        builder->getIntValue(builder->getUIntType(), 8));
                    builder->emitStore(dstAddr, dstVal);
                }
                advanceOffset(1);
                break;
            case kIROp_UInt64Type:
            case kIROp_Int64Type:
            case kIROp_DoubleType:
                ensureOffsetAt8ByteBoundary();
                if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcVal = builder->emitLoad(concreteVar);
                    auto dstVal = builder->emitBitCast(builder->getUInt64Type(), srcVal);
                    auto lowBits = builder->emitCast(builder->getUIntType(), dstVal);
                    auto highBits = builder->emitShr(
                        builder->getUInt64Type(),
                        dstVal,
                        builder->getIntValue(builder->getIntType(), 32));
                    highBits = builder->emitCast(builder->getUIntType(), highBits);

                    auto dstAddr = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    builder->emitStore(dstAddr, lowBits);
                    fieldOffset++;
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        builder->emitStore(dstAddr, highBits);
                        fieldOffset++;
                    }
                }
                break;
            case kIROp_PtrType:
#if SLANG_PTR_IS_64
            case kIROp_UIntPtrType:
            case kIROp_IntPtrType:
#endif
                ensureOffsetAt8ByteBoundary();
                if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcVal = builder->emitLoad(concreteVar);
                    // Use uint2 instead of uint64 to avoid Int64 capability requirement
                    auto uint2Type = builder->getVectorType(builder->getUIntType(), 2);
                    auto uint2Val = builder->emitBitCast(uint2Type, srcVal);
                    auto lowBits = builder->emitElementExtract(uint2Val, IRIntegerValue(0));
                    auto highBits = builder->emitElementExtract(uint2Val, IRIntegerValue(1));

                    auto dstAddr = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    builder->emitStore(dstAddr, lowBits);
                    fieldOffset++;
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        builder->emitStore(dstAddr, highBits);
                        fieldOffset++;
                    }
                }
                break;
            default:
                SLANG_UNREACHABLE("unknown basic type");
            }
        }

        virtual void marshalResourceHandle(
            IRBuilder* builder,
            IRType* dataType,
            IRInst* concreteVar) override
        {
            SLANG_UNUSED(dataType);
            ensureOffsetAt4ByteBoundary();
            if (fieldOffset + 1 < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
            {
                auto srcVal = builder->emitLoad(concreteVar);
                // Use uint2 instead of uint64 to avoid Int64 capability requirement
                auto uint2Type = builder->getVectorType(builder->getUIntType(), 2);
                auto uint2Val = builder->emitBitCast(uint2Type, srcVal);
                auto lowBits = builder->emitElementExtract(uint2Val, IRIntegerValue(0));
                auto highBits = builder->emitElementExtract(uint2Val, IRIntegerValue(1));
                auto dstAddr1 = builder->emitFieldAddress(
                    uintPtrType,
                    anyValueVar,
                    anyValInfo->fieldKeys[fieldOffset]);
                builder->emitStore(dstAddr1, lowBits);
                auto dstAddr2 = builder->emitFieldAddress(
                    uintPtrType,
                    anyValueVar,
                    anyValInfo->fieldKeys[fieldOffset + 1]);
                builder->emitStore(dstAddr2, highBits);
                advanceOffset(8);
            }
        }

        virtual void marshalDescriptorHandle(
            IRBuilder* builder,
            IRType* dataType,
            IRInst* concreteVar,
            bool isBindless,
            IRIntegerValue sizeInBytes) override
        {
            ensureOffsetAt4ByteBoundary();

            if (!isBindless)
            {
                // Non-bindless targets: Use CastDescriptorHandleToUInt2
                // This has proper lowering support in GLSL/SPIRV/HLSL emitters
                if (fieldOffset + 1 < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcVal = builder->emitLoad(concreteVar);
                    auto uint2Type = builder->getVectorType(builder->getUIntType(), 2);

                    // Use the explicit IR cast op instead of bitcast
                    auto uint2Val = builder->emitIntrinsicInst(
                        uint2Type,
                        kIROp_CastDescriptorHandleToUInt2,
                        1,
                        &srcVal);

                    auto lowBits = builder->emitElementExtract(uint2Val, IRIntegerValue(0));
                    auto highBits = builder->emitElementExtract(uint2Val, IRIntegerValue(1));

                    auto dstAddr1 = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    builder->emitStore(dstAddr1, lowBits);

                    auto dstAddr2 = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset + 1]);
                    builder->emitStore(dstAddr2, highBits);
                }
                advanceOffset(8);
            }
            else
            {
                // Bindless targets (CUDA, CPU, Metal): DescriptorHandle is a native type
                // Marshal as raw bytes based on actual size
                SLANG_UNUSED(dataType);

                auto numFields = (sizeInBytes + 3) / 4;

                // For bindless targets, we can use bitcast since the type is native
                auto srcVal = builder->emitLoad(concreteVar);
                auto uintType = builder->getUIntType();
                auto uintArrayType = builder->getArrayType(
                    uintType,
                    builder->getIntValue(builder->getIntType(), numFields));
                auto srcBitcast = builder->emitBitCast(uintArrayType, srcVal);

                for (IRIntegerValue i = 0; i < numFields; i++)
                {
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcElement = builder->emitElementExtract(srcBitcast, i);
                        auto dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        builder->emitStore(dstAddr, srcElement);
                    }
                    advanceOffset(4);
                }
            }
        }
    };

    IRFunc* generatePackingFunc(IRType* type, IRAnyValueType* anyValueType)
    {
        IRBuilder builder(module);
        builder.setInsertBefore(type);
        auto anyValInfo = ensureAnyValueType(anyValueType);

        auto func = builder.createFunc();

        StringBuilder nameSb;
        nameSb << "packAnyValue" << getIntVal(anyValueType->getSize());
        builder.addNameHintDecoration(func, nameSb.getUnownedSlice());
        // Currently we don't add linkage to the generated func, since we
        // do not have a way to compute mangled names from an IR entity.
        // This will leads to duplicate packing functions in linked code
        // but there won't be correctness issues.

        auto funcType = builder.getFuncType(1, &type, anyValInfo->type);
        func->setFullType(funcType);
        builder.setInsertInto(func);

        builder.emitBlock();

        auto param = builder.emitParam(type);
        auto concreteTypedVar = builder.emitVar(type);
        builder.emitStore(concreteTypedVar, param);
        auto resultVar = builder.emitVar(anyValInfo->type);

        // Initialize fields to 0 to prevent downstream compiler error.
        for (uint32_t offset = 0; offset < (uint32_t)anyValInfo->fieldKeys.getCount(); offset++)
        {
            auto fieldAddr = builder.emitFieldAddress(
                builder.getPtrType(builder.getUIntType()),
                resultVar,
                anyValInfo->fieldKeys[offset]);
            builder.emitStore(fieldAddr, builder.getIntValue(builder.getUIntType(), 0));
        }

        TypePackingContext context;
        context.anyValInfo = anyValInfo;
        context.fieldOffset = context.intraFieldOffset = 0;
        context.uintPtrType = builder.getPtrType(builder.getUIntType());
        context.anyValueVar = resultVar;
        context.targetProgram = targetProgram;
        emitMarshallingCode(&builder, &context, concreteTypedVar);

        auto load = builder.emitLoad(resultVar);
        builder.emitReturn(load);
        return func;
    }

    struct TypeUnpackingContext : TypeMarshallingContext
    {
        virtual void marshalBasicType(IRBuilder* builder, IRType* dataType, IRInst* concreteVar)
            override
        {
            switch (dataType->getOp())
            {
            case kIROp_IntType:
            case kIROp_FloatType:
                {
                    ensureOffsetAt4ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        auto srcVal = builder->emitLoad(srcAddr);
                        srcVal = builder->emitBitCast(dataType, srcVal);
                        builder->emitStore(concreteVar, srcVal);
                    }
                    advanceOffset(4);
                    break;
                }
            case kIROp_BoolType:
                {
                    ensureOffsetAt4ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        auto srcVal = builder->emitLoad(srcAddr);
                        srcVal = builder->emitNeq(
                            srcVal,
                            builder->getIntValue(builder->getUIntType(), 0));
                        builder->emitStore(concreteVar, srcVal);
                    }
                    advanceOffset(4);
                    break;
                }
            case kIROp_UIntType:
                {
                    ensureOffsetAt4ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        auto srcVal = builder->emitLoad(srcAddr);
                        builder->emitStore(concreteVar, srcVal);
                    }
                    advanceOffset(4);
                    break;
                }
            case kIROp_Int16Type:
            case kIROp_UInt16Type:
            case kIROp_HalfType:
                {
                    ensureOffsetAt2ByteBoundary();
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        auto srcVal = builder->emitLoad(srcAddr);
                        if (intraFieldOffset == 0)
                        {
                            srcVal = builder->emitBitAnd(
                                srcVal->getFullType(),
                                srcVal,
                                builder->getIntValue(builder->getUIntType(), 0xFFFF));
                        }
                        else
                        {
                            srcVal = builder->emitShr(
                                srcVal->getFullType(),
                                srcVal,
                                builder->getIntValue(builder->getUIntType(), 16));
                        }
                        if (dataType->getOp() == kIROp_Int16Type)
                        {
                            srcVal = builder->emitCast(builder->getType(kIROp_Int16Type), srcVal);
                        }
                        else
                        {
                            srcVal = builder->emitCast(builder->getType(kIROp_UInt16Type), srcVal);
                        }
                        if (dataType->getOp() == kIROp_HalfType)
                        {
                            srcVal = builder->emitBitCast(dataType, srcVal);
                        }
                        builder->emitStore(concreteVar, srcVal);
                    }
                    advanceOffset(2);
                    break;
                }
            case kIROp_Int8Type:
            case kIROp_UInt8Type:
                if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcAddr = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    auto srcVal = builder->emitLoad(srcAddr);
                    srcVal = builder->emitBitfieldExtract(
                        srcVal->getFullType(),
                        srcVal,
                        builder->getIntValue(builder->getUIntType(), 8 * intraFieldOffset),
                        builder->getIntValue(builder->getUIntType(), 8));
                    if (dataType->getOp() == kIROp_Int8Type)
                    {
                        srcVal = builder->emitCast(builder->getType(kIROp_Int8Type), srcVal);
                    }
                    else
                    {
                        srcVal = builder->emitCast(builder->getType(kIROp_UInt8Type), srcVal);
                    }
                    builder->emitStore(concreteVar, srcVal);
                }
                advanceOffset(1);
                break;
            case kIROp_UInt64Type:
            case kIROp_Int64Type:
            case kIROp_DoubleType:
                ensureOffsetAt8ByteBoundary();
                if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcAddr = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    auto lowBits = builder->emitLoad(srcAddr);
                    fieldOffset++;
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr1 = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        fieldOffset++;
                        auto highBits = builder->emitLoad(srcAddr1);
                        auto combinedBits = builder->emitMakeUInt64(lowBits, highBits);
                        if (dataType->getOp() != kIROp_UInt64Type)
                            combinedBits = builder->emitBitCast(dataType, combinedBits);
                        builder->emitStore(concreteVar, combinedBits);
                    }
                }
                break;
            case kIROp_PtrType:
#if SLANG_PTR_IS_64
            case kIROp_IntPtrType:
            case kIROp_UIntPtrType:
#endif
                ensureOffsetAt8ByteBoundary();
                if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcAddr = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    auto lowBits = builder->emitLoad(srcAddr);
                    fieldOffset++;
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr1 = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        fieldOffset++;
                        auto highBits = builder->emitLoad(srcAddr1);
                        // Use uint2 instead of uint64 to avoid Int64 capability requirement
                        auto uint2Type = builder->getVectorType(builder->getUIntType(), 2);
                        IRInst* components[2] = {lowBits, highBits};
                        auto uint2Val = builder->emitMakeVector(uint2Type, 2, components);
                        auto combinedBits = builder->emitBitCast(dataType, uint2Val);
                        builder->emitStore(concreteVar, combinedBits);
                    }
                }
                break;
            default:
                SLANG_UNREACHABLE("unknown basic type");
            }
        }

        virtual void marshalResourceHandle(
            IRBuilder* builder,
            IRType* dataType,
            IRInst* concreteVar) override
        {
            ensureOffsetAt4ByteBoundary();
            if (fieldOffset + 1 < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
            {
                auto srcAddr = builder->emitFieldAddress(
                    uintPtrType,
                    anyValueVar,
                    anyValInfo->fieldKeys[fieldOffset]);
                auto lowBits = builder->emitLoad(srcAddr);

                auto srcAddr1 = builder->emitFieldAddress(
                    uintPtrType,
                    anyValueVar,
                    anyValInfo->fieldKeys[fieldOffset + 1]);
                auto highBits = builder->emitLoad(srcAddr1);

                // Use uint2 instead of uint64 to avoid Int64 capability requirement
                auto uint2Type = builder->getVectorType(builder->getUIntType(), 2);
                IRInst* components[2] = {lowBits, highBits};
                auto uint2Val = builder->emitMakeVector(uint2Type, 2, components);
                auto combinedBits = builder->emitBitCast(dataType, uint2Val);
                builder->emitStore(concreteVar, combinedBits);
                advanceOffset(8);
            }
        }

        virtual void marshalDescriptorHandle(
            IRBuilder* builder,
            IRType* dataType,
            IRInst* concreteVar,
            bool isBindless,
            IRIntegerValue sizeInBytes) override
        {
            ensureOffsetAt4ByteBoundary();

            if (!isBindless)
            {
                // Non-bindless targets: Use CastUInt2ToDescriptorHandle
                if (fieldOffset + 1 < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcAddr = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset]);
                    auto lowBits = builder->emitLoad(srcAddr);

                    auto srcAddr1 = builder->emitFieldAddress(
                        uintPtrType,
                        anyValueVar,
                        anyValInfo->fieldKeys[fieldOffset + 1]);
                    auto highBits = builder->emitLoad(srcAddr1);

                    // Build uint2 from components
                    auto uint2Type = builder->getVectorType(builder->getUIntType(), 2);
                    IRInst* components[2] = {lowBits, highBits};
                    auto uint2Val = builder->emitMakeVector(uint2Type, 2, components);

                    // Use the explicit IR cast op instead of bitcast
                    auto result = builder->emitIntrinsicInst(
                        dataType,
                        kIROp_CastUInt2ToDescriptorHandle,
                        1,
                        &uint2Val);

                    builder->emitStore(concreteVar, result);
                }
                advanceOffset(8);
            }
            else
            {
                // Bindless targets (CUDA, CPU, Metal): DescriptorHandle is a native type
                // Unmarshal from raw bytes based on actual size
                auto numFields = (sizeInBytes + 3) / 4;

                auto uintType = builder->getUIntType();
                auto uintArrayType = builder->getArrayType(
                    uintType,
                    builder->getIntValue(builder->getIntType(), numFields));

                // Create a temporary variable to hold the uint32 array
                auto tempVar = builder->emitVar(uintArrayType);

                // Read each uint32 field from anyValue into the temp array
                for (IRIntegerValue i = 0; i < numFields; i++)
                {
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        auto srcVal = builder->emitLoad(srcAddr);
                        auto dstAddr = builder->emitElementAddress(
                            tempVar,
                            builder->getIntValue(builder->getIntType(), i));
                        builder->emitStore(dstAddr, srcVal);
                    }
                    advanceOffset(4);
                }

                // Bitcast the uint32 array to the target type
                auto tempVal = builder->emitLoad(tempVar);
                auto result = builder->emitBitCast(dataType, tempVal);
                builder->emitStore(concreteVar, result);
            }
        }
    };

    IRFunc* generateUnpackingFunc(IRType* type, IRAnyValueType* anyValueType)
    {
        IRBuilder builder(module);
        builder.setInsertBefore(type);
        auto anyValInfo = ensureAnyValueType(anyValueType);

        auto func = builder.createFunc();

        StringBuilder nameSb;
        nameSb << "unpackAnyValue" << getIntVal(anyValueType->getSize());
        builder.addNameHintDecoration(func, nameSb.getUnownedSlice());

        auto funcType = builder.getFuncType(1, &anyValInfo->type, type);
        func->setFullType(funcType);
        builder.setInsertInto(func);

        builder.emitBlock();

        auto param = builder.emitParam(anyValInfo->type);
        auto anyValueVar = builder.emitVar(anyValInfo->type);
        builder.emitStore(anyValueVar, param);
        auto resultVar = builder.emitVar(type);

        TypeUnpackingContext context;
        context.anyValInfo = anyValInfo;
        context.fieldOffset = context.intraFieldOffset = 0;
        context.uintPtrType = builder.getPtrType(builder.getUIntType());
        context.anyValueVar = anyValueVar;
        context.targetProgram = targetProgram;
        emitMarshallingCode(&builder, &context, resultVar);
        auto load = builder.emitLoad(resultVar);
        builder.emitReturn(load);
        return func;
    }

    // Ensures the marshalling functions between `type` and `anyValueType` are already generated.
    // Returns the generated marshalling functions.
    MarshallingFunctionSet ensureMarshallingFunc(IRType* type, IRAnyValueType* anyValueType)
    {
        auto size = getIntVal(anyValueType->getSize());
        MarshallingFunctionKey key;
        key.originalType = type;
        key.anyValueSize = size;
        MarshallingFunctionSet funcSet;
        if (mapTypeMarshalingFunctions.tryGetValue(key, funcSet))
            return funcSet;
        funcSet.packFunc = generatePackingFunc(type, anyValueType);
        funcSet.unpackFunc = generateUnpackingFunc(type, anyValueType);
        mapTypeMarshalingFunctions[key] = funcSet;
        return funcSet;
    }

    void processPackInst(IRPackAnyValue* packInst)
    {
        auto operand = packInst->getValue();
        auto func = ensureMarshallingFunc(
            operand->getDataType(),
            cast<IRAnyValueType>(packInst->getDataType()));
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(packInst);
        auto callInst = builder->emitCallInst(packInst->getDataType(), func.packFunc, 1, &operand);
        packInst->replaceUsesWith(callInst);
        packInst->removeAndDeallocate();
    }

    void processUnpackInst(IRUnpackAnyValue* unpackInst)
    {
        auto operand = unpackInst->getValue();
        auto func = ensureMarshallingFunc(
            unpackInst->getDataType(),
            cast<IRAnyValueType>(operand->getDataType()));
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(unpackInst);
        auto callInst =
            builder->emitCallInst(unpackInst->getDataType(), func.unpackFunc, 1, &operand);
        unpackInst->replaceUsesWith(callInst);
        unpackInst->removeAndDeallocate();
    }

    void processAnyValueType(IRAnyValueType* type)
    {
        auto info = ensureAnyValueType(type);
        type->replaceUsesWith(info->type);
    }

    void processInst(IRInst* inst)
    {
        if (auto packInst = as<IRPackAnyValue>(inst))
        {
            processPackInst(packInst);
        }
        else if (auto unpackInst = as<IRUnpackAnyValue>(inst))
        {
            processUnpackInst(unpackInst);
        }
    }

    void processModule()
    {
        // We start by initializing our shared IR building state,
        // since we will re-use that state for any code we
        // generate along the way.
        //
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Finally, replace all `AnyValueType` with the actual struct type that implements it.
        for (auto inst : module->getModuleInst()->getChildren())
        {
            if (auto anyValueType = as<IRAnyValueType>(inst))
                processAnyValueType(anyValueType);
        }
    }
};

void generateAnyValueMarshallingFunctions(IRModule* module, TargetProgram* targetProgram)
{
    AnyValueMarshallingContext context(module, targetProgram);
    context.processModule();
}

SlangInt alignUp(SlangInt x, SlangInt alignment)
{
    return (x + alignment - 1) / alignment * alignment;
}

SlangInt _getAnyValueSizeRaw(CompilerOptionSet& optionSet, IRType* type, SlangInt offset)
{
    // Try to get size and alignment from the layout system.
    IRSizeAndAlignment sizeAndAlignment;

    if (SLANG_SUCCEEDED(getSizeAndAlignment(
            optionSet,
            IRTypeLayoutRules::getNatural(),
            type,
            &sizeAndAlignment)))
    {
        // Check for valid size (not indeterminate and non-negative)
        if (sizeAndAlignment.size != IRSizeAndAlignment::kIndeterminateSize &&
            sizeAndAlignment.size >= 0)
        {
            return alignUp(offset, sizeAndAlignment.alignment) + sizeAndAlignment.size;
        }
    }

    // Handle special cases not covered by the layout system
    switch (type->getOp())
    {
    case kIROp_AssociatedType:
        {
            auto associatedType = cast<IRAssociatedType>(type);
            SlangInt maxSize = 0;
            for (UInt i = 0; i < associatedType->getOperandCount(); i++)
                maxSize = Math::Max(
                    maxSize,
                    _getAnyValueSizeRaw(optionSet, (IRType*)associatedType->getOperand(i), offset));
            return maxSize;
        }
    case kIROp_ThisType:
        {
            auto thisType = cast<IRThisType>(type);
            auto interfaceType = thisType->getConstraintType();
            auto size = getInterfaceAnyValueSize(interfaceType, interfaceType->sourceLoc);
            return alignUp(offset, 4) + alignUp((SlangInt)size, 4);
        }
    case kIROp_ExtractExistentialType:
        {
            auto existentialValue = type->getOperand(0);
            auto interfaceType = cast<IRInterfaceType>(existentialValue->getDataType());
            auto size = getInterfaceAnyValueSize(interfaceType, interfaceType->sourceLoc);
            return alignUp(offset, 4) + alignUp((SlangInt)size, 4);
        }
    case kIROp_LookupWitnessMethod:
        {
            auto witnessTableVal = type->getOperand(0);
            auto key = type->getOperand(1);
            IRType* assocType = nullptr;
            if (auto witnessTableType = as<IRWitnessTableTypeBase>(witnessTableVal->getDataType()))
            {
                auto interfaceType = as<IRInterfaceType>(witnessTableType->getConformanceType());

                // Walk through interface operands to find a match, the result should be an
                // associated type entry.
                //
                for (UIndex ii = 0; ii < interfaceType->getOperandCount(); ii++)
                {
                    auto entry = cast<IRInterfaceRequirementEntry>(interfaceType->getOperand(ii));
                    if (entry->getRequirementKey() == key &&
                        as<IRAssociatedType>(entry->getRequirementVal()))
                    {
                        assocType = (IRType*)entry->getRequirementVal();
                        break;
                    }
                }
            }

            if (!assocType)
                return -1;

            IRIntegerValue anyValueSize = kInvalidAnyValueSize;
            for (UInt i = 0; i < assocType->getOperandCount(); i++)
            {
                anyValueSize = Math::Min(
                    anyValueSize,
                    getInterfaceAnyValueSize(assocType->getOperand(i), type->sourceLoc));
            }

            if (anyValueSize == kInvalidAnyValueSize)
                return -1;

            return alignUp(offset, 4) + alignUp((SlangInt)anyValueSize, 4);
        }

        // treat CoopVec as an opaque handle type
    case kIROp_CoopVectorType:
        return alignUp(offset, 4) + 8;

    default:
        return -1;
    }
}

SlangInt getAnyValueSize(CompilerOptionSet& optionSet, IRType* type)
{
    auto rawSize = _getAnyValueSizeRaw(optionSet, type, 0);
    if (rawSize < 0)
        return rawSize;
    return alignUp(rawSize, 4);
}
} // namespace Slang
