#include "slang-ir-any-value-marshalling.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
    // This is a subpass of generics lowering IR transformation.
    // This pass generates packing/unpacking functions for `AnyValue`s,
    // and replaces all `IRPackAnyValue` and `IRUnpackAnyValue` with calls to these
    // functions.
    struct AnyValueMarshallingContext
    {
        SharedGenericsLoweringContext* sharedContext;

        // Stores information about generated `AnyValue` struct types.
        struct AnyValueTypeInfo : RefObject
        {
            IRType* type; // The generated IR value for the `AnyValue<N>` struct type.
            List<IRStructKey*> fieldKeys; // `IRStructKey`s for the fields of the generated type.
        };

        Dictionary<IRIntegerValue, RefPtr<AnyValueTypeInfo>> generatedAnyValueTypes;

        struct MarshallingFunctionKey
        {
            IRType* originalType;
            IRIntegerValue anyValueSize;
            bool operator ==(MarshallingFunctionKey other)
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
            if (auto typeInfo = generatedAnyValueTypes.TryGetValue(size))
                return typeInfo->Ptr();
            RefPtr<AnyValueTypeInfo> info = new AnyValueTypeInfo();
            IRBuilder builder;
            builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
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
                nameSb.Clear();
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
            // Defines what to do with basic typed data elements.
            virtual void marshalBasicType(IRBuilder* builder, IRType* dataType, IRInst* concreteTypedVar) = 0;
            // Defines what to do with resource handle elements.
            virtual void marshalResourceHandle(IRBuilder* builder, IRType* dataType, IRInst* concreteTypedVar) = 0;
            // Validates that the type fits in the given AnyValueSize.
            // After calling emitMarshallingCode, `fieldOffset` will be increased to the required `AnyValue` size.
            // If this is larger than the provided AnyValue size, report a dianogstic. We might want to front load
            // this in a separate IR validation pass in the future, but this is the easiest way to report the
            // diagnostic now.
            void validateAnyTypeSize(DiagnosticSink* sink, IRType* concreteType)
            {
                if (fieldOffset > static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    sink->diagnose(concreteType->sourceLoc, Diagnostics::typeDoesNotFitAnyValueSize, concreteType);
                }
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
                context->marshalBasicType(builder, dataType, concreteTypedVar);
                break;
            case kIROp_VectorType:
            {
                auto vectorType = static_cast<IRVectorType*>(dataType);
                auto elementType = vectorType->getElementType();
                auto elementCount = getIntVal(vectorType->getElementCount());
                auto elementPtrType = builder->getPtrType(elementType);
                for (IRIntegerValue i = 0; i < elementCount; i++)
                {
                    auto elementAddr = builder->emitElementAddress(
                        elementPtrType,
                        concreteTypedVar,
                        builder->getIntValue(builder->getIntType(), i));
                    emitMarshallingCode(builder, context, elementAddr);
                }
                break;
            }
            case kIROp_MatrixType:
            {
                auto matrixType = static_cast<IRMatrixType*>(dataType);
                auto elementType = matrixType->getElementType();
                auto colCount = getIntVal(matrixType->getColumnCount());
                auto rowCount = getIntVal(matrixType->getRowCount());
                for (IRIntegerValue i = 0; i < colCount; i++)
                {
                    auto col = builder->emitElementAddress(
                        elementType,
                        concreteTypedVar,
                        builder->getIntValue(builder->getIntType(), i));
                    for (IRIntegerValue j = 0; j < rowCount; j++)
                    {
                        auto element = builder->emitElementExtract(
                            elementType,
                            col,
                            builder->getIntValue(builder->getIntType(), i));
                        emitMarshallingCode(builder, context, element);
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
                auto elementPtrType = builder->getPtrType(arrayType->getElementType());
                for (IRIntegerValue i = 0; i < getIntVal(arrayType->getElementCount()); i++)
                {
                    auto fieldAddr = builder->emitElementAddress(
                        elementPtrType,
                        concreteTypedVar,
                        builder->getIntValue(builder->getIntType(), i));
                    emitMarshallingCode(builder, context, fieldAddr);
                }
                break;
            }
            default:
                if (as<IRTextureTypeBase>(dataType) || as<IRSamplerStateTypeBase>(dataType))
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
            virtual void marshalBasicType(IRBuilder* builder, IRType* dataType, IRInst* concreteVar) override
            {
                switch (dataType->getOp())
                {
                case kIROp_IntType:
                case kIROp_FloatType:
                {
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
                    fieldOffset++;
                    break;
                }
                case kIROp_UIntType:
                {
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcVal = builder->emitLoad(concreteVar);
                        auto dstAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        builder->emitStore(dstAddr, srcVal);
                    }
                    fieldOffset++;
                    break;
                }
                case kIROp_UInt64Type:
                case kIROp_Int64Type:
                case kIROp_DoubleType:
                case kIROp_Int8Type:
                case kIROp_Int16Type:
                case kIROp_UInt8Type:
                case kIROp_UInt16Type:
                case kIROp_HalfType:
                    SLANG_UNIMPLEMENTED_X("AnyValue type packing for non 32-bit elements");
                    break;
                default:
                    SLANG_UNREACHABLE("unknown basic type");
                }
            }

            virtual void marshalResourceHandle(IRBuilder* builder, IRType* dataType, IRInst* concreteVar) override
            {
                SLANG_UNUSED(dataType);
                if (fieldOffset + 1 < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcVal = builder->emitLoad(concreteVar);
                    auto uint64Val = builder->emitBitCast(builder->getUInt64Type(), srcVal);
                    auto lowBits = builder->emitConstructorInst(builder->getUIntType(), 1, &uint64Val);
                    auto shiftedBits = builder->emitShr(
                        builder->getUInt64Type(),
                        uint64Val,
                        builder->getIntValue(builder->getIntType(), 32));
                    auto highBits = builder->emitBitCast(builder->getUIntType(), shiftedBits);
                    auto dstAddr1 = builder->emitFieldAddress(
                        uintPtrType, anyValueVar, anyValInfo->fieldKeys[fieldOffset]);
                    builder->emitStore(dstAddr1, lowBits);
                    auto dstAddr2 = builder->emitFieldAddress(
                        uintPtrType, anyValueVar, anyValInfo->fieldKeys[fieldOffset + 1]);
                    builder->emitStore(dstAddr2, highBits);
                    fieldOffset += 2;
                }
            }
        };

        IRFunc* generatePackingFunc(IRType* type, IRAnyValueType* anyValueType)
        {
            IRBuilder builder;
            builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
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

            TypePackingContext context;
            context.anyValInfo = anyValInfo;
            context.fieldOffset = context.intraFieldOffset = 0;
            context.uintPtrType = builder.getPtrType(builder.getUIntType());
            context.anyValueVar = resultVar;
            emitMarshallingCode(&builder, &context, concreteTypedVar);

            context.validateAnyTypeSize(sharedContext->sink, type);

            // Initialize the rest of unused fields to 0 to prevent downstream compiler error.
            for (uint32_t offset = context.fieldOffset; offset < (uint32_t)anyValInfo->fieldKeys.getCount(); offset++)
            {
                auto fieldAddr = builder.emitFieldAddress(builder.getUIntType(), resultVar, context.anyValInfo->fieldKeys[offset]);
                builder.emitStore(fieldAddr, builder.getIntValue(builder.getUIntType(), 0));
            }

            auto load = builder.emitLoad(resultVar);
            builder.emitReturn(load);
            return func;
        }

        struct TypeUnpackingContext : TypeMarshallingContext
        {
            virtual void marshalBasicType(IRBuilder* builder, IRType* dataType, IRInst* concreteVar) override
            {
                switch (dataType->getOp())
                {
                case kIROp_IntType:
                case kIROp_FloatType:
                {
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
                    fieldOffset++;
                    break;
                }
                case kIROp_UIntType:
                {
                    if (fieldOffset < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                    {
                        auto srcAddr = builder->emitFieldAddress(
                            uintPtrType,
                            anyValueVar,
                            anyValInfo->fieldKeys[fieldOffset]);
                        auto srcVal = builder->emitLoad(srcAddr);
                        builder->emitStore(concreteVar, srcVal);
                    }
                    fieldOffset++;
                    break;
                }
                case kIROp_UInt64Type:
                case kIROp_Int64Type:
                case kIROp_DoubleType:
                case kIROp_Int8Type:
                case kIROp_Int16Type:
                case kIROp_UInt8Type:
                case kIROp_UInt16Type:
                case kIROp_HalfType:
                    SLANG_UNIMPLEMENTED_X("AnyValue type packing for non 32-bit elements");
                    break;
                default:
                    SLANG_UNREACHABLE("unknown basic type");
                }
            }

            virtual void marshalResourceHandle(
                IRBuilder* builder, IRType* dataType, IRInst* concreteVar) override
            {
                if (fieldOffset + 1 < static_cast<uint32_t>(anyValInfo->fieldKeys.getCount()))
                {
                    auto srcAddr = builder->emitFieldAddress(
                        uintPtrType, anyValueVar, anyValInfo->fieldKeys[fieldOffset]);
                    auto lowBits = builder->emitLoad(srcAddr);
                    
                    auto srcAddr1 = builder->emitFieldAddress(
                        uintPtrType, anyValueVar, anyValInfo->fieldKeys[fieldOffset + 1]);
                    auto highBits = builder->emitLoad(srcAddr1);

                    auto combinedBits = builder->emitMakeUInt64(lowBits, highBits);
                    combinedBits = builder->emitBitCast(dataType, combinedBits);
                    builder->emitStore(concreteVar, combinedBits);
                    fieldOffset += 2;
                }
            }
        };

        IRFunc* generateUnpackingFunc(IRType* type, IRAnyValueType* anyValueType)
        {
            IRBuilder builder;
            builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
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
            if (mapTypeMarshalingFunctions.TryGetValue(key, funcSet))
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
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
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
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(unpackInst);
            auto callInst = builder->emitCallInst(unpackInst->getDataType(), func.unpackFunc, 1, &operand);
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
            SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
            sharedBuilder->module = sharedContext->module;
            sharedBuilder->session = sharedContext->module->session;

            sharedContext->addToWorkList(sharedContext->module->getModuleInst());

            while (sharedContext->workList.getCount() != 0)
            {
                IRInst* inst = sharedContext->workList.getLast();

                sharedContext->workList.removeLast();
                sharedContext->workListSet.Remove(inst);

                processInst(inst);

                for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                {
                    sharedContext->addToWorkList(child);
                }
            }

            // Finally, replace all `AnyValueType` with the actual struct type that implements it.
            for (auto inst : sharedContext->module->getModuleInst()->getChildren())
            {
                if (auto anyValueType = as<IRAnyValueType>(inst))
                    processAnyValueType(anyValueType);
            }
            // Because we replaced all `AnyValueType` uses, some old type definitions (e.g. PtrType(AnyValueType))
            // will become duplicates with new types we introduced (e.g. PtrType(AnyValueStruct)), and therefore
            // invalidates our `globalValueNumberingMap` hash map. We need to rebuild it.
            sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();
        }
    };

    void generateAnyValueMarshallingFunctions(SharedGenericsLoweringContext* sharedContext)
    {
        AnyValueMarshallingContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }
}
