#include "slang-ir-lower-buffer-element-type.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{

struct TypeLoweringConfig
{
    AddressSpace addressSpace;
    IRTypeLayoutRules* layoutRule;
    bool operator==(const TypeLoweringConfig& other) const
    {
        return addressSpace == other.addressSpace && layoutRule == other.layoutRule;
    }
    HashCode getHashCode() const
    {
        return combineHash(Slang::getHashCode(addressSpace), Slang::getHashCode(layoutRule));
    }
};
TypeLoweringConfig getTypeLoweringConfigForBuffer(TargetProgram* target, IRType* bufferType);

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
        IRInst* apply(IRBuilder& builder, IRType* resultType, IRInst* operandAddr)
        {
            if (!*this)
                return builder.emitLoad(operandAddr);
            if (kind == ConversionMethodKind::Func)
                return builder.emitCallInst(resultType, func, 1, &operandAddr);
            else
            {
                auto val = builder.emitLoad(operandAddr);
                return builder.emitIntrinsicInst(resultType, op, 1, &val);
            }
        }
        void applyDestinationDriven(IRBuilder& builder, IRInst* dest, IRInst* operand)
        {
            if (!*this)
            {
                builder.emitStore(dest, operand);
                return;
            }
            if (kind == ConversionMethodKind::Func)
            {
                IRInst* operands[] = {dest, operand};
                builder.emitCallInst(builder.getVoidType(), func, 2, operands);
            }
            else
            {
                auto val = builder.emitIntrinsicInst(
                    tryGetPointedToType(&builder, dest->getDataType()),
                    op,
                    1,
                    &operand);
                builder.emitStore(dest, val);
            }
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

    struct LoweredTypeMap : RefObject
    {
        Dictionary<IRType*, LoweredElementTypeInfo> loweredTypeInfo;
        Dictionary<IRType*, LoweredElementTypeInfo> mapLoweredTypeToInfo;
    };

    Dictionary<TypeLoweringConfig, RefPtr<LoweredTypeMap>> loweredTypeInfoMaps;

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

    struct SpecializationKey
    {
        IRFunc* callee;
        IRFuncType* specializedFuncType;
        bool operator==(const SpecializationKey& other) const
        {
            return (callee == other.callee && specializedFuncType == other.specializedFuncType);
        }
        HashCode64 getHashCode() const
        {
            return combineHash(Slang::getHashCode(callee), Slang::getHashCode(specializedFuncType));
        }
    };
    // Specialized functions that takes storage-typed pointers instead of logical-typed pointers.
    Dictionary<SpecializationKey, IRFunc*> specializedFuncs;

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
        IRStructKey* dataKey)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto refStructType = builder.getRefType(structType, AddressSpace::Generic);
        auto funcType = builder.getFuncType(1, (IRType**)&refStructType, matrixType);
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("unpackStorage"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto rowCount = (Index)getIntVal(matrixType->getRowCount());
        auto colCount = (Index)getIntVal(matrixType->getColumnCount());
        auto packedParamRef = builder.emitParam(refStructType);
        auto packedParam = builder.emitLoad(packedParamRef);
        auto vectorArray = builder.emitFieldExtract(packedParam, dataKey);
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
        auto outStructType = builder.getRefType(structType, AddressSpace::Generic);
        IRType* paramTypes[] = {outStructType, matrixType};
        auto funcType = builder.getFuncType(2, paramTypes, builder.getVoidType());
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("packMatrix"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto rowCount = getIntVal(matrixType->getRowCount());
        auto colCount = getIntVal(matrixType->getColumnCount());
        auto outParam = builder.emitParam(outStructType);
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
        builder.emitStore(outParam, result);
        builder.emitReturn();
        return func;
    }

    IRFunc* createArrayUnpackFunc(
        IRArrayType* arrayType,
        IRStructType* structType,
        IRStructKey* dataKey,
        LoweredElementTypeInfo innerTypeInfo)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto refStructType = builder.getRefType(structType, AddressSpace::Generic);
        auto funcType = builder.getFuncType(1, (IRType**)&refStructType, arrayType);
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("unpackStorage"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto packedParam = builder.emitParam(refStructType);
        auto packedArray = builder.emitFieldAddress(packedParam, dataKey);
        auto count = getArraySizeVal(arrayType->getElementCount());
        IRInst* result = nullptr;
        if (count <= kMaxArraySizeToUnroll)
        {
            // If the array is small enough, just process each element directly.
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto packedElementAddr = builder.emitElementAddress(packedArray, ii);
                auto originalElement = innerTypeInfo.convertLoweredToOriginal.apply(
                    builder,
                    innerTypeInfo.originalType,
                    packedElementAddr);
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
            auto packedElementAddr = builder.emitElementAddress(packedArray, loopParam);
            auto originalElement = innerTypeInfo.convertLoweredToOriginal.apply(
                builder,
                innerTypeInfo.originalType,
                packedElementAddr);
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
        IRStructKey* arrayStructKey,
        LoweredElementTypeInfo innerTypeInfo)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto outLoweredType = builder.getRefType(structType, AddressSpace::Generic);
        IRType* paramTypes[] = {outLoweredType, structType};
        auto funcType = builder.getFuncType(2, paramTypes, builder.getVoidType());
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("packStorage"));
        builder.addForceInlineDecoration(func);
        builder.setInsertInto(func);
        builder.emitBlock();
        auto outParam = builder.emitParam(outLoweredType);
        auto originalParam = builder.emitParam(arrayType);
        auto count = getArraySizeVal(arrayType->getElementCount());
        auto destArray = builder.emitFieldAddress(outParam, arrayStructKey);
        if (count <= kMaxArraySizeToUnroll)
        {
            // If the array is small enough, just process each element directly.
            List<IRInst*> args;
            args.setCount((Index)count);
            for (IRIntegerValue ii = 0; ii < count; ++ii)
            {
                auto originalElement = builder.emitElementExtract(originalParam, ii);
                auto destArrayElement = builder.emitElementAddress(destArray, ii);
                innerTypeInfo.convertOriginalToLowered.applyDestinationDriven(
                    builder,
                    destArrayElement,
                    originalElement);
            }
        }
        else
        {
            // The general case for large arrays is to emit a loop through the elements.
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
            auto varPtr = builder.emitElementAddress(destArray, loopParam);
            innerTypeInfo.convertOriginalToLowered.applyDestinationDriven(
                builder,
                varPtr,
                originalElement);
            builder.setInsertInto(loopBreakBlock);
        }
        builder.emitReturn();
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
        case IRTypeLayoutRuleName::C:
            return "c";
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

    bool shouldLowerMatrixType(IRMatrixType* matrixType, TypeLoweringConfig config)
    {
        // For spirv, we always want to lower all matrix types, because SPIRV does not support
        // specifying matrix layout/stride if the matrix type is used in places other than
        // defining a struct field. This means that if a matrix is used to define a varying
        // parameter, we always want to wrap it in a struct.
        //
        if (target->shouldEmitSPIRVDirectly())
        {
            return true;
        }

        if (getIntVal(matrixType->getLayout()) == defaultMatrixLayout &&
            config.layoutRule->ruleName == IRTypeLayoutRuleName::Natural)
        {
            // For other targets, we only lower the matrix types if they differ from the default
            // matrix layout.
            return false;
        }
        return true;
    }

    LoweredElementTypeInfo getLoweredTypeInfoImpl(IRType* type, TypeLoweringConfig config)
    {
        IRBuilder builder(type);
        builder.setInsertAfter(type);

        LoweredElementTypeInfo info;
        info.originalType = type;

        if (auto matrixType = as<IRMatrixType>(type))
        {
            if (!shouldLowerMatrixType(matrixType, config))
            {
                info.loweredType = type;
                return info;
            }

            auto loweredType = builder.createStructType();
            builder.addPhysicalTypeDecoration(loweredType);

            StringBuilder nameSB;
            bool isColMajor =
                getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
            nameSB << "_MatrixStorage_";
            getTypeNameHint(nameSB, matrixType->getElementType());
            nameSB << getIntVal(matrixType->getRowCount()) << "x"
                   << getIntVal(matrixType->getColumnCount());
            if (isColMajor)
                nameSB << "_ColMajor";
            nameSB << getLayoutName(config.layoutRule->ruleName);
            builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
            auto structKey = builder.createStructKey();
            builder.addNameHintDecoration(structKey, UnownedStringSlice("data"));
            auto vectorSize = isColMajor ? matrixType->getRowCount() : matrixType->getColumnCount();
            if (config.layoutRule->ruleName == IRTypeLayoutRuleName::Std140 &&
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
            getSizeAndAlignment(
                target->getOptionSet(),
                config.layoutRule,
                vectorType,
                &elementSizeAlignment);
            elementSizeAlignment = config.layoutRule->alignCompositeElement(elementSizeAlignment);

            auto arrayType = builder.getArrayType(
                vectorType,
                isColMajor ? matrixType->getColumnCount() : matrixType->getRowCount(),
                builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride()));
            builder.createStructField(loweredType, structKey, arrayType);

            info.loweredType = loweredType;
            info.loweredInnerArrayType = arrayType;
            info.loweredInnerStructKey = structKey;
            info.convertLoweredToOriginal =
                createMatrixUnpackFunc(matrixType, loweredType, structKey);
            info.convertOriginalToLowered =
                createMatrixPackFunc(matrixType, loweredType, vectorType, arrayType);
            return info;
        }
        else if (auto arrayTypeBase = as<IRArrayTypeBase>(type))
        {
            auto loweredInnerTypeInfo = getLoweredTypeInfo(arrayTypeBase->getElementType(), config);

            if (config.layoutRule->ruleName == IRTypeLayoutRuleName::Std140 &&
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

            // For spirv backend, we always want to lower all array types for non-varying
            // parameters, even if the element type comes out the same. This is because different
            // layout rules may have different array stride requirements.
            if (!target->shouldEmitSPIRVDirectly() || config.addressSpace == AddressSpace::Input)
            {
                if (!loweredInnerTypeInfo.convertLoweredToOriginal)
                {
                    info.loweredType = type;
                    return info;
                }
            }

            auto arrayType = as<IRArrayType>(arrayTypeBase);
            if (arrayType)
            {
                auto loweredType = builder.createStructType();
                builder.addPhysicalTypeDecoration(loweredType);

                info.loweredType = loweredType;
                StringBuilder nameSB;
                nameSB << "_Array_" << getLayoutName(config.layoutRule->ruleName) << "_";
                getTypeNameHint(nameSB, arrayType->getElementType());
                nameSB << getArraySizeVal(arrayType->getElementCount());

                builder.addNameHintDecoration(
                    loweredType,
                    nameSB.produceString().getUnownedSlice());
                auto structKey = builder.createStructKey();
                builder.addNameHintDecoration(structKey, UnownedStringSlice("data"));
                IRSizeAndAlignment elementSizeAlignment;
                getSizeAndAlignment(
                    target->getOptionSet(),
                    config.layoutRule,
                    loweredInnerTypeInfo.loweredType,
                    &elementSizeAlignment);
                elementSizeAlignment =
                    config.layoutRule->alignCompositeElement(elementSizeAlignment);
                auto innerArrayType = builder.getArrayType(
                    loweredInnerTypeInfo.loweredType,
                    arrayType->getElementCount(),
                    builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride()));
                builder.createStructField(loweredType, structKey, innerArrayType);
                info.loweredInnerArrayType = innerArrayType;
                info.loweredInnerStructKey = structKey;
                info.convertLoweredToOriginal =
                    createArrayUnpackFunc(arrayType, loweredType, structKey, loweredInnerTypeInfo);
                info.convertOriginalToLowered =
                    createArrayPackFunc(arrayType, loweredType, structKey, loweredInnerTypeInfo);
            }
            else
            {
                IRSizeAndAlignment elementSizeAlignment;
                getSizeAndAlignment(
                    target->getOptionSet(),
                    config.layoutRule,
                    loweredInnerTypeInfo.loweredType,
                    &elementSizeAlignment);
                elementSizeAlignment =
                    config.layoutRule->alignCompositeElement(elementSizeAlignment);
                auto innerArrayType = builder.getArrayTypeBase(
                    arrayTypeBase->getOp(),
                    loweredInnerTypeInfo.loweredType,
                    nullptr,
                    builder.getIntValue(builder.getIntType(), elementSizeAlignment.getStride()));
                info.loweredType = innerArrayType;
            }
            return info;
        }
        else if (auto structType = as<IRStructType>(type))
        {
            List<LoweredElementTypeInfo> fieldLoweredTypeInfo;
            bool isTrivial = true;
            for (auto field : structType->getFields())
            {
                auto loweredFieldTypeInfo = getLoweredTypeInfo(field->getFieldType(), config);
                fieldLoweredTypeInfo.add(loweredFieldTypeInfo);
                if (loweredFieldTypeInfo.convertLoweredToOriginal ||
                    config.layoutRule->ruleName != IRTypeLayoutRuleName::Natural)
                    isTrivial = false;
            }

            // For spirv backend, we always want to lower all array types, even if the element type
            // comes out the same. This is because different layout rules may have different array
            // stride requirements.
            //
            // Additionally, `buffer` blocks do not work correctly unless lowered when targeting
            // GLSL.
            if (!isKhronosTarget(target->getTargetReq()))
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
            builder.addPhysicalTypeDecoration(loweredType);

            StringBuilder nameSB;
            getTypeNameHint(nameSB, type);
            nameSB << "_" << getLayoutName(config.layoutRule->ruleName);
            builder.addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
            info.loweredType = loweredType;
            // Create fields.
            {
                Index fieldId = 0;
                for (auto field : structType->getFields())
                {
                    auto& loweredFieldTypeInfo = fieldLoweredTypeInfo[fieldId];
                    // When lowering type for user pointer, skip fields that are unsized array.
                    if (config.addressSpace == AddressSpace::UserPointer &&
                        as<IRUnsizedArrayType>(loweredFieldTypeInfo.loweredType))
                    {
                        fieldId++;
                        loweredFieldTypeInfo.loweredType = builder.getVoidType();
                        continue;
                    }
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
                builder.addForceInlineDecoration(info.convertLoweredToOriginal.func);
                auto refLoweredType = builder.getRefType(loweredType, AddressSpace::Generic);
                info.convertLoweredToOriginal.func->setFullType(
                    builder.getFuncType(1, (IRType**)&refLoweredType, type));
                builder.emitBlock();
                auto loweredParam = builder.emitParam(refLoweredType);
                List<IRInst*> args;
                Index fieldId = 0;
                for (auto field : structType->getFields())
                {
                    if (as<IRVoidType>(fieldLoweredTypeInfo[fieldId].loweredType))
                    {
                        fieldId++;
                        continue;
                    }
                    auto storageField = builder.emitFieldAddress(loweredParam, field->getKey());
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
                builder.addForceInlineDecoration(info.convertOriginalToLowered.func);

                auto outLoweredType = builder.getRefType(loweredType, AddressSpace::Generic);
                IRType* paramTypes[] = {outLoweredType, type};
                info.convertOriginalToLowered.func->setFullType(
                    builder.getFuncType(2, paramTypes, builder.getVoidType()));
                builder.emitBlock();
                auto outParam = builder.emitParam(outLoweredType);
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
                    auto destAddr = builder.emitFieldAddress(outParam, field->getKey());

                    fieldLoweredTypeInfo[fieldId].convertOriginalToLowered.applyDestinationDriven(
                        builder,
                        destAddr,
                        fieldVal);
                    fieldId++;
                }
                builder.emitReturn();
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

                        // Find an integer type of the correct size for the current layout rule.
                        IRSizeAndAlignment boolSizeAndAlignment;
                        if (getSizeAndAlignment(
                                target->getOptionSet(),
                                config.layoutRule,
                                scalarType,
                                &boolSizeAndAlignment) == SLANG_OK)
                        {
                            IntInfo ii;
                            ii.width = boolSizeAndAlignment.size * 8;
                            ii.isSigned = true;
                            info.loweredType = builder.getType(getIntTypeOpFromInfo(ii));
                        }
                        else
                        {
                            // Just in case that fails for some reason, just use an int.
                            info.loweredType = builder.getIntType();
                        }

                        if (vectorType)
                            info.loweredType = builder.getVectorType(
                                info.loweredType,
                                vectorType->getElementCount());
                        info.convertLoweredToOriginal = kIROp_BuiltinCast;
                        info.convertOriginalToLowered = kIROp_BuiltinCast;
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

    LoweredTypeMap& getTypeLoweringMap(TypeLoweringConfig config)
    {
        RefPtr<LoweredTypeMap> map;
        if (loweredTypeInfoMaps.tryGetValue(config, map))
            return *map;
        map = new LoweredTypeMap();
        loweredTypeInfoMaps.add(config, map);
        return *map;
    }

    LoweredElementTypeInfo getLoweredTypeInfo(IRType* type, TypeLoweringConfig config)
    {
        // If `type` is already a lowered type, no more lowering is required.
        LoweredElementTypeInfo info;
        auto& map = getTypeLoweringMap(config);
        auto& mapLoweredTypeToInfo = map.mapLoweredTypeToInfo;
        auto& loweredTypeInfo = map.loweredTypeInfo;
        if (mapLoweredTypeToInfo.tryGetValue(type))
        {
            info.originalType = type;
            info.loweredType = type;
            return info;
        }
        if (loweredTypeInfo.tryGetValue(type, info))
            return info;
        info = getLoweredTypeInfoImpl(type, config);
        IRSizeAndAlignment sizeAlignment;
        getSizeAndAlignment(
            target->getOptionSet(),
            config.layoutRule,
            info.loweredType,
            &sizeAlignment);
        loweredTypeInfo.set(type, info);
        mapLoweredTypeToInfo.set(info.loweredType, info);
        conversionMethodMap[{info.originalType, info.loweredType}] = info.convertLoweredToOriginal;
        conversionMethodMap[{info.loweredType, info.originalType}] = info.convertOriginalToLowered;
        return info;
    }

    IRType* getLoweredPtrLikeType(IRType* originalPtrLikeType, IRType* newElementType)
    {
        IRBuilder builder(newElementType);
        builder.setInsertAfter(newElementType);
        if (auto ptrType = as<IRPtrTypeBase>(originalPtrLikeType))
        {
            // Intentionally drop address space info from the pointer type to keep
            // the IR simple. There are passes after this that may alter the arguments
            // to functions to have a different address-space, so we want to have new
            // functions and instructs we introduce in this pass address-space generic.
            // and let the follow-up address-space specialization pass to fill in the final
            // address space.
            return builder.getPtrType(ptrType->getOp(), newElementType);
        }

        if (as<IRPointerLikeType>(originalPtrLikeType) ||
            as<IRHLSLStructuredBufferTypeBase>(originalPtrLikeType) ||
            as<IRGLSLShaderStorageBufferType>(originalPtrLikeType))
        {
            return builder.getPtrType(newElementType);
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
        TypeLoweringConfig config;
    };

    IRInst* getBufferAddr(IRBuilder& builder, IRInst* loadStoreInst)
    {
        switch (loadStoreInst->getOp())
        {
        case kIROp_Load:
        case kIROp_Store:
            return loadStoreInst->getOperand(0);
        case kIROp_StructuredBufferLoad:
        case kIROp_StructuredBufferLoadStatus:
        case kIROp_RWStructuredBufferLoad:
        case kIROp_RWStructuredBufferLoadStatus:
        case kIROp_RWStructuredBufferStore:
            return builder.emitRWStructuredBufferGetElementPtr(
                loadStoreInst->getOperand(0),
                loadStoreInst->getOperand(1));
        default:
            return nullptr;
        }
    }

    bool maybeTranslateTailingPointerGetElementAddress(
        IRBuilder& builder,
        IRFieldAddress* fieldAddr,
        IRCastStorageToLogical* castInst,
        TypeLoweringConfig& config,
        List<IRCastStorageToLogical*>& castInstWorkList)
    {
        // If we are accessing an unsized array element from a pointer, we need to
        // compute
        // the trailing ptr that points to the first element of the array.
        // And then replace all getElementPtr(arrayPtr, index) with
        // getOffsetPtr(trailingPtr, index).

        auto ptrType = as<IRPtrTypeBase>(fieldAddr->getDataType());
        if (!ptrType)
            return false;
        if (ptrType->getAddressSpace() != AddressSpace::UserPointer)
            return false;
        if (auto unsizedArrayType = as<IRUnsizedArrayType>(ptrType->getValueType()))
        {
            builder.setInsertBefore(fieldAddr);
            auto newArrayPtrVal = fieldAddr->getBase();
            auto loweredInnerType = getLoweredTypeInfo(unsizedArrayType->getElementType(), config);

            IRSizeAndAlignment arrayElementSizeAlignment;
            getSizeAndAlignment(
                target->getOptionSet(),
                config.layoutRule,
                loweredInnerType.loweredType,
                &arrayElementSizeAlignment);
            IRSizeAndAlignment baseSizeAlignment;
            getSizeAndAlignment(
                target->getOptionSet(),
                config.layoutRule,
                tryGetPointedToType(&builder, fieldAddr->getBase()->getDataType()),
                &baseSizeAlignment);

            // Convert pointer to uint64 and adjust offset.
            IRIntegerValue offset = baseSizeAlignment.size;
            offset = align(offset, arrayElementSizeAlignment.alignment);
            if (offset != 0)
            {
                auto rawPtr = builder.emitBitCast(builder.getUInt64Type(), newArrayPtrVal);
                newArrayPtrVal = builder.emitAdd(
                    rawPtr->getFullType(),
                    rawPtr,
                    builder.getIntValue(builder.getUInt64Type(), offset));
            }
            newArrayPtrVal = builder.emitBitCast(
                builder.getPtrType(loweredInnerType.loweredType),
                newArrayPtrVal);
            traverseUses(
                fieldAddr,
                [&](IRUse* fieldAddrUse)
                {
                    auto fieldAddrUser = fieldAddrUse->getUser();
                    if (fieldAddrUser->getOp() == kIROp_GetElementPtr)
                    {
                        builder.setInsertBefore(fieldAddrUser);
                        auto newElementPtr =
                            builder.emitGetOffsetPtr(newArrayPtrVal, fieldAddrUser->getOperand(1));
                        auto castedGEP = builder.emitCastStorageToLogical(
                            fieldAddrUser->getFullType(),
                            newElementPtr,
                            castInst->getBufferType());
                        fieldAddrUser->replaceUsesWith(castedGEP);
                        fieldAddrUser->removeAndDeallocate();
                        castInstWorkList.add(castedGEP);
                    }
                    else if (fieldAddrUser->getOp() == kIROp_GetOffsetPtr)
                    {
                    }
                    else
                    {
                        SLANG_UNEXPECTED("unknown use of pointer to unsized array.");
                    }
                });
            SLANG_ASSERT(!fieldAddr->hasUses());
            fieldAddr->removeAndDeallocate();
            return true;
        }
        return false;
    }

    void deferStorageToLogicalCasts(
        IRModule* module,
        List<IRCastStorageToLogical*> castInstWorkList)
    {
        IRBuilder builder(module);

        while (castInstWorkList.getCount())
        {
            // We process call instructions after other instructions, so we
            // can be sure that all castStorageToLogical insts have already
            // been pushed to the call argument lists before we process it.
            HashSet<IRCall*> callWorkList;
            // Defer the storage-to-logical cast operation to latest possible time to avoid
            // unnecessary packing/unpacking.
            for (Index i = 0; i < castInstWorkList.getCount(); i++)
            {
                auto castInst = castInstWorkList[i];
                auto ptrVal = castInst->getOperand(0);
                auto config =
                    getTypeLoweringConfigForBuffer(target, (IRType*)castInst->getBufferType());
                traverseUses(
                    castInst,
                    [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        switch (user->getOp())
                        {
                        case kIROp_FieldAddress:
                            // If our logical struct type ends with an unsized array field, the
                            // storage struct type won't have this field defined.
                            // Therefore, all fieldAddress(obj, lastField) inst retrieving the last
                            // field of such struct should be translated into
                            // `(ArrayElementType*)((StorageStruct*)(obj)+1) + idx`.
                            // That is, we should first compute the tailing pointer of the
                            // struct, and replace all getElementPtr(fieldAddr, idx) with
                            // getOffsetPtr(tailingPtr, idx).
                            if (maybeTranslateTailingPointerGetElementAddress(
                                    builder,
                                    (IRFieldAddress*)user,
                                    castInst,
                                    config,
                                    castInstWorkList))
                                return;
                            [[fallthrough]];
                        case kIROp_GetElementPtr:
                        case kIROp_GetOffsetPtr:
                        case kIROp_RWStructuredBufferGetElementPtr:
                            {
                                // gep(castStorageToLogical(x)) ==> castStorageToLogical(gep(x))
                                if (user->getOperand(0) != castInst)
                                    break;
                                auto logicalBaseType = castInst->getDataType();
                                auto logicalType = user->getDataType();
                                IRInst* storageBaseAddr = ptrVal;
                                auto originalBaseValueType =
                                    tryGetPointedToType(&builder, logicalBaseType);
                                if (user->getOp() == kIROp_GetElementPtr)
                                {
                                    // If original type is an array, the lowered type will be a
                                    // struct. In that case, all existing address insts should be
                                    // appended with a field extract.
                                    if (as<IRArrayType>(originalBaseValueType))
                                    {
                                        builder.setInsertBefore(user);
                                        List<IRInst*> args;
                                        for (UInt i = 0; i < user->getOperandCount(); i++)
                                            args.add(user->getOperand(i));
                                        auto arrayLowerInfo =
                                            getLoweredTypeInfo(originalBaseValueType, config);
                                        storageBaseAddr = builder.emitFieldAddress(
                                            builder.getPtrType(
                                                arrayLowerInfo.loweredInnerArrayType),
                                            ptrVal,
                                            arrayLowerInfo.loweredInnerStructKey);
                                    }
                                    if (as<IRMatrixType>(originalBaseValueType))
                                    {
                                        // We are tring to get a pointer to a lowered matrix
                                        // element. We process this insts at a later phase.
                                        SLANG_ASSERT(user->getOp() == kIROp_GetElementPtr);
                                        lowerMatrixAddresses(
                                            module,
                                            MatrixAddrWorkItem{user, config});
                                        break;
                                    }
                                }

                                ShortList<IRInst*> newArgs;
                                newArgs.add(storageBaseAddr);
                                for (UInt i = 1; i < user->getOperandCount(); i++)
                                    newArgs.add(user->getOperand(i));
                                builder.setInsertBefore(user);
                                auto logicalValueType = tryGetPointedToType(&builder, logicalType);
                                auto storageTypeInfo = getLoweredTypeInfo(logicalValueType, config);
                                auto storageGEP = builder.emitIntrinsicInst(
                                    builder.getPtrType(storageTypeInfo.loweredType),
                                    user->getOp(),
                                    newArgs.getCount(),
                                    newArgs.getArrayView().getBuffer());
                                auto castOfGEP = builder.emitCastStorageToLogical(
                                    logicalType,
                                    storageGEP,
                                    castInst->getBufferType());
                                user->replaceUsesWith(castOfGEP);
                                user->removeAndDeallocate();
                                castInstWorkList.add(castOfGEP);
                                break;
                            }
                        case kIROp_Call:
                            {
                                // call(f, castStorageToLogical(x)) ==> call(f', x)
                                //
                                // If we see a call that takes a logical typed pointer, we will
                                // specialize the callee to take a storage typed pointer instead,
                                // and push the cast to inside the callee.
                                // We will process calls after other gep insts, so for now just add
                                // it into a separate worklist.
                                callWorkList.add((IRCall*)user);
                                break;
                            }
                        }
                    });
            }

            // Now that we have processed all GEP instructions, we can now proceed to
            // process all calls. This is done by making a clone of the callee, and change
            // the parameter type from logical type to storage type, and insert a
            // castStorageToLogical on the parameter. Then we go back to the beginning and make sure
            // we process those newly created castStorageToLogical insts.
            List<IRCastStorageToLogical*> newCasts;
            for (auto call : callWorkList)
            {
                auto calleeFunc = as<IRGlobalValueWithParams>(call->getCallee());
                List<IRInst*> oldParams;

                for (auto param : calleeFunc->getParams())
                    oldParams.add(param);
                SLANG_ASSERT(oldParams.getCount() == (Index)call->getArgCount());

                ShortList<IRType*> paramTypes;
                ShortList<IRInst*> newArgs;
                for (UInt i = 0; i < call->getArgCount(); i++)
                {
                    auto arg = call->getArg(i);
                    if (auto castArg = as<IRCastStorageToLogical>(arg))
                    {
                        auto oldParamPtrType = oldParams[i]->getDataType();
                        auto storageValueType =
                            tryGetPointedToType(&builder, castArg->getOperand(0)->getDataType());
                        auto storagePtrType =
                            getLoweredPtrLikeType(oldParamPtrType, storageValueType);
                        paramTypes.add(storagePtrType);
                        newArgs.add(castArg->getOperand(0));
                    }
                    else
                    {
                        paramTypes.add(arg->getDataType());
                        newArgs.add(arg);
                    }
                }
                auto specializedFuncType = builder.getFuncType(
                    (UInt)paramTypes.getCount(),
                    paramTypes.getArrayView().getBuffer(),
                    call->getDataType());
                auto key = SpecializationKey{(IRFunc*)calleeFunc, specializedFuncType};
                IRFunc* specializedFunc = nullptr;
                if (!specializedFuncs.tryGetValue(key, specializedFunc))
                {
                    specializedFunc = createSpecializedFuncThatUseStorageType(
                        call,
                        specializedFuncType,
                        newCasts);
                }
                builder.setInsertBefore(call);
                auto newCall = builder.emitCallInst(
                    call->getFullType(),
                    specializedFunc,
                    newArgs.getArrayView().arrayView);
                call->replaceUsesWith(newCall);
                call->removeAndDeallocate();
            }

            // Remove any casts that have no more uses.
            for (auto cast : castInstWorkList)
            {
                if (!cast->hasUses())
                    cast->removeAndDeallocate();
            }

            // Continue to process new casts added during function specialization.
            castInstWorkList.swapWith(newCasts);
        }
    }

    IRFunc* createSpecializedFuncThatUseStorageType(
        IRCall* call,
        IRFuncType* specializedFuncType,
        List<IRCastStorageToLogical*>& outNewCasts)
    {
        IRBuilder builder(call);
        builder.setInsertBefore(call->getCallee());

        // Create a clone of the callee.
        IRCloneEnv cloneEnv;
        auto clonedFunc = as<IRFunc>(cloneInst(&cloneEnv, &builder, call->getCallee()));
        List<IRUse*> uses;

        // If a parameter is being translated to storage type,
        // insert a cast to convert it to logical type.
        List<IRParam*> params;
        for (auto param : clonedFunc->getParams())
            params.add(param);
        for (UInt i = 0; i < (UInt)params.getCount(); i++)
        {
            auto param = params[i];
            SLANG_RELEASE_ASSERT(i < call->getArgCount());
            auto arg = call->getArg(i);
            auto cast = as<IRCastStorageToLogical>(arg);
            if (!cast)
                continue;
            auto logicalParamType = param->getFullType();
            auto storageType = specializedFuncType->getParamType(i);
            param->setFullType((IRType*)storageType);
            setInsertBeforeOrdinaryInst(&builder, param);

            // Store uses of param before creating a cast inst that uses it.
            uses.clear();
            for (auto use = param->firstUse; use; use = use->nextUse)
                uses.add(use);
            auto castedParam =
                builder.emitCastStorageToLogical(logicalParamType, param, cast->getBufferType());
            outNewCasts.add(castedParam);

            // Replace all previous uses of param to use castedParam instead.
            for (auto use : uses)
                builder.replaceOperand(use, castedParam);
        }
        clonedFunc->setFullType(specializedFuncType);
        removeLinkageDecorations(clonedFunc);
        return clonedFunc;
    }

    void processModule(IRModule* module)
    {
        IRBuilder builder(module);
        struct BufferTypeInfo
        {
            IRType* bufferType;
            IRType* elementType;
            IRType* loweredBufferType = nullptr;
            bool shouldWrapArrayInStruct = false;
        };
        List<BufferTypeInfo> bufferTypeInsts;
        for (auto globalInst : module->getGlobalInsts())
        {
            IRType* elementType = nullptr;

            if (auto ptrType = as<IRPtrTypeBase>(globalInst))
            {
                switch (ptrType->getAddressSpace())
                {
                case AddressSpace::UserPointer:
                    if (!options.lowerBufferPointer)
                        continue;
                    [[fallthrough]];
                case AddressSpace::Input:
                case AddressSpace::Output:
                    elementType = ptrType->getValueType();
                    break;
                }
            }
            if (auto structBuffer = as<IRHLSLStructuredBufferTypeBase>(globalInst))
            {
                elementType = structBuffer->getElementType();
                auto config = getTypeLoweringConfigForBuffer(target, structBuffer);

                // Create size and alignment decoration for potential use
                // in`StructuredBufferGetDimensions`.
                IRSizeAndAlignment sizeAlignment;
                getSizeAndAlignment(
                    target->getOptionSet(),
                    config.layoutRule,
                    elementType,
                    &sizeAlignment);
                SLANG_UNUSED(sizeAlignment);
            }
            else if (auto constBuffer = as<IRUniformParameterGroupType>(globalInst))
                elementType = constBuffer->getElementType();
            else if (auto storageBuffer = as<IRGLSLShaderStorageBufferType>(globalInst))
                elementType = storageBuffer->getElementType();

            if (as<IRTextureBufferType>(globalInst))
                continue;
            if (!as<IRStructType>(elementType) && !as<IRMatrixType>(elementType) &&
                !as<IRArrayType>(elementType) && !as<IRBoolType>(elementType))
                continue;
            bufferTypeInsts.add(BufferTypeInfo{(IRType*)globalInst, elementType});
        }


        List<IRCastStorageToLogical*> castInstWorkList;

        for (auto& bufferTypeInfo : bufferTypeInsts)
        {
            auto bufferType = bufferTypeInfo.bufferType;
            auto elementType = bufferTypeInfo.elementType;

            if (elementType->findDecoration<IRPhysicalTypeDecoration>())
                continue;

            auto config = getTypeLoweringConfigForBuffer(target, bufferType);
            auto loweredBufferElementTypeInfo = getLoweredTypeInfo(elementType, config);

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

            // Replace all global buffer declarations to use the storage type instead,
            // and insert initial `castStorageToLogical` instructions to convert the
            // storage-typed pointer to logical-typed pointer.

            traverseUses(
                bufferType,
                [&](IRUse* use)
                {
                    auto user = use->getUser();
                    if (use != &user->typeUse)
                        return;
                    auto ptrVal = use->getUser();
                    builder.setInsertAfter(ptrVal);
                    builder.replaceOperand(use, loweredBufferType);
                    auto logicalBufferType = getLoweredPtrLikeType(bufferType, elementType);
                    auto castStorageToLogical =
                        builder.emitCastStorageToLogical(logicalBufferType, ptrVal, bufferType);
                    traverseUses(
                        ptrVal,
                        [&](IRUse* ptrUse)
                        {
                            if (ptrUse->getUser() != castStorageToLogical)
                                builder.replaceOperand(ptrUse, castStorageToLogical);
                        });
                    castInstWorkList.add(castStorageToLogical);
                });
            bufferTypeInfo.loweredBufferType = loweredBufferType;
        }

        // Push down `CastStorageToLogical` insts we inserted above to latest possible locations,
        // specializing all function calls along the way, until we truly need the the logical value.
        // This means that `FieldAddr(CastStorageToLogical(buffer), field0))` is translated to
        // `CastStorageToLogical(FieldAddr(buffer, field0))`. This way we can be sure that we are
        // doing minimal packing/unpacking.
        deferStorageToLogicalCasts(module, _Move(castInstWorkList));

        // Now translate the `CastStorageToLogical` into actual packing/unpacking code.
        materializeStorageToLogicalCasts(module->getModuleInst());

        // Replace all remaining uses of bufferType to loweredBufferType, these uses are
        // non-operational and should be directly replaceable, such as uses in `IRFuncType`.
        for (auto bufferTypeInst : bufferTypeInsts)
        {
            if (!bufferTypeInst.loweredBufferType)
                continue;
            bufferTypeInst.bufferType->replaceUsesWith(bufferTypeInst.loweredBufferType);
            bufferTypeInst.bufferType->removeAndDeallocate();
        }
    }

    void materializeStorageToLogicalCastsImpl(IRCastStorageToLogical* castInst)
    {
        // Translate the values to use new lowered buffer type instead.

        auto ptrVal = castInst->getOperand(0);
        auto oldPtrType = castInst->getFullType();
        auto originalElementType = oldPtrType->getOperand(0);
        auto config = getTypeLoweringConfigForBuffer(target, (IRType*)castInst->getBufferType());

        IRBuilder builder(ptrVal);


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
            loweredElementTypeInfo = getLoweredTypeInfo((IRType*)originalElementType, config);
        }

        if (loweredElementTypeInfo.loweredType == loweredElementTypeInfo.originalType)
        {
            castInst->replaceUsesWith(ptrVal);
            castInst->removeAndDeallocate();
            return;
        }

        traverseUses(
            castInst,
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
                        builder.setInsertBefore(user);
                        auto addr = getBufferAddr(builder, user);
                        if (addr == castInst)
                            addr = ptrVal;
                        if (!addr)
                        {
                            IRCloneEnv cloneEnv = {};
                            builder.setInsertBefore(user);
                            auto newLoad = cloneInst(&cloneEnv, &builder, user);
                            newLoad->setFullType(loweredElementTypeInfo.loweredType);
                            addr = builder.emitVar(loweredElementTypeInfo.loweredType);
                            builder.emitStore(addr, newLoad);
                        }
                        if (auto alignedAttr = user->findAttr<IRAlignedAttr>())
                        {
                            builder.addAlignedAddressDecoration(addr, alignedAttr->getAlignment());
                        }
                        auto unpackedVal = loweredElementTypeInfo.convertLoweredToOriginal.apply(
                            builder,
                            loweredElementTypeInfo.originalType,
                            addr);
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
                        IRInst* addr = getBufferAddr(builder, user);
                        if (addr)
                        {
                            addr = ptrVal;
                            if (auto alignedAttr = user->findAttr<IRAlignedAttr>())
                            {
                                builder.addAlignedAddressDecoration(
                                    addr,
                                    alignedAttr->getAlignment());
                            }

                            loweredElementTypeInfo.convertOriginalToLowered.applyDestinationDriven(
                                builder,
                                addr,
                                originalVal);
                            user->removeAndDeallocate();
                        }
                        else if (auto sbAppend = as<IRStructuredBufferAppend>(user))
                        {
                            builder.setInsertBefore(sbAppend);
                            addr = builder.emitVar(loweredElementTypeInfo.loweredType);
                            loweredElementTypeInfo.convertOriginalToLowered.applyDestinationDriven(
                                builder,
                                addr,
                                originalVal);
                            auto packedVal = builder.emitLoad(addr);
                            sbAppend->setOperand(1, packedVal);
                        }
                        else
                        {
                            SLANG_UNREACHABLE("unhandled store type");
                        }
                        break;
                    }
                default:
                    SLANG_UNREACHABLE(
                        "lowerBufferElementType: unknown user of CastStorageToLogical.");

                    break;
                }
            });

        if (!castInst->hasUses())
            castInst->removeAndDeallocate();
    }

    void collectCastStorageToLogicalInsts(List<IRCastStorageToLogical*>& insts, IRInst* root)
    {
        if (root->getOp() == kIROp_CastStorageToLogical)
        {
            insts.add((IRCastStorageToLogical*)root);
            return;
        }
        for (auto child : root->getChildren())
        {
            collectCastStorageToLogicalInsts(insts, child);
        }
    }

    void materializeStorageToLogicalCasts(IRInst* root)
    {
        List<IRCastStorageToLogical*> castInsts;
        collectCastStorageToLogicalInsts(castInsts, root);
        for (auto inst : castInsts)
        {
            materializeStorageToLogicalCastsImpl(inst);
        }
    }

    // Lower all getElementPtr insts of a lowered matrix out of existance.
    void lowerMatrixAddresses(IRModule* module, MatrixAddrWorkItem workItem)
    {
        IRBuilder builder(module);
        auto majorAddr = workItem.matrixAddrInst;
        auto majorGEP = as<IRGetElementPtr>(majorAddr);
        SLANG_ASSERT(majorGEP);
        auto baseCast = as<IRCastStorageToLogical>(majorGEP->getBase());
        SLANG_ASSERT(baseCast);
        auto storageBase = baseCast->getOperand(0);
        auto loweredMatrixType = cast<IRPtrTypeBase>(storageBase->getFullType())->getValueType();
        auto matrixTypeInfo =
            getTypeLoweringMap(workItem.config).mapLoweredTypeToInfo.tryGetValue(loweredMatrixType);
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
                            storageBase,
                            matrixTypeInfo->loweredInnerStructKey);
                        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
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
                            storageBase,
                            matrixTypeInfo->loweredInnerStructKey);
                        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                        {
                            for (IRIntegerValue i = 0; i < rowCount; i++)
                            {
                                auto vectorAddr = builder.emitElementAddress(dataPtr, i);
                                auto elementAddr =
                                    builder.emitElementAddress(vectorAddr, majorGEP->getIndex());
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
                        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
                        {
                            Swap(rowIndex, colIndex);
                        }
                        auto dataPtr = builder.emitFieldAddress(
                            getLoweredPtrLikeType(
                                majorAddr->getDataType(),
                                matrixTypeInfo->loweredInnerArrayType),
                            storageBase,
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
        if (!majorAddr->hasUses())
            majorAddr->removeAndDeallocate();
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

IRTypeLayoutRules* getTypeLayoutRulesFromOp(IROp layoutTypeOp, IRTypeLayoutRules* defaultLayout)
{
    switch (layoutTypeOp)
    {
    case kIROp_DefaultBufferLayoutType:
        return defaultLayout;
    case kIROp_Std140BufferLayoutType:
        return IRTypeLayoutRules::getStd140();
    case kIROp_Std430BufferLayoutType:
        return IRTypeLayoutRules::getStd430();
    case kIROp_ScalarBufferLayoutType:
        return IRTypeLayoutRules::getNatural();
    case kIROp_CBufferLayoutType:
        return IRTypeLayoutRules::getC();
    }
    return defaultLayout;
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

        // If the user specified a C-compatible buffer layout, then do that.
        if (target->getOptionSet().shouldUseCLayout())
            return IRTypeLayoutRules::getC();

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
            return getTypeLayoutRulesFromOp(layoutTypeOp, IRTypeLayoutRules::getStd430());
        }
    case kIROp_ConstantBufferType:
    case kIROp_ParameterBlockType:
        {
            auto parameterGroupType = as<IRUniformParameterGroupType>(bufferType);

            auto layoutTypeOp = parameterGroupType->getDataLayout()
                                    ? parameterGroupType->getDataLayout()->getOp()
                                    : kIROp_DefaultBufferLayoutType;
            return getTypeLayoutRulesFromOp(layoutTypeOp, IRTypeLayoutRules::getStd140());
        }
    case kIROp_GLSLShaderStorageBufferType:
        {
            auto storageBufferType = as<IRGLSLShaderStorageBufferType>(bufferType);
            auto layoutTypeOp = storageBufferType->getDataLayout()
                                    ? storageBufferType->getDataLayout()->getOp()
                                    : kIROp_Std430BufferLayoutType;
            return getTypeLayoutRulesFromOp(layoutTypeOp, IRTypeLayoutRules::getStd430());
        }
    case kIROp_PtrType:
        return IRTypeLayoutRules::getNatural();
    }
    return IRTypeLayoutRules::getNatural();
}

TypeLoweringConfig getTypeLoweringConfigForBuffer(TargetProgram* target, IRType* bufferType)
{
    AddressSpace addrSpace = AddressSpace::Generic;
    if (auto ptrType = as<IRPtrTypeBase>(bufferType))
    {
        switch (ptrType->getAddressSpace())
        {
        case AddressSpace::Input:
        case AddressSpace::Output:
            addrSpace = AddressSpace::Input;
            break;
        case AddressSpace::UserPointer:
            addrSpace = AddressSpace::UserPointer;
            break;
        }
    }
    auto rules = getTypeLayoutRuleForBuffer(target, bufferType);
    return TypeLoweringConfig{addrSpace, rules};
}

} // namespace Slang
