#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"
#include "slang-ir-layout.h"
#include "slang-ir-lower-cuda-builtin-types.h"
namespace Slang
{

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
        auto matrixArray = builder.emitFieldExtract(arrayType, packedParam, dataKey);
        List<IRInst*> args;
        args.setCount(rowCount * colCount);
        if (getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR)
        {
            for (IRIntegerValue c = 0; c < colCount; c++)
                for (IRIntegerValue r = 0; r < rowCount; r++)
                    args[(Index)(r * colCount + c)] = builder.emitElementExtract(matrixArray, (Index)(r*colCount + c));
        }
        else
        {
            for (IRIntegerValue c = 0; c < colCount; c++)
                for (IRIntegerValue r = 0; r < rowCount; r++)
                    args[(Index)(c * rowCount + r)] = builder.emitElementExtract(matrixArray, (Index)(r*colCount + c));
        }
        IRInst* result = builder.emitMakeMatrix(matrixType, (UInt)args.getCount(), args.getBuffer());
        builder.emitReturn(result);
        return func;
    }

    IRFunc* createMatrixPackFunc(
        IRMatrixType* matrixType,
        IRStructType* structType,
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

        auto matrixArray = builder.emitMakeArray(arrayType, (UInt)elements.getCount(), elements.getBuffer());
        auto result = builder.emitMakeStruct(structType, 1, &matrixArray);
        builder.emitReturn(result);
        return func;
    }

    IRFunc* createVectorUnpackFunc(
        IRVectorType* vectorType,
        IRStructType* structType,
        IRStructKey* dataKey,
        IRArrayType* arrayType)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto funcType = builder.getFuncType(1, (IRType**)&structType, vectorType);
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("unpackVector"));
        builder.setInsertInto(func);
        builder.emitBlock();
        auto packedParam = builder.emitParam(structType);
        auto packedArray = builder.emitFieldExtract(arrayType, packedParam, dataKey);
        auto count = getIntVal(vectorType->getElementCount());
        List<IRInst*> args;
        args.setCount((Index)count);
        for (IRIntegerValue ii = 0; ii < count; ++ii)
        {
            args[(Index)ii] = builder.emitElementExtract(packedArray, ii);
        }
        auto result = builder.emitMakeVector(vectorType, (UInt)args.getCount(), args.getBuffer());
        builder.emitReturn(result);
        return func;
    }

    IRFunc* createVectorPackFunc(
        IRVectorType* vectorType,
        IRStructType* structType,
        IRArrayType* arrayType)
    {
        IRBuilder builder(structType);
        builder.setInsertAfter(structType);
        auto func = builder.createFunc();
        auto funcType = builder.getFuncType(1, (IRType**)&vectorType, structType);
        func->setFullType(funcType);
        builder.addNameHintDecoration(func, UnownedStringSlice("packVector"));
        builder.setInsertInto(func);
        builder.emitBlock();
        auto originalParam = builder.emitParam(vectorType);
        auto count = getIntVal(vectorType->getElementCount());
        List<IRInst*> args;
        args.setCount((Index)count);
        for (IRIntegerValue ii = 0; ii < count; ++ii)
        {
            args[(Index)ii] = builder.emitElementExtract(originalParam, ii);
        }
        auto packedArray = builder.emitMakeArray(arrayType, (UInt)args.getCount(), args.getBuffer());
        auto result = builder.emitMakeStruct(structType, 1, &packedArray);
        builder.emitReturn(result);
        return func;
    }

    LoweredBuiltinTypeInfo lowerMatrixType(
        IRBuilder* builder,
        IRMatrixType* matrixType,
        String nameSuffix)
    {
        LoweredBuiltinTypeInfo info;

        auto loweredType = builder->createStructType();
        StringBuilder nameSB;
        bool isColMajor = getIntVal(matrixType->getLayout()) == SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
        nameSB << "_MatrixStorage_";
        getTypeNameHint(nameSB, matrixType->getElementType());
        nameSB << getIntVal(matrixType->getRowCount()) << "x" << getIntVal(matrixType->getColumnCount());
        if (isColMajor)
            nameSB << "_ColMajor";
        nameSB << nameSuffix;
        builder->addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());
        auto structKey = builder->createStructKey();
        builder->addNameHintDecoration(structKey, UnownedStringSlice("data"));
        //auto vectorType = builder->getVectorType(matrixType->getElementType(),
        //    isColMajor?matrixType->getRowCount():matrixType->getColumnCount());

        auto arrayType =
            builder->getArrayType(
                matrixType->getElementType(),
                builder->getIntValue(
                    builder->getUIntType(), 
                    getIntVal(matrixType->getColumnCount()) * getIntVal(matrixType->getRowCount())));

        builder->createStructField(loweredType, structKey, arrayType);

        info.loweredType = loweredType;
        info.loweredInnerArrayType = arrayType;
        info.loweredInnerStructKey = structKey;
        info.convertLoweredToOriginal = createMatrixUnpackFunc(matrixType, loweredType, structKey, arrayType);
        info.convertOriginalToLowered = createMatrixPackFunc(matrixType, loweredType, arrayType);
        return info;
    }

    LoweredBuiltinTypeInfo lowerVectorType(
        IRBuilder* builder,
        IRVectorType* vectorType,
        String nameSuffix)
    {
        LoweredBuiltinTypeInfo info;

        auto loweredType = builder->createStructType();

        StringBuilder nameSB;
        nameSB << "_VectorStorage_";
        getTypeNameHint(nameSB, vectorType->getElementType());
        nameSB << getIntVal(vectorType->getElementCount()) << "_";
        nameSB << nameSuffix;
        builder->addNameHintDecoration(loweredType, nameSB.produceString().getUnownedSlice());

        info.loweredType = loweredType;
        auto structKey = builder->createStructKey();
        builder->addNameHintDecoration(structKey, UnownedStringSlice("data"));

        auto arrayType = builder->getArrayType(
            vectorType->getElementType(),
            vectorType->getElementCount());

        builder->createStructField(loweredType, structKey, arrayType);

        info.convertLoweredToOriginal = createVectorUnpackFunc(vectorType, loweredType, structKey, arrayType);
        info.convertOriginalToLowered = createVectorPackFunc(vectorType, loweredType, arrayType);

        return info;
    }
};