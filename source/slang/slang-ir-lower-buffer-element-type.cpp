#include "slang-ir-lower-buffer-element-type.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct LoweredElementTypeContext
{
    static const IRIntegerValue kMaxArraySizeToUnroll = 32;

    enum ConversionMethodKind
    {
        Func,
        Opcode
    };
    struct ConversionMethod
    {
        ConversionMethodKind kind = ConversionMethodKind::Func;
        union
        {
            IRFunc* func;
            IROp op;
        };
        ConversionMethod() { func = nullptr; }
        operator bool()
        {
            return kind == ConversionMethodKind::Func ? func != nullptr : op != kIROp_Nop;
        }
        ConversionMethod& operator=(IRFunc* f)
        {
            kind = ConversionMethodKind::Func;
            this->func = f;
            return *this;
        }
        ConversionMethod& operator=(IROp irop)
        {
            kind = ConversionMethodKind::Opcode;
            this->op = irop;
            return *this;
        }
        IRInst* apply(IRBuilder& builder, IRType* resultType, IRInst* operand)
        {
            if (!*this)
                return operand;
            if (kind == ConversionMethodKind::Func)
                return builder.emitCallInst(resultType, func, 1, &operand);
            else
                return builder.emitIntrinsicInst(resultType, op, 1, &operand);
        }
    };

    struct LoweredElementTypeInfo
    {
        IRType* originalType;
        IRType* loweredType;
        IRType* loweredInnerArrayType =
            nullptr; // For matrix/array types that are lowered into a struct type, this is the
                     // inner array type of the data field.
        IRStructKey* loweredInnerStructKey =
            nullptr; // For matrix/array types that are lowered into a struct type, this is the
                     // struct key of the data field.
        ConversionMethod convertOriginalToLowered;
        ConversionMethod convertLoweredToOriginal;
    };

    Dictionary<IRType*, LoweredElementTypeInfo> loweredTypeInfo[(int)IRTypeLayoutRuleName::_Count];
    Dictionary<IRType*, LoweredElementTypeInfo>
        mapLoweredTypeToInfo[(int)IRTypeLayoutRuleName::_Count];

    struct ConversionMethodKey
    {
        IRType* toType;
        IRType* fromType;
        bool operator==(const ConversionMethodKey& other) const
        {
            return toType == other.toType && fromType == other.fromType;
        }
        HashCode64 getHashCode() const
        {
            return combineHash(Slang::getHashCode(toType), Slang::getHashCode(fromType));
        }
    };

    Dictionary<ConversionMethodKey, ConversionMethod> conversionMethodMap;
    ConversionMethod getConversionMethod(IRType* toType, IRType* fromType)
    {
        ConversionMethodKey key;
        key.toType = toType;
        key.fromType = fromType;
        ConversionMethod method;
        conversionMethodMap.tryGetValue(key, method);
        return method;
    }

    SlangMatrixLayoutMode defaultMatrixLayout = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    TargetProgram* target;
    BufferElementTypeLoweringOptions options;

    LoweredElementTypeContext(
        TargetProgram* target,
        BufferElementTypeLoweringOptions inOptions,
        SlangMatrixLayoutMode inDefaultMatrixLayout)
        : target(target), defaultMatrixLayout(inDefaultMatrixLayout), options(inOptions)
    {
    }

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
                    args[(Index)(r * colCount + c)] = element;
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
        IRInst* result =
            builder.emitMakeMatrix(matrixType, (UInt)args.getCount(), args.getBuffer());
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
                // Fill in default values for remaining elements in the vector.
                for (IRIntegerValue r = rowCount; r < getIntVal(vectorType->getElementCount()); r++)
                {
                    vecArgs.add(builder.emitDefaultConstruct(vectorType->getElementType()));
                }
                auto colVector = builder.emitMakeVector(
                    vectorType,
                    (UInt)vecArgs.getCount(),
                    vecArgs.getBuffer());
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
                // Fill in default values for remaining elements in the vector.
                for (IRIntegerValue c = colCount; c < getIntVal(vectorType->getElementCount()); c++)
                {
                    vecArgs.add(builder.emitDefaultConstruct(vectorType->getElementType()));
                }
                auto rowVector = builder.emitMakeVector(
                    vectorType,
                    (UInt)vecArgs.getCount(),
                    vecArgs.getBuffer());
                vectors.add(rowVector);
            }
        }

        auto vectorArray =
            builder.emitMakeArray(arrayType, (UInt)vectors.getCount(), vectors.getBuffer());
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
        IRInst* result = nullptr;
        if (count <= kMaxArraySizeToUnroll)
        {
            // If the array is small enough, just process each element directly.
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto packedElement = builder.emitElementExtract(packedArray, ii);
                auto originalElement = innerTypeInfo.convertLoweredToOriginal.apply(
                    builder,
                    innerTypeInfo.originalType,
                    packedElement);
                args[(Index)ii] = originalElement;
            }
            result = builder.emitMakeArray(arrayType, (UInt)args.getCount(), args.getBuffer());
        }
        else
        {
            // The general case for large arrays is to emit a loop through the elements.
            IRVar* resultVar = builder.emitVar(arrayType);
            IRBlock* loopBodyBlock;
            IRBlock* loopBreakBlock;
            auto loopParam = emitLoopBlocks(
                &builder,
                builder.getIntValue(builder.getIntType(), 0),
                builder.getIntValue(builder.getIntType(), count),
                loopBodyBlock,
                loopBreakBlock);

            builder.setInsertBefore(loopBodyBlock->getFirstOrdinaryInst());
            auto packedElement = builder.emitElementExtract(packedArray, loopParam);
            auto originalElement = innerTypeInfo.convertLoweredToOriginal.apply(
                builder,
                innerTypeInfo.originalType,
                packedElement);
            auto varPtr = builder.emitElementAddress(resultVar, loopParam);
            builder.emitStore(varPtr, originalElement);
            builder.setInsertInto(loopBreakBlock);
            result = builder.emitLoad(resultVar);
        }
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
        IRInst* packedArray = nullptr;
        auto count = getIntVal(arrayType->getElementCount());
        if (count <= kMaxArraySizeToUnroll)
        {
            // If the array is small enough, just process each element directly.
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto originalElement = builder.emitElementExtract(originalParam, ii);
                auto packedElement = innerTypeInfo.convertOriginalToLowered.apply(
                    builder,
                    innerTypeInfo.loweredType,
                    originalElement);
                args[(Index)ii] = packedElement;
            }
            packedArray =
                builder.emitMakeArray(innerArrayType, (UInt)args.getCount(), args.getBuffer());
        }
        else
        {
            // The general case for large arrays is to emit a loop through the elements.
            IRVar* packedArrayVar = builder.emitVar(innerArrayType);
            IRBlock* loopBodyBlock;
            IRBlock* loopBreakBlock;
            auto loopParam = emitLoopBlocks(
                &builder,
                builder.getIntValue(builder.getIntType(), 0),
                builder.getIntValue(builder.getIntType(), count),
                loopBodyBlock,
                loopBreakBlock);

            builder.setInsertBefore(loopBodyBlock->getFirstOrdinaryInst());
            auto originalElement = builder.emitElementExtract(originalParam, loopParam);
            auto packedElement = innerTypeInfo.convertOriginalToLowered.apply(
                builder,
                innerTypeInfo.loweredType,
                originalElement);
            auto varPtr = builder.emitElementAddress(packedArrayVar, loopParam);
            builder.emitStore(varPtr, packedElement);
            builder.setInsertInto(loopBreakBlock);
            packedArray = builder.emitLoad(packedArrayVar);
        }

        auto result = builder.emitMakeStruct(structType, 1, &packedArray);
        builder.emitReturn(result);
        return func;
    }

    const char* getLayoutName(IRTypeLayoutRuleName name)
    {
        switch (name)
        {
        case IRTypeLayoutRuleName::Std140:
            return "std140";
        case IRTypeLayoutRuleName::Std430:
            return "std430";
        case IRTypeLayoutRuleName::Natural:
            return "natural";
        default:
            return "default";
        }
    }

    // Returns the number of elements N that ensures the IRVectorType(elementType,N)
    // has 16-byte aligned size and N is no less than `minCount`.
    IRIntegerValue get16ByteAlignedVectorElementCount(IRType* elementType, IRIntegerValue minCount)
    {
        IRSizeAndAlignment sizeAlignment;
        getNaturalSizeAndAlignment(target->getOptionSet(), elementType, &sizeAlignment);
        if (sizeAlignment.size)
            return align(sizeAlignment.size * minCount, 16) / sizeAlignment.size;
        return 4;
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
            bool isColMajor =
                getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
            nameSB << "_MatrixStorage_";
            getTypeNameHint(nameSB, matrixType->getElementType());
            nameSB << getIntVal(matrixType->getRowCount()) << "x"
                   << getIntVal(matrixType->getColumnCount());
            if (isColMajor)
                nameSB << "_ColMajor";
            nameSB << getLayoutName(rules->ruleName);
            builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
            auto structKey = builder.createStructKey();
            builder.addNameHintDecoration(structKey, UnownedStringSlice("data"));
            auto vectorSize = isColMajor ? matrixType->getRowCount() : matrixType->getColumnCount();
            if (rules->ruleName == IRTypeLayoutRuleName::Std140 &&
                options.use16ByteArrayElementForConstantBuffer)
            {
                // For constant buffer layout, we need to use 16-byte aligned vector if
                // we are required to ensure array element types has 16-byte stride.
                vectorSize = builder.getIntValue(get16ByteAlignedVectorElementCount(
                    matrixType->getElementType(),
                    getIntVal(vectorSize)));
            }

            auto vectorType = builder.getVectorType(matrixType->getElementType(), vectorSize);
            IRSizeAndAlignment elementSizeAlignment;
            getSizeAndAlignment(target->getOptionSet(), rules, vectorType, &elementSizeAlignment);
            elementSizeAlignment = rules->alignCompositeElement(elementSizeAlignment);

            auto arrayType = builder.getArrayType(
                vectorType,
                isColMajor ? matrixType->getColumnCount() : matrixType->getRowCount(),
                builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride()));
            builder.createStructField(loweredType, structKey, arrayType);

            info.loweredType = loweredType;
            info.loweredInnerArrayType = arrayType;
            info.loweredInnerStructKey = structKey;
            info.convertLoweredToOriginal =
                createMatrixUnpackFunc(matrixType, loweredType, structKey, arrayType);
            info.convertOriginalToLowered =
                createMatrixPackFunc(matrixType, loweredType, vectorType, arrayType);
            return info;
        }
        else if (auto arrayType = as<IRArrayType>(type))
        {
            auto loweredInnerTypeInfo = getLoweredTypeInfo(arrayType->getElementType(), rules);

            if (rules->ruleName == IRTypeLayoutRuleName::Std140 &&
                options.use16ByteArrayElementForConstantBuffer)
            {
                // For constant buffer layout, we need to use 16-byte-aligned vector if
                // we are required to ensure array element types has 16-byte stride.
                // We only need to handle the case where the element type is a scalar or vector
                // type here, because if the element type is a matrix type or struct type,
                // the size promotion will be handled during lowering of the element type.
                IRType* packedVectorType = nullptr;
                if (auto vectorType = as<IRVectorType>(loweredInnerTypeInfo.loweredType))
                {
                    packedVectorType = builder.getVectorType(
                        vectorType->getElementType(),
                        builder.getIntValue(get16ByteAlignedVectorElementCount(
                            vectorType->getElementType(),
                            getIntVal(vectorType->getElementCount()))));
                    if (packedVectorType != loweredInnerTypeInfo.originalType)
                    {
                        loweredInnerTypeInfo.convertLoweredToOriginal = kIROp_VectorReshape;
                        loweredInnerTypeInfo.convertOriginalToLowered = kIROp_VectorReshape;
                    }
                }
                else if (auto scalarType = as<IRBasicType>(loweredInnerTypeInfo.loweredType))
                {
                    packedVectorType = builder.getVectorType(
                        loweredInnerTypeInfo.loweredType,
                        get16ByteAlignedVectorElementCount(scalarType, 1));
                    loweredInnerTypeInfo.convertLoweredToOriginal = kIROp_VectorReshape;
                    loweredInnerTypeInfo.convertOriginalToLowered = kIROp_MakeVectorFromScalar;
                }
                if (packedVectorType)
                {
                    loweredInnerTypeInfo.loweredType = packedVectorType;
                    if (loweredInnerTypeInfo.convertLoweredToOriginal)
                        conversionMethodMap[ConversionMethodKey{
                            packedVectorType,
                            loweredInnerTypeInfo.originalType}] =
                            loweredInnerTypeInfo.convertOriginalToLowered;
                    if (loweredInnerTypeInfo.convertOriginalToLowered)
                        conversionMethodMap[ConversionMethodKey{
                            loweredInnerTypeInfo.originalType,
                            packedVectorType}] = loweredInnerTypeInfo.convertLoweredToOriginal;
                }
            }

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
            getSizeAndAlignment(
                target->getOptionSet(),
                rules,
                loweredInnerTypeInfo.loweredType,
                &elementSizeAlignment);
            elementSizeAlignment = rules->alignCompositeElement(elementSizeAlignment);
            auto innerArrayType = builder.getArrayType(
                loweredInnerTypeInfo.loweredType,
                arrayType->getElementCount(),
                builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride()));
            builder.createStructField(loweredType, structKey, innerArrayType);
            info.loweredInnerArrayType = innerArrayType;
            info.loweredInnerStructKey = structKey;
            info.convertLoweredToOriginal = createArrayUnpackFunc(
                arrayType,
                loweredType,
                structKey,
                innerArrayType,
                loweredInnerTypeInfo);
            info.convertOriginalToLowered =
                createArrayPackFunc(arrayType, loweredType, innerArrayType, loweredInnerTypeInfo);
            return info;
        }
        else if (as<IRArrayTypeBase>(type))
        {
            info.loweredType = builder.getVoidType();
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
                if (loweredFieldTypeInfo.convertLoweredToOriginal ||
                    rules->ruleName != IRTypeLayoutRuleName::Natural)
                    isTrivial = false;
            }

            // For spirv backend, we always want to lower all array types, even if the element type
            // comes out the same. This is because different layout rules may have different array
            // stride requirements.
            if (!target->shouldEmitSPIRVDirectly())
            {
                // For non-spirv target, we skip lowering this type if all field types are
                // unchanged.
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
                    if (as<IRVoidType>(fieldLoweredTypeInfo[fieldId].loweredType))
                    {
                        fieldId++;
                        continue;
                    }
                    auto loweredFieldTypeInfo = fieldLoweredTypeInfo[fieldId];
                    builder.createStructField(
                        loweredType,
                        field->getKey(),
                        loweredFieldTypeInfo.loweredType);
                    fieldId++;
                }
            }

            // Create unpack func.
            {
                builder.setInsertAfter(loweredType);
                info.convertLoweredToOriginal = builder.createFunc();
                builder.setInsertInto(info.convertLoweredToOriginal.func);
                builder.addNameHintDecoration(
                    info.convertLoweredToOriginal.func,
                    UnownedStringSlice("unpackStorage"));
                info.convertLoweredToOriginal.func->setFullType(
                    builder.getFuncType(1, (IRType**)&loweredType, type));
                builder.emitBlock();
                auto loweredParam = builder.emitParam(loweredType);
                List<IRInst*> args;
                Index fieldId = 0;
                for (auto field : structType->getFields())
                {
                    if (as<IRVoidType>(fieldLoweredTypeInfo[fieldId].loweredType))
                    {
                        fieldId++;
                        continue;
                    }
                    auto storageField = builder.emitFieldExtract(
                        fieldLoweredTypeInfo[fieldId].loweredType,
                        loweredParam,
                        field->getKey());
                    auto unpackedField =
                        fieldLoweredTypeInfo[fieldId].convertLoweredToOriginal.apply(
                            builder,
                            field->getFieldType(),
                            storageField);
                    args.add(unpackedField);
                    fieldId++;
                }
                auto result = builder.emitMakeStruct(type, args);
                builder.emitReturn(result);
            }

            // Create pack func.
            {
                builder.setInsertAfter(info.convertLoweredToOriginal.func);
                info.convertOriginalToLowered = builder.createFunc();
                builder.setInsertInto(info.convertOriginalToLowered.func);
                builder.addNameHintDecoration(
                    info.convertOriginalToLowered.func,
                    UnownedStringSlice("packStorage"));
                info.convertOriginalToLowered.func->setFullType(
                    builder.getFuncType(1, (IRType**)&type, loweredType));
                builder.emitBlock();
                auto param = builder.emitParam(type);
                List<IRInst*> args;
                Index fieldId = 0;
                for (auto field : structType->getFields())
                {
                    if (as<IRVoidType>(fieldLoweredTypeInfo[fieldId].loweredType))
                    {
                        fieldId++;
                        continue;
                    }
                    auto fieldVal =
                        builder.emitFieldExtract(field->getFieldType(), param, field->getKey());
                    auto packedField = fieldLoweredTypeInfo[fieldId].convertOriginalToLowered.apply(
                        builder,
                        fieldLoweredTypeInfo[fieldId].loweredType,
                        fieldVal);
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
            switch (target->getTargetReq()->getTarget())
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
                            info.loweredType = builder.getVectorType(
                                info.loweredType,
                                vectorType->getElementCount());
                        // Create unpack func.
                        {
                            builder.setInsertAfter(type);
                            info.convertLoweredToOriginal = builder.createFunc();
                            builder.setInsertInto(info.convertLoweredToOriginal.func);
                            builder.addNameHintDecoration(
                                info.convertLoweredToOriginal.func,
                                UnownedStringSlice("unpackStorage"));
                            info.convertLoweredToOriginal.func->setFullType(
                                builder.getFuncType(1, (IRType**)&info.loweredType, type));
                            builder.emitBlock();
                            auto loweredParam = builder.emitParam(info.loweredType);
                            auto result = builder.emitCast(type, loweredParam);
                            builder.emitReturn(result);
                        }

                        // Create pack func.
                        {
                            builder.setInsertAfter(info.convertLoweredToOriginal.func);
                            info.convertOriginalToLowered = builder.createFunc();
                            builder.setInsertInto(info.convertOriginalToLowered.func);
                            builder.addNameHintDecoration(
                                info.convertOriginalToLowered.func,
                                UnownedStringSlice("packStorage"));
                            info.convertOriginalToLowered.func->setFullType(
                                builder.getFuncType(1, (IRType**)&type, info.loweredType));
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
        // If `type` is already a lowered type, no more lowering is required.
        LoweredElementTypeInfo info;
        if (mapLoweredTypeToInfo->tryGetValue(type))
        {
            info.originalType = type;
            info.loweredType = type;
            return info;
        }

        if (loweredTypeInfo[(int)rules->ruleName].tryGetValue(type, info))
            return info;
        info = getLoweredTypeInfoImpl(type, rules);
        IRSizeAndAlignment sizeAlignment;
        getSizeAndAlignment(target->getOptionSet(), rules, info.loweredType, &sizeAlignment);
        loweredTypeInfo[(int)rules->ruleName].set(type, info);
        mapLoweredTypeToInfo[(int)rules->ruleName].set(info.loweredType, info);
        conversionMethodMap[{info.originalType, info.loweredType}] = info.convertLoweredToOriginal;
        conversionMethodMap[{info.loweredType, info.originalType}] = info.convertOriginalToLowered;
        return info;
    }

    IRType* getLoweredPtrLikeType(IRType* originalPtrLikeType, IRType* newElementType)
    {
        if (as<IRPointerLikeType>(originalPtrLikeType) || as<IRPtrTypeBase>(originalPtrLikeType) ||
            as<IRHLSLStructuredBufferTypeBase>(originalPtrLikeType))
        {
            IRBuilder builder(newElementType);
            builder.setInsertAfter(newElementType);
            ShortList<IRInst*> operands;
            for (UInt i = 0; i < originalPtrLikeType->getOperandCount(); i++)
                operands.add(originalPtrLikeType->getOperand(i));
            operands[0] = newElementType;
            return builder.getType(
                originalPtrLikeType->getOp(),
                (UInt)operands.getCount(),
                operands.getArrayView().getBuffer());
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
            if (options.lowerBufferPointer)
            {
                if (auto ptrType = as<IRPtrType>(globalInst))
                {
                    if (ptrType->getAddressSpace() == AddressSpace::UserPointer)
                        elementType = ptrType->getValueType();
                }
            }
            else
            {
                if (auto structBuffer = as<IRHLSLStructuredBufferTypeBase>(globalInst))
                    elementType = structBuffer->getElementType();
                else if (auto constBuffer = as<IRUniformParameterGroupType>(globalInst))
                    elementType = constBuffer->getElementType();
            }
            if (as<IRTextureBufferType>(globalInst))
                continue;
            if (!as<IRStructType>(elementType) && !as<IRMatrixType>(elementType) &&
                !as<IRArrayType>(elementType) && !as<IRBoolType>(elementType))
                continue;
            bufferTypeInsts.add(BufferTypeInfo{(IRType*)globalInst, elementType});
        }

        // Maintain a pending work list of all matrix addresses, and try to lower them out of
        // existance after everything else has been lowered.

        List<MatrixAddrWorkItem> matrixAddrInsts;

        for (auto bufferTypeInfo : bufferTypeInsts)
        {
            auto bufferType = bufferTypeInfo.bufferType;
            auto elementType = bufferTypeInfo.elementType;
            auto layoutRules = getTypeLayoutRuleForBuffer(target, bufferType);
            auto loweredBufferElementTypeInfo = getLoweredTypeInfo(elementType, layoutRules);

            // If the lowered type is the same as original type, no change is required.
            if (loweredBufferElementTypeInfo.loweredType ==
                loweredBufferElementTypeInfo.originalType)
                continue;

            builder.setInsertBefore(bufferType);

            ShortList<IRInst*> typeOperands;
            for (UInt i = 0; i < bufferType->getOperandCount(); i++)
                typeOperands.add(bufferType->getOperand(i));
            typeOperands[0] = loweredBufferElementTypeInfo.loweredType;
            auto loweredBufferType = builder.getType(
                bufferType->getOp(),
                (UInt)typeOperands.getCount(),
                typeOperands.getArrayView().getBuffer());

            // We treat a value of a buffer type as a pointer, and use a work list to translate
            // all loads and stores through the pointer values that needs lowering.

            List<IRInst*> ptrValsWorkList;
            traverseUses(
                bufferType,
                [&](IRUse* use)
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

                // If we are accessing an unsized array element from a pointer, we need to compute
                // the trailing ptr that points to the first element of the array.
                // And then replace all getElementPtr(arrayPtr, index) with
                // getOffsetPtr(trailingPtr, index).
                if (auto fieldAddr = as<IRFieldAddress>(ptrVal))
                {
                    if (auto ptrType = as<IRPtrType>(ptrVal->getDataType()))
                    {
                        if (auto unsizedArrayType = as<IRUnsizedArrayType>(ptrType->getValueType()))
                        {
                            builder.setInsertBefore(ptrVal);
                            auto newArrayPtrVal = fieldAddr->getBase();
                            auto loweredInnerType =
                                getLoweredTypeInfo(unsizedArrayType->getElementType(), layoutRules);

                            IRSizeAndAlignment arrayElementSizeAlignment;
                            getSizeAndAlignment(
                                target->getOptionSet(),
                                layoutRules,
                                loweredInnerType.loweredType,
                                &arrayElementSizeAlignment);
                            IRSizeAndAlignment baseSizeAlignment;
                            getSizeAndAlignment(
                                target->getOptionSet(),
                                layoutRules,
                                tryGetPointedToType(&builder, fieldAddr->getBase()->getDataType()),
                                &baseSizeAlignment);

                            // Convert pointer to uint64 and adjust offset.
                            IRIntegerValue offset = baseSizeAlignment.size;
                            offset = align(offset, arrayElementSizeAlignment.alignment);
                            if (offset != 0)
                            {
                                auto rawPtr =
                                    builder.emitBitCast(builder.getUInt64Type(), newArrayPtrVal);
                                newArrayPtrVal = builder.emitAdd(
                                    rawPtr->getFullType(),
                                    rawPtr,
                                    builder.getIntValue(builder.getUInt64Type(), offset));
                            }
                            newArrayPtrVal = builder.emitBitCast(
                                builder.getPtrType(
                                    loweredInnerType.loweredType,
                                    ptrType->getAddressSpace()),
                                newArrayPtrVal);
                            traverseUses(
                                ptrVal,
                                [&](IRUse* use)
                                {
                                    auto user = use->getUser();
                                    if (user->getOp() == kIROp_GetElementPtr)
                                    {
                                        builder.setInsertBefore(user);
                                        auto newElementPtr = builder.emitGetOffsetPtr(
                                            newArrayPtrVal,
                                            user->getOperand(1));
                                        user->replaceUsesWith(newElementPtr);
                                        user->removeAndDeallocate();
                                        ptrValsWorkList.add(newElementPtr);
                                    }
                                    else if (user->getOp() == kIROp_GetOffsetPtr)
                                    {
                                    }
                                    else
                                    {
                                        SLANG_UNEXPECTED(
                                            "unknown use of pointer to unsized array.");
                                    }
                                });
                            SLANG_ASSERT(!ptrVal->hasUses());
                            ptrVal->removeAndDeallocate();
                            continue;
                        }
                    }
                }

                LoweredElementTypeInfo loweredElementTypeInfo = {};
                if (auto getElementPtr = as<IRGetElementPtr>(ptrVal))
                {
                    if (auto arrayType = as<IRArrayTypeBase>(
                            tryGetPointedToType(&builder, getElementPtr->getBase()->getDataType())))
                    {
                        // For WGSL, an array of scalar or vector type will always be converted to
                        // an array of 16-byte aligned vector type. In this case, we will run into a
                        // GetElementPtr where the result type is different from the element type of
                        // the base array.
                        // We should setup loweredElementTypeInfo so the remaining logic can handle
                        // this case and insert proper packing/unpacking logic around it.
                        if (arrayType->getElementType() != originalElementType &&
                            isScalarOrVectorType(originalElementType))
                        {
                            loweredElementTypeInfo.loweredType = arrayType->getElementType();
                            loweredElementTypeInfo.originalType = (IRType*)originalElementType;
                            loweredElementTypeInfo.convertLoweredToOriginal = getConversionMethod(
                                loweredElementTypeInfo.originalType,
                                loweredElementTypeInfo.loweredType);
                            loweredElementTypeInfo.convertOriginalToLowered = getConversionMethod(
                                loweredElementTypeInfo.loweredType,
                                loweredElementTypeInfo.originalType);
                        }
                    }
                }

                // For general cases we simply check if the element type needs lowering.
                // If so we will insert packing/unpacking logic if necessary.
                //
                if (!loweredElementTypeInfo.loweredType)
                {
                    loweredElementTypeInfo =
                        getLoweredTypeInfo((IRType*)originalElementType, layoutRules);
                }

                if (!loweredElementTypeInfo.convertLoweredToOriginal)
                    continue;

                ptrVal->setFullType(getLoweredPtrLikeType(
                    ptrVal->getFullType(),
                    loweredElementTypeInfo.loweredType));

                traverseUses(
                    ptrVal,
                    [&](IRUse* use)
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
                                auto unpackedVal =
                                    loweredElementTypeInfo.convertLoweredToOriginal.apply(
                                        builder,
                                        loweredElementTypeInfo.originalType,
                                        newLoad);
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
                                auto packedVal =
                                    loweredElementTypeInfo.convertOriginalToLowered.apply(
                                        builder,
                                        loweredElementTypeInfo.loweredType,
                                        originalVal);
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
                                // In that case, all existing address insts should be appended with
                                // a field extract.
                                if (as<IRArrayType>(originalElementType))
                                {
                                    builder.setInsertBefore(user);
                                    List<IRInst*> args;
                                    for (UInt i = 0; i < user->getOperandCount(); i++)
                                        args.add(user->getOperand(i));
                                    auto newArrayPtrVal = builder.emitFieldAddress(
                                        builder.getPtrType(
                                            loweredElementTypeInfo.loweredInnerArrayType),
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
                                    matrixAddrInsts.add(MatrixAddrWorkItem{user, layoutRules});
                                }
                                else
                                {
                                    // If we getting a derived address from the pointer, we need
                                    // to recursively lower the new address. We do so by pushing
                                    // the address inst into the work list.
                                    ptrValsWorkList.add(user);
                                }
                            }
                            break;
                        case kIROp_RWStructuredBufferGetElementPtr:
                        case kIROp_GetOffsetPtr:
                            ptrValsWorkList.add(user);
                            break;
                        case kIROp_StructuredBufferGetDimensions:
                            break;
                        case kIROp_Call:
                            {
                                // If a structured buffer or pointer typed value is used directly as
                                // an argument, we don't need to do any marshalling here.
                                if (as<IRHLSLStructuredBufferTypeBase>(ptrVal->getDataType()))
                                    break;
                                if (options.lowerBufferPointer &&
                                    as<IRPtrType>(ptrVal->getDataType()))
                                    break;
                                // If we are calling a function with an l-value pointer from buffer
                                // access, we need to materialize the object as a local variable,
                                // and pass the address of the local variable to the function.
                                builder.setInsertBefore(user);
                                auto newLoad =
                                    builder.emitLoad(loweredElementTypeInfo.loweredType, ptrVal);
                                auto unpackedVal =
                                    loweredElementTypeInfo.convertLoweredToOriginal.apply(
                                        builder,
                                        (IRType*)originalElementType,
                                        newLoad);
                                auto var = builder.emitVar((IRType*)originalElementType);
                                builder.emitStore(var, unpackedVal);
                                use->set(var);
                                builder.setInsertAfter(user);
                                auto newVal = builder.emitLoad(var);
                                auto packedVal =
                                    loweredElementTypeInfo.convertOriginalToLowered.apply(
                                        builder,
                                        (IRType*)loweredElementTypeInfo.loweredType,
                                        newVal);
                                builder.emitStore(ptrVal, packedVal);
                            }
                            break;
                        default:
                            break;
                        }
                    });
            }

            // Replace all remaining uses of bufferType to loweredBufferType, these uses are
            // non-operational and should be directly replaceable, such as uses in `IRFuncType`.
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
            auto loweredMatrixType =
                cast<IRPtrTypeBase>(majorGEP->getBase()->getFullType())->getValueType();
            auto matrixTypeInfo =
                mapLoweredTypeToInfo[layoutRuleName].tryGetValue(loweredMatrixType);
            SLANG_ASSERT(matrixTypeInfo);
            auto matrixType = as<IRMatrixType>(matrixTypeInfo->originalType);
            auto rowCount = getIntVal(matrixType->getRowCount());
            traverseUses(
                majorAddr,
                [&](IRUse* use)
                {
                    auto user = use->getUser();
                    builder.setInsertBefore(user);
                    switch (user->getOp())
                    {
                    case kIROp_Load:
                        {
                            IRInst* resultInst = nullptr;
                            auto dataPtr = builder.emitFieldAddress(
                                getLoweredPtrLikeType(
                                    majorAddr->getDataType(),
                                    matrixTypeInfo->loweredInnerArrayType),
                                majorGEP->getBase(),
                                matrixTypeInfo->loweredInnerStructKey);
                            if (getIntVal(matrixType->getLayout()) ==
                                SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                            {
                                List<IRInst*> args;
                                for (IRIntegerValue i = 0; i < rowCount; i++)
                                {
                                    auto vector =
                                        builder.emitLoad(builder.emitElementAddress(dataPtr, i));
                                    auto element =
                                        builder.emitElementExtract(vector, majorGEP->getIndex());
                                    args.add(element);
                                }
                                resultInst = builder.emitMakeVector(
                                    builder.getVectorType(
                                        matrixType->getElementType(),
                                        (IRIntegerValue)args.getCount()),
                                    args);
                            }
                            else
                            {
                                auto element =
                                    builder.emitElementAddress(dataPtr, majorGEP->getIndex());
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
                                getLoweredPtrLikeType(
                                    majorAddr->getDataType(),
                                    matrixTypeInfo->loweredInnerArrayType),
                                majorGEP->getBase(),
                                matrixTypeInfo->loweredInnerStructKey);
                            if (getIntVal(matrixType->getLayout()) ==
                                SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                            {
                                for (IRIntegerValue i = 0; i < rowCount; i++)
                                {
                                    auto vectorAddr = builder.emitElementAddress(dataPtr, i);
                                    auto elementAddr = builder.emitElementAddress(
                                        vectorAddr,
                                        majorGEP->getIndex());
                                    builder.emitStore(
                                        elementAddr,
                                        builder.emitElementExtract(storeInst->getVal(), i));
                                }
                            }
                            else
                            {
                                auto rowAddr =
                                    builder.emitElementAddress(dataPtr, majorGEP->getIndex());
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
                            if (getIntVal(matrixType->getLayout()) ==
                                SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                            {
                                Swap(rowIndex, colIndex);
                            }
                            auto dataPtr = builder.emitFieldAddress(
                                getLoweredPtrLikeType(
                                    majorAddr->getDataType(),
                                    matrixTypeInfo->loweredInnerArrayType),
                                majorGEP->getBase(),
                                matrixTypeInfo->loweredInnerStructKey);
                            auto vectorAddr = builder.emitElementAddress(dataPtr, rowIndex);
                            auto elementAddr = builder.emitElementAddress(vectorAddr, colIndex);
                            gep2->replaceUsesWith(elementAddr);
                            gep2->removeAndDeallocate();
                            break;
                        }
                    default:
                        SLANG_UNREACHABLE("unhandled inst of a matrix address inst that needs "
                                          "storage lowering.");
                        break;
                    }
                });
        }
    }
};

void lowerBufferElementTypeToStorageType(
    TargetProgram* target,
    IRModule* module,
    BufferElementTypeLoweringOptions options)
{
    SlangMatrixLayoutMode defaultMatrixMode =
        (SlangMatrixLayoutMode)target->getOptionSet().getMatrixLayoutMode();
    if ((isCPUTarget(target->getTargetReq()) || isCUDATarget(target->getTargetReq()) ||
         isMetalTarget(target->getTargetReq())))
        defaultMatrixMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    else if (defaultMatrixMode == SLANG_MATRIX_LAYOUT_MODE_UNKNOWN)
        defaultMatrixMode = SLANG_MATRIX_LAYOUT_ROW_MAJOR;
    LoweredElementTypeContext context(target, options, defaultMatrixMode);
    context.processModule(module);
}


IRTypeLayoutRules* getTypeLayoutRuleForBuffer(TargetProgram* target, IRType* bufferType)
{
    if (target->getTargetReq()->getTarget() != CodeGenTarget::WGSL)
    {
        if (!isKhronosTarget(target->getTargetReq()))
            return IRTypeLayoutRules::getNatural();

        // If we are just emitting GLSL, we can just use the general layout rule.
        if (!target->shouldEmitSPIRVDirectly())
            return IRTypeLayoutRules::getNatural();

        // If the user specified a scalar buffer layout, then just use that.
        if (target->getOptionSet().shouldUseScalarLayout())
            return IRTypeLayoutRules::getNatural();
    }

    if (target->getOptionSet().shouldUseDXLayout())
    {
        if (as<IRUniformParameterGroupType>(bufferType))
        {
            return IRTypeLayoutRules::getConstantBuffer();
        }
        else
            return IRTypeLayoutRules::getNatural();
    }

    // The default behavior is to use std140 for constant buffers and std430 for other buffers.
    switch (bufferType->getOp())
    {
    case kIROp_HLSLStructuredBufferType:
    case kIROp_HLSLRWStructuredBufferType:
    case kIROp_HLSLAppendStructuredBufferType:
    case kIROp_HLSLConsumeStructuredBufferType:
    case kIROp_HLSLRasterizerOrderedStructuredBufferType:
        {
            auto structBufferType = as<IRHLSLStructuredBufferTypeBase>(bufferType);
            auto layoutTypeOp = structBufferType->getDataLayout()
                                    ? structBufferType->getDataLayout()->getOp()
                                    : kIROp_DefaultBufferLayoutType;
            switch (layoutTypeOp)
            {
            case kIROp_DefaultBufferLayoutType:
                return IRTypeLayoutRules::getStd430();
            case kIROp_Std140BufferLayoutType:
                return IRTypeLayoutRules::getStd140();
            case kIROp_Std430BufferLayoutType:
                return IRTypeLayoutRules::getStd430();
            case kIROp_ScalarBufferLayoutType:
                return IRTypeLayoutRules::getNatural();
            }
            return IRTypeLayoutRules::getStd430();
        }
    case kIROp_ConstantBufferType:
    case kIROp_ParameterBlockType:
        {
            auto parameterGroupType = as<IRUniformParameterGroupType>(bufferType);

            auto layoutTypeOp = parameterGroupType->getDataLayout()
                                    ? parameterGroupType->getDataLayout()->getOp()
                                    : kIROp_DefaultBufferLayoutType;
            switch (layoutTypeOp)
            {
            case kIROp_DefaultBufferLayoutType:
                return IRTypeLayoutRules::getStd140();
            case kIROp_Std140BufferLayoutType:
                return IRTypeLayoutRules::getStd140();
            case kIROp_Std430BufferLayoutType:
                return IRTypeLayoutRules::getStd430();
            case kIROp_ScalarBufferLayoutType:
                return IRTypeLayoutRules::getNatural();
            }
            return IRTypeLayoutRules::getStd140();
        }
    case kIROp_PtrType:
        return IRTypeLayoutRules::getNatural();
    }
    return IRTypeLayoutRules::getNatural();
}

} // namespace Slang
