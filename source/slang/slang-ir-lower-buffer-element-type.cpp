#include "slang-ir-lower-buffer-element-type.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-layout.h"

namespace Slang
{
    struct LoweredElementTypeContext
    {
        struct LoweredElementTypeInfo
        {
            IRType* originalType;
            IRType* loweredType;
            IRType* loweredInnerArrayType = nullptr; // For matrix/array types that are lowered into a struct type, this is the inner array type of the data field.
            IRStructKey* loweredInnerStructKey = nullptr; // For matrix/array types that are lowered into a struct type, this is the struct key of the data field.
            IRFunc* convertOriginalToLowered = nullptr;
            IRFunc* convertLoweredToOriginal = nullptr;
        };

        Dictionary<IRType*, LoweredElementTypeInfo> loweredTypeInfo[(int)IRTypeLayoutRuleName::_Count];
        Dictionary<IRType*, LoweredElementTypeInfo> mapLoweredTypeToInfo[(int)IRTypeLayoutRuleName::_Count];

        SlangMatrixLayoutMode defaultMatrixLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
        TargetRequest* target;

        LoweredElementTypeContext(TargetRequest* target, SlangMatrixLayoutMode inDefaultMatrixLayout)
            : target(target), defaultMatrixLayout(inDefaultMatrixLayout)
        {}

        IRFunc* createMatrixUnpackFunc(
            IRMatrixType* matrixType,
            IRStructType* structType,
            IRStructKey* dataKey,
            IRArrayType* arrayType)
        {
            IRBuilder builder(structType);
            builder.setInsertAfter(structType);
            auto func = builder.createFunc();
            auto funcType = builder.getFuncType(1, (IRType**)&structType, matrixType);
            func->setFullType(funcType);
            builder.addNameHintDecoration(func, UnownedStringSlice("unpackStorage"));
            builder.setInsertInto(func);
            builder.emitBlock();
            auto rowCount = (Index)getIntVal(matrixType->getRowCount());
            auto colCount = (Index)getIntVal(matrixType->getColumnCount());
            auto packedParam = builder.emitParam(structType);
            auto vectorArray = builder.emitFieldExtract(arrayType, packedParam, dataKey);
            List<IRInst*> args;
            args.setCount(rowCount * colCount);
            if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
            {
                for (IRIntegerValue c = 0; c < colCount; c++)
                {
                    auto vector = builder.emitElementExtract(vectorArray, c);
                    for (IRIntegerValue r = 0; r < rowCount; r++)
                    {
                        auto element = builder.emitElementExtract(vector, r);
                        args[(Index)(r*colCount + c)] = element;
                    }
                }
            }
            else
            {
                for (IRIntegerValue r = 0; r < rowCount; r++)
                {
                    auto vector = builder.emitElementExtract(vectorArray, r);
                    for (IRIntegerValue c = 0; c < colCount; c++)
                    {
                        auto element = builder.emitElementExtract(vector, c);
                        args[(Index)(r * colCount + c)] = element;
                    }
                }
            }
            IRInst* result = builder.emitMakeMatrix(matrixType, (UInt)args.getCount(), args.getBuffer());
            builder.emitReturn(result);
            return func;
        }

        IRFunc* createMatrixPackFunc(
            IRMatrixType* matrixType,
            IRStructType* structType,
            IRVectorType* vectorType,
            IRArrayType* arrayType)
        {
            IRBuilder builder(structType);
            builder.setInsertAfter(structType);
            auto func = builder.createFunc();
            auto funcType = builder.getFuncType(1, (IRType**)&matrixType, structType);
            func->setFullType(funcType);
            builder.addNameHintDecoration(func, UnownedStringSlice("packMatrix"));
            builder.setInsertInto(func);
            builder.emitBlock();
            auto rowCount = getIntVal(matrixType->getRowCount());
            auto colCount = getIntVal(matrixType->getColumnCount());
            auto originalParam = builder.emitParam(matrixType);
            List<IRInst*> elements;
            elements.setCount((Index)(rowCount * colCount));
            for (IRIntegerValue r = 0; r < rowCount; r++)
            {
                auto vector = builder.emitElementExtract(originalParam, r);
                for (IRIntegerValue c = 0; c < colCount; c++)
                {
                    auto element = builder.emitElementExtract(vector, c);
                    elements[(Index)(r * colCount + c)] = element;
                }
            }
            List<IRInst*> vectors;
            if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
            {
                for (IRIntegerValue c = 0; c < colCount; c++)
                {
                    List<IRInst*> vecArgs;
                    for (IRIntegerValue r = 0; r < rowCount; r++)
                    {
                        auto element = elements[(Index)(r * colCount + c)];
                        vecArgs.add(element);
                    }
                    auto colVector = builder.emitMakeVector(vectorType, (UInt)vecArgs.getCount(), vecArgs.getBuffer());
                    vectors.add(colVector);
                }
            }
            else
            {
                for (IRIntegerValue r = 0; r < rowCount; r++)
                {
                    List<IRInst*> vecArgs;
                    for (IRIntegerValue c = 0; c < colCount; c++)
                    {
                        auto element = elements[(Index)(r * colCount + c)];
                        vecArgs.add(element);
                    }
                    auto rowVector = builder.emitMakeVector(vectorType, (UInt)vecArgs.getCount(), vecArgs.getBuffer());
                    vectors.add(rowVector);
                }
            }

            auto vectorArray = builder.emitMakeArray(arrayType, (UInt)vectors.getCount(), vectors.getBuffer());
            auto result = builder.emitMakeStruct(structType, 1, &vectorArray);
            builder.emitReturn(result);
            return func;
        }

        IRFunc* createArrayUnpackFunc(
            IRArrayType* arrayType,
            IRStructType* structType,
            IRStructKey* dataKey,
            IRArrayType* innerArrayType,
            LoweredElementTypeInfo innerTypeInfo)
        {
            IRBuilder builder(structType);
            builder.setInsertAfter(structType);
            auto func = builder.createFunc();
            auto funcType = builder.getFuncType(1, (IRType**)&structType, arrayType);
            func->setFullType(funcType);
            builder.addNameHintDecoration(func, UnownedStringSlice("unpackStorage"));
            builder.setInsertInto(func);
            builder.emitBlock();
            auto packedParam = builder.emitParam(structType);
            auto packedArray = builder.emitFieldExtract(innerArrayType, packedParam, dataKey);
            auto count = getIntVal(arrayType->getElementCount());
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto packedElement = builder.emitElementExtract(packedArray, ii);
                auto originalElement = innerTypeInfo.convertLoweredToOriginal
                    ? builder.emitCallInst(innerTypeInfo.originalType, innerTypeInfo.convertLoweredToOriginal, 1, &packedElement)
                    : packedElement;
                args[(Index)ii] = originalElement;
            }
            auto result = builder.emitMakeArray(arrayType, (UInt)args.getCount(), args.getBuffer());
            builder.emitReturn(result);
            return func;
        }

        IRFunc* createArrayPackFunc(
            IRArrayType* arrayType,
            IRStructType* structType,
            IRArrayType* innerArrayType,
            LoweredElementTypeInfo innerTypeInfo)
        {
            IRBuilder builder(structType);
            builder.setInsertAfter(structType);
            auto func = builder.createFunc();
            auto funcType = builder.getFuncType(1, (IRType**)&arrayType, structType);
            func->setFullType(funcType);
            builder.addNameHintDecoration(func, UnownedStringSlice("packStorage"));
            builder.setInsertInto(func);
            builder.emitBlock();
            auto originalParam = builder.emitParam(arrayType);
            auto count = getIntVal(arrayType->getElementCount());
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto originalElement = builder.emitElementExtract(originalParam, ii);
                auto packedElement = innerTypeInfo.convertOriginalToLowered
                    ? builder.emitCallInst(innerTypeInfo.loweredType, innerTypeInfo.convertOriginalToLowered, 1, &originalElement)
                    : originalElement;
                args[(Index)ii] = packedElement;
            }
            auto packedArray = builder.emitMakeArray(innerArrayType, (UInt)args.getCount(), args.getBuffer());
            auto result = builder.emitMakeStruct(structType, 1, &packedArray);
            builder.emitReturn(result);
            return func;
        }

        const char* getLayoutName(IRTypeLayoutRuleName name)
        {
            switch (name)
            {
                case IRTypeLayoutRuleName::Std140: return "std140";
                case IRTypeLayoutRuleName::Std430: return "std430";
                case IRTypeLayoutRuleName::Natural: return "natural";
                default: return "default";
            }
        }

        LoweredElementTypeInfo getLoweredTypeInfoImpl(IRType* type, IRTypeLayoutRules* rules)
        {
            IRBuilder builder(type);
            builder.setInsertAfter(type);

            LoweredElementTypeInfo info;
            info.originalType = type;

            if (auto matrixType = as<IRMatrixType>(type))
            {
                // For spirv, we always want to lower all matrix types, because matrix types
                // are considered abstract types.
                if (!target->shouldEmitSPIRVDirectly())
                {
                    // For other targets, we only lower the matrix types if they differ from the default
                    // matrix layout.
                    if (getIntVal(matrixType->getLayout()) == defaultMatrixLayout &&
                        rules->ruleName == IRTypeLayoutRuleName::Natural)
                    {
                        info.loweredType = type;
                        return info;
                    }
                }

                auto loweredType = builder.createStructType();
                StringBuilder nameSB;
                bool isColMajor = getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
                nameSB << "_MatrixStorage_";
                getTypeNameHint(nameSB, matrixType->getElementType());
                nameSB << getIntVal(matrixType->getRowCount()) << "x" << getIntVal(matrixType->getColumnCount());
                if (isColMajor)
                    nameSB << "_ColMajor";
                nameSB << getLayoutName(rules->ruleName);
                builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
                auto structKey = builder.createStructKey();
                builder.addNameHintDecoration(structKey, UnownedStringSlice("data"));
                auto vectorType = builder.getVectorType(matrixType->getElementType(),
                    isColMajor?matrixType->getRowCount():matrixType->getColumnCount());
                IRSizeAndAlignment elementSizeAlignment;
                getSizeAndAlignment(target, rules, vectorType, &elementSizeAlignment);
                elementSizeAlignment = rules->alignCompositeElement(elementSizeAlignment);

                auto arrayType = builder.getArrayType(
                    vectorType,
                    isColMajor?matrixType->getColumnCount():matrixType->getRowCount(),
                    builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride()));
                builder.createStructField(loweredType, structKey, arrayType);

                info.loweredType = loweredType;
                info.loweredInnerArrayType = arrayType;
                info.loweredInnerStructKey = structKey;
                info.convertLoweredToOriginal = createMatrixUnpackFunc(matrixType, loweredType, structKey, arrayType);
                info.convertOriginalToLowered = createMatrixPackFunc(matrixType, loweredType, vectorType, arrayType);
                return info;
            }
            else if (auto arrayType = as<IRArrayType>(type))
            {
                auto loweredInnerTypeInfo = getLoweredTypeInfo(arrayType->getElementType(), rules);
                // For spirv backend, we always want to lower all array types, even if the element type
                // comes out the same. This is because different layout rules may have different array
                // stride requirements.
                if (!target->shouldEmitSPIRVDirectly())
                {
                    if (!loweredInnerTypeInfo.convertLoweredToOriginal)
                    {
                        info.loweredType = type;
                        return info;
                    }
                }
                auto loweredType = builder.createStructType();
                info.loweredType = loweredType;
                StringBuilder nameSB;
                nameSB << "_Array_" << getLayoutName(rules->ruleName) << "_";
                getTypeNameHint(nameSB, arrayType->getElementType());
                nameSB << getIntVal(arrayType->getElementCount());
                builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
                auto structKey = builder.createStructKey();
                builder.addNameHintDecoration(structKey, UnownedStringSlice("data"));
                IRSizeAndAlignment elementSizeAlignment;
                getSizeAndAlignment(target, rules, loweredInnerTypeInfo.loweredType, &elementSizeAlignment);
                elementSizeAlignment = rules->alignCompositeElement(elementSizeAlignment);
                auto innerArrayType = builder.getArrayType(
                    loweredInnerTypeInfo.loweredType,
                    arrayType->getElementCount(),
                    builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride()));
                builder.createStructField(loweredType, structKey, innerArrayType);
                info.loweredInnerArrayType = innerArrayType;
                info.loweredInnerStructKey = structKey;
                info.convertLoweredToOriginal = createArrayUnpackFunc(arrayType, loweredType, structKey, innerArrayType, loweredInnerTypeInfo);
                info.convertOriginalToLowered = createArrayPackFunc(arrayType, loweredType, innerArrayType, loweredInnerTypeInfo);

                return info;
            }
            else if (auto structType = as<IRStructType>(type))
            {
                List<LoweredElementTypeInfo> fieldLoweredTypeInfo;
                bool isTrivial = true;
                for (auto field : structType->getFields())
                {
                    auto loweredFieldTypeInfo = getLoweredTypeInfo(field->getFieldType(), rules);
                    fieldLoweredTypeInfo.add(loweredFieldTypeInfo);
                    if (loweredFieldTypeInfo.convertLoweredToOriginal || rules->ruleName != IRTypeLayoutRuleName::Natural)
                        isTrivial = false;
                }

                // For spirv backend, we always want to lower all array types, even if the element type
                // comes out the same. This is because different layout rules may have different array
                // stride requirements.
                if (!target->shouldEmitSPIRVDirectly())
                {
                    // For non-spirv target, we skip lowering this type if all field types are unchanged.
                    if (isTrivial)
                    {
                        info.loweredType = type;
                        return info;
                    }
                }
                auto loweredType = builder.createStructType();
                StringBuilder nameSB;
                getTypeNameHint(nameSB, type);
                nameSB << "_" << getLayoutName(rules->ruleName);
                builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
                info.loweredType = loweredType;
                // Create fields.
                {
                    Index fieldId = 0;
                    for (auto field : structType->getFields())
                    {
                        auto loweredFieldTypeInfo = fieldLoweredTypeInfo[fieldId];
                        builder.createStructField(loweredType, field->getKey(), loweredFieldTypeInfo.loweredType);
                        fieldId++;
                    }
                }

                // Create unpack func.
                {
                    builder.setInsertAfter(loweredType);
                    info.convertLoweredToOriginal = builder.createFunc();
                    builder.setInsertInto(info.convertLoweredToOriginal);
                    builder.addNameHintDecoration(info.convertLoweredToOriginal, UnownedStringSlice("unpackStorage"));
                    info.convertLoweredToOriginal->setFullType(builder.getFuncType(1, (IRType**)&loweredType, type));
                    builder.emitBlock();
                    auto loweredParam = builder.emitParam(loweredType);
                    List<IRInst*> args;
                    Index fieldId = 0;
                    for (auto field : structType->getFields())
                    {
                        auto storageField = builder.emitFieldExtract(fieldLoweredTypeInfo[fieldId].loweredType, loweredParam, field->getKey());
                        auto unpackedField = fieldLoweredTypeInfo[fieldId].convertLoweredToOriginal
                            ? builder.emitCallInst(field->getFieldType(), fieldLoweredTypeInfo[fieldId].convertLoweredToOriginal, 1, &storageField)
                            : storageField;
                        args.add(unpackedField);
                        fieldId++;
                    }
                    auto result = builder.emitMakeStruct(type, args);
                    builder.emitReturn(result);
                }

                // Create pack func.
                {
                    builder.setInsertAfter(info.convertLoweredToOriginal);
                    info.convertOriginalToLowered = builder.createFunc();
                    builder.setInsertInto(info.convertOriginalToLowered);
                    builder.addNameHintDecoration(info.convertOriginalToLowered, UnownedStringSlice("packStorage"));
                    info.convertOriginalToLowered->setFullType(builder.getFuncType(1, (IRType**)&type, loweredType));
                    builder.emitBlock();
                    auto param = builder.emitParam(type);
                    List<IRInst*> args;
                    Index fieldId = 0;
                    for (auto field : structType->getFields())
                    {
                        auto fieldVal = builder.emitFieldExtract(field->getFieldType(), param, field->getKey());
                        auto packedField = fieldLoweredTypeInfo[fieldId].convertOriginalToLowered
                            ? builder.emitCallInst(fieldLoweredTypeInfo[fieldId].loweredType, fieldLoweredTypeInfo[fieldId].convertOriginalToLowered, 1, &fieldVal)
                            : fieldVal;
                        args.add(packedField);
                        fieldId++;
                    }
                    auto result = builder.emitMakeStruct(loweredType, args);
                    builder.emitReturn(result);
                }

                return info;
            }

            if (target->shouldEmitSPIRVDirectly())
            {
                switch (target->getTarget())
                {
                case CodeGenTarget::SPIRV:
                case CodeGenTarget::SPIRVAssembly:
                {
                    auto scalarType = type;
                    auto vectorType = as<IRVectorType>(scalarType);
                    if (vectorType)
                        scalarType = vectorType->getElementType();

                    if (as<IRBoolType>(scalarType))
                    {
                        // Bool is an abstract type in SPIRV, so we need to lower them into an int.
                        info.loweredType = builder.getIntType();
                        if (vectorType)
                            info.loweredType = builder.getVectorType(info.loweredType, vectorType->getElementCount());
                        // Create unpack func.
                        {
                            builder.setInsertAfter(type);
                            info.convertLoweredToOriginal = builder.createFunc();
                            builder.setInsertInto(info.convertLoweredToOriginal);
                            builder.addNameHintDecoration(info.convertLoweredToOriginal, UnownedStringSlice("unpackStorage"));
                            info.convertLoweredToOriginal->setFullType(builder.getFuncType(1, (IRType**)&info.loweredType, type));
                            builder.emitBlock();
                            auto loweredParam = builder.emitParam(info.loweredType);
                            auto result = builder.emitCast(type, loweredParam);
                            builder.emitReturn(result);
                        }

                        // Create pack func.
                        {
                            builder.setInsertAfter(info.convertLoweredToOriginal);
                            info.convertOriginalToLowered = builder.createFunc();
                            builder.setInsertInto(info.convertOriginalToLowered);
                            builder.addNameHintDecoration(info.convertOriginalToLowered, UnownedStringSlice("packStorage"));
                            info.convertOriginalToLowered->setFullType(builder.getFuncType(1, (IRType**)&type, info.loweredType));
                            builder.emitBlock();
                            auto param = builder.emitParam(type);
                            auto result = builder.emitCast(info.loweredType, param);
                            builder.emitReturn(result);
                        }
                        return info;
                    }
                }
                default:
                    break;
                }
            }

            info.loweredType = type;
            return info;
        }

        LoweredElementTypeInfo getLoweredTypeInfo(IRType* type, IRTypeLayoutRules* rules)
        {
            LoweredElementTypeInfo info;
            if (loweredTypeInfo[(int)rules->ruleName].tryGetValue(type, info))
                return info;
            info = getLoweredTypeInfoImpl(type, rules);
            IRSizeAndAlignment sizeAlignment;
            getSizeAndAlignment(target, rules, info.loweredType, &sizeAlignment);
            loweredTypeInfo[(int)rules->ruleName].set(type, info);
            mapLoweredTypeToInfo[(int)rules->ruleName].set(info.loweredType, info);
            return info;
        }

        IRType* getLoweredPtrLikeType(IRType* originalPtrLikeType, IRType* newElementType)
        {
            if (as<IRPointerLikeType>(originalPtrLikeType) || as<IRPtrTypeBase>(originalPtrLikeType) || as<IRHLSLStructuredBufferTypeBase>(originalPtrLikeType))
            {
                IRBuilder builder(newElementType);
                builder.setInsertAfter(newElementType);
                return builder.getType(originalPtrLikeType->getOp(), newElementType);
            }
            SLANG_UNREACHABLE("unhandled ptr like or buffer type");
        }

        IRInst* getStoreVal(IRInst* storeInst)
        {
            if (auto store = as<IRStore>(storeInst))
                return store->getVal();
            else if (auto sbStore = as<IRRWStructuredBufferStore>(storeInst))
                return sbStore->getVal();
            return nullptr;
        }

        struct MatrixAddrWorkItem
        {
            IRInst* matrixAddrInst;
            IRTypeLayoutRules* layoutRules;
        };

        void processModule(IRModule* module)
        {
            IRBuilder builder(module);
            struct BufferTypeInfo
            {
                IRType* bufferType;
                IRType* elementType;
            };
            List<BufferTypeInfo> bufferTypeInsts;
            for (auto globalInst : module->getGlobalInsts())
            {
                IRType* elementType = nullptr;
                if (auto structBuffer = as<IRHLSLStructuredBufferTypeBase>(globalInst))
                    elementType = structBuffer->getElementType();
                else if (auto constBuffer = as<IRUniformParameterGroupType>(globalInst))
                    elementType = constBuffer->getElementType();
                if (as<IRTextureBufferType>(globalInst))
                    continue;
                if (!as<IRStructType>(elementType) && !as<IRMatrixType>(elementType) && !as<IRArrayType>(elementType))
                    continue;
                bufferTypeInsts.add(BufferTypeInfo{ (IRType*)globalInst, elementType });
            }

            // Maintain a pending work list of all matrix addresses, and try to lower them out of existance
            // after everything else has been lowered.
            
            List<MatrixAddrWorkItem> matrixAddrInsts;

            for (auto bufferTypeInfo : bufferTypeInsts)
            {
                auto bufferType = bufferTypeInfo.bufferType;
                auto elementType = bufferTypeInfo.elementType;
                auto layoutRules = getTypeLayoutRuleForBuffer(target, bufferType);
                auto loweredBufferElementTypeInfo = getLoweredTypeInfo(elementType, layoutRules);

                // If the lowered type is the same as original type, no change is required.
                if (!loweredBufferElementTypeInfo.convertLoweredToOriginal)
                    continue;

                builder.setInsertBefore(bufferType);

                auto loweredBufferType = builder.getType(
                    bufferType->getOp(),
                    loweredBufferElementTypeInfo.loweredType);

                // We treat a value of a buffer type as a pointer, and use a work list to translate
                // all loads and stores through the pointer values that needs lowering.

                List<IRInst*> ptrValsWorkList;
                traverseUses(bufferType, [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        if (use != &user->typeUse)
                            return;
                        ptrValsWorkList.add(use->getUser());
                    });

                // Translate the values to use new lowered buffer type instead.
                for (Index i = 0; i < ptrValsWorkList.getCount(); i++)
                {
                    auto ptrVal = ptrValsWorkList[i];
                    auto oldPtrType = ptrVal->getFullType();
                    auto originalElementType = oldPtrType->getOperand(0);
                    auto loweredElementTypeInfo = getLoweredTypeInfo((IRType*)originalElementType, layoutRules);
                    if (!loweredElementTypeInfo.convertLoweredToOriginal)
                        continue;

                    ptrVal->setFullType(getLoweredPtrLikeType(ptrVal->getFullType(), loweredElementTypeInfo.loweredType));

                    traverseUses(ptrVal, [&](IRUse* use)
                        {
                            auto user = use->getUser();
                            if (as<IRDecoration>(user))
                                return;
                            switch (user->getOp())
                            {
                            case kIROp_Load:
                            case kIROp_StructuredBufferLoad:
                            case kIROp_StructuredBufferLoadStatus:
                            case kIROp_RWStructuredBufferLoad:
                            case kIROp_RWStructuredBufferLoadStatus:
                            case kIROp_StructuredBufferConsume:
                                {
                                    IRCloneEnv cloneEnv = {};
                                    builder.setInsertBefore(user);
                                    auto newLoad = cloneInst(&cloneEnv, &builder, user);
                                    newLoad->setFullType(loweredElementTypeInfo.loweredType);
                                    auto unpackedVal = builder.emitCallInst((IRType*)originalElementType, loweredElementTypeInfo.convertLoweredToOriginal, 1, &newLoad);
                                    user->replaceUsesWith(unpackedVal);
                                    user->removeAndDeallocate();
                                    break;
                                }
                            case kIROp_Store:
                            case kIROp_RWStructuredBufferStore:
                            case kIROp_StructuredBufferAppend:
                                {
                                    // Use must be the dest operand of the store inst.
                                    if (use != user->getOperands() + 0)
                                        break;
                                    IRCloneEnv cloneEnv = {};
                                    builder.setInsertBefore(user);
                                    auto originalVal = getStoreVal(user);
                                    auto packedVal = builder.emitCallInst(loweredElementTypeInfo.loweredType, loweredElementTypeInfo.convertOriginalToLowered, 1, &originalVal);
                                    if (auto store = as<IRStore>(user))
                                        store->val.set(packedVal);
                                    else if (auto sbStore = as<IRRWStructuredBufferStore>(user))
                                        sbStore->setOperand(2, packedVal);
                                    else if (auto sbAppend = as<IRStructuredBufferAppend>(user))
                                        sbAppend->setOperand(1, packedVal);
                                    else
                                        SLANG_UNREACHABLE("unhandled store type");
                                    break;
                                }
                            case kIROp_GetElementPtr:
                            case kIROp_FieldAddress:
                                {
                                    // If original type is an array, the lowered type will be a struct.
                                    // In that case, all existing address insts should be appended with a field extract.
                                    if (as<IRArrayType>(originalElementType))
                                    {
                                        builder.setInsertBefore(user);
                                        List<IRInst*> args;
                                        for (UInt i = 0; i < user->getOperandCount(); i++)
                                            args.add(user->getOperand(i));
                                        auto newArrayPtrVal = builder.emitFieldAddress(
                                            builder.getPtrType(loweredElementTypeInfo.loweredInnerArrayType),
                                            ptrVal,
                                            loweredElementTypeInfo.loweredInnerStructKey);
                                        builder.replaceOperand(use, newArrayPtrVal);
                                        ptrValsWorkList.add(user);
                                    }
                                    else if (as<IRMatrixType>(originalElementType))
                                    {
                                        // We are tring to get a pointer to a lowered matrix element.
                                        // We process this insts at a later phase.
                                        SLANG_ASSERT(user->getOp() == kIROp_GetElementPtr);
                                        matrixAddrInsts.add(MatrixAddrWorkItem{ user, layoutRules });
                                    }
                                    else
                                    {
                                        // If we getting a derived address from the pointer, we need to recursively
                                        // lower the new address. We do so by pushing the address inst into the
                                        // work list.
                                        ptrValsWorkList.add(user);
                                    }
                                }
                                break;
                            case kIROp_RWStructuredBufferGetElementPtr:
                                ptrValsWorkList.add(user);
                                break;
                            case kIROp_StructuredBufferGetDimensions:
                                break;
                            case kIROp_Call:
                                {
                                    // If a structured buffer typed value is used directly as an argument,
                                    // we don't need to do any marshalling here.
                                    if (as<IRHLSLStructuredBufferTypeBase>(ptrVal->getDataType()))
                                        break;

                                    // If we are calling a function with an l-value pointer from buffer access,
                                    // we need to materialize the object as a local variable, and pass the address
                                    // of the local variable to the function.
                                    builder.setInsertBefore(user);
                                    auto newLoad = builder.emitLoad(loweredElementTypeInfo.loweredType, ptrVal);
                                    auto unpackedVal = builder.emitCallInst((IRType*)originalElementType, loweredElementTypeInfo.convertLoweredToOriginal, 1, &newLoad);
                                    auto var = builder.emitVar((IRType*)originalElementType);
                                    builder.emitStore(var, unpackedVal);
                                    use->set(var);
                                    builder.setInsertAfter(user);
                                    auto newVal = builder.emitLoad(var);
                                    auto packedVal = builder.emitCallInst((IRType*)loweredElementTypeInfo.loweredType, loweredElementTypeInfo.convertOriginalToLowered, 1, &newVal);
                                    builder.emitStore(ptrVal, packedVal);
                                }
                                break;
                            default:
                                SLANG_UNREACHABLE("unhandled inst of a buffer/pointer value that needs storage lowering.");
                                break;
                            }
                        });
                }

                // Replace all remaining uses of bufferType to loweredBufferType, these uses are non-operational and should be
                // directly replaceable, such as uses in `IRFuncType`.
                bufferType->replaceUsesWith(loweredBufferType);
                bufferType->removeAndDeallocate();
            }

            // Process all matrix address uses.
            lowerMatrixAddresses(module, matrixAddrInsts);
        }

        // Lower all getElementPtr insts of a lowered matrix out of existance.
        void lowerMatrixAddresses(IRModule* module, List<MatrixAddrWorkItem>& matrixAddrInsts)
        {
            IRBuilder builder(module);
            for (auto workItem : matrixAddrInsts)
            {
                auto majorAddr = workItem.matrixAddrInst;
                auto layoutRules = workItem.layoutRules;

                int layoutRuleName = (int)layoutRules->ruleName;
                auto majorGEP = as<IRGetElementPtr>(majorAddr);
                SLANG_ASSERT(majorGEP);
                auto loweredMatrixType = cast<IRPtrTypeBase>(majorGEP->getBase()->getFullType())->getValueType();
                auto matrixTypeInfo = mapLoweredTypeToInfo[layoutRuleName].tryGetValue(loweredMatrixType);
                SLANG_ASSERT(matrixTypeInfo);
                auto matrixType = as<IRMatrixType>(matrixTypeInfo->originalType);
                auto rowCount = getIntVal(matrixType->getRowCount());
                traverseUses(majorAddr, [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        builder.setInsertBefore(user);
                        switch (user->getOp())
                        {
                        case kIROp_Load:
                            {
                                IRInst* resultInst = nullptr;
                                auto dataPtr = builder.emitFieldAddress(
                                    builder.getPtrType(matrixTypeInfo->loweredInnerArrayType),
                                    majorGEP->getBase(),
                                    matrixTypeInfo->loweredInnerStructKey);
                                if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                                {
                                    List<IRInst*> args;
                                    for (IRIntegerValue i = 0; i < rowCount; i++)
                                    {
                                        auto vector = builder.emitLoad(builder.emitElementAddress(dataPtr, i));
                                        auto element = builder.emitElementExtract(vector, majorGEP->getIndex());
                                        args.add(element);
                                    }
                                    resultInst = builder.emitMakeVector(builder.getVectorType(matrixType->getElementType(), (IRIntegerValue)args.getCount()), args);
                                }
                                else
                                {
                                    auto element = builder.emitElementAddress(dataPtr, majorGEP->getIndex());
                                    resultInst = builder.emitLoad(element);
                                }
                                user->replaceUsesWith(resultInst);
                                user->removeAndDeallocate();
                            }
                            break;
                        case kIROp_Store:
                            {
                                auto storeInst = cast<IRStore>(user);
                                if (storeInst->getOperand(0) != majorAddr)
                                    break;
                                auto dataPtr = builder.emitFieldAddress(
                                    builder.getPtrType(matrixTypeInfo->loweredInnerArrayType),
                                    majorGEP->getBase(),
                                    matrixTypeInfo->loweredInnerStructKey);
                                if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                                {
                                    for (IRIntegerValue i = 0; i < rowCount; i++)
                                    {
                                        auto vectorAddr = builder.emitElementAddress(dataPtr, i);
                                        auto elementAddr = builder.emitElementAddress(vectorAddr, majorGEP->getIndex());
                                        builder.emitStore(elementAddr, builder.emitElementExtract(storeInst->getVal(), i));
                                    }
                                }
                                else
                                {
                                    auto rowAddr = builder.emitElementAddress(dataPtr, majorGEP->getIndex());
                                    builder.emitStore(rowAddr, storeInst->getVal());
                                    user->removeAndDeallocate();
                                }
                                break;
                            }
                        case kIROp_GetElementPtr:
                            {
                                auto gep2 = cast<IRGetElementPtr>(user);
                                auto rowIndex = majorGEP->getIndex();
                                auto colIndex = gep2->getIndex();
                                if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                                {
                                    Swap(rowIndex, colIndex);
                                }
                                auto dataPtr = builder.emitFieldAddress(
                                    builder.getPtrType(matrixTypeInfo->loweredInnerArrayType),
                                    majorGEP->getBase(),
                                    matrixTypeInfo->loweredInnerStructKey);
                                auto vectorAddr = builder.emitElementAddress(dataPtr, rowIndex);
                                auto elementAddr = builder.emitElementAddress(vectorAddr, colIndex);
                                gep2->replaceUsesWith(elementAddr);
                                gep2->removeAndDeallocate();
                                break;
                            }
                        default:
                            SLANG_UNREACHABLE("unhandled inst of a matrix address inst that needs storage lowering.");
                            break;
                        }
                    });
            }
        }
    };

    void lowerBufferElementTypeToStorageType(TargetRequest* target, IRModule* module)
    {
        SlangMatrixLayoutMode defaultMatrixMode = (SlangMatrixLayoutMode)target->getDefaultMatrixLayoutMode();
        if (defaultMatrixMode == SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
            defaultMatrixMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
        LoweredElementTypeContext context(target, defaultMatrixMode);
        context.processModule(module);
    }


    IRTypeLayoutRules* getTypeLayoutRuleForBuffer(TargetRequest* target, IRType* bufferType)
    {
        if (!isKhronosTarget(target))
            return IRTypeLayoutRules::getNatural();

        // If we are just emitting GLSL, we can just use the general layout rule.
        if (!target->shouldEmitSPIRVDirectly())
            return IRTypeLayoutRules::getNatural();

        // If the user specified a scalar buffer layout, then just use that.
        if (target->getForceGLSLScalarBufferLayout())
            return IRTypeLayoutRules::getNatural();

        // The default behavior is to use std140 for constant buffers and std430 for other buffers.
        switch (bufferType->getOp())
        {
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_HLSLAppendStructuredBufferType:
        case kIROp_HLSLConsumeStructuredBufferType:
        case kIROp_HLSLRasterizerOrderedStructuredBufferType:
            return IRTypeLayoutRules::getStd430();
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
            return IRTypeLayoutRules::getStd140();
        }
        return IRTypeLayoutRules::getNatural();
    }

}
