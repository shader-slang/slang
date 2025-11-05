#include "slang-ir-cuda-immutable-load.h"

#include "slang-ir-inst-pass-base.h"
#include "slang-ir-insts.h"
#include "slang-ir-layout.h"
#include "slang-ir-util.h"

namespace Slang
{

enum LoadMethodKind
{
    Func,
    Opcode
};

struct LoadMethod
{
    LoadMethodKind kind = LoadMethodKind::Func;
    union
    {
        IRFunc* func;
        IROp op;
    };
    LoadMethod() { func = nullptr; }
    operator bool() { return kind == LoadMethodKind::Func ? func != nullptr : op != kIROp_Nop; }
    LoadMethod(IRFunc* f)
        : kind(LoadMethodKind::Func), func(f)
    {
    }
    LoadMethod(IROp irop)
        : kind(LoadMethodKind::Opcode), op(irop)
    {
    }
    LoadMethod& operator=(IRFunc* f)
    {
        kind = LoadMethodKind::Func;
        this->func = f;
        return *this;
    }
    LoadMethod& operator=(IROp irop)
    {
        kind = LoadMethodKind::Opcode;
        this->op = irop;
        return *this;
    }
    IRInst* apply(IRBuilder& builder, IRType* resultType, IRInst* operandAddr)
    {
        if (kind == LoadMethodKind::Func)
        {
            return builder.emitCallInst(resultType, func, 1, &operandAddr);
        }
        else
        {
            return builder.emitIntrinsicInst(resultType, op, 1, &operandAddr);
        }
    }
};

struct ImmutableBufferLoadLoweringContext : InstPassBase
{
    Dictionary<IRType*, LoadMethod> loadFuncs;
    TargetProgram* targetProgram;

    IRFunc* createLoadFunc(IRBuilder& builder, IRType* valueType, IRParam*& outParam)
    {
        auto func = builder.createFunc();
        builder.addNameHintDecoration(func, toSlice("slang_ldg"));
        builder.setInsertInto(func);
        auto block = builder.emitBlock();
        auto ptrType = builder.getPtrType(valueType);
        builder.setInsertInto(block);
        outParam = builder.emitParam(ptrType);
        builder.addNameHintDecoration(outParam, toSlice("ptr"));
        func->setFullType(builder.getFuncType(ptrType, valueType));
        return func;
    }

    LoadMethod createLoadFuncForType(IRType* type)
    {
        IRBuilder builder(type);
        builder.setInsertAfter(type);
        switch (type->getOp())
        {
        case kIROp_FloatType:
        case kIROp_HalfType:
        case kIROp_DoubleType:
        case kIROp_Int8Type:
        case kIROp_Int16Type:
        case kIROp_IntType:
        case kIROp_Int64Type:
        case kIROp_IntPtrType:
        case kIROp_UInt8Type:
        case kIROp_UInt16Type:
        case kIROp_UIntType:
        case kIROp_UInt64Type:
        case kIROp_UIntPtrType:
        case kIROp_BoolType:
        case kIROp_CharType:
            return kIROp_CUDALDG;
        case kIROp_VectorType:
            {
                // For vector types that has a direct mapping to CUDA __ldg,
                // use the instruction directly.
                auto vectorType = as<IRVectorType>(type);
                auto elementType = vectorType->getElementType();
                auto elementCount = getIntVal(vectorType->getElementCount());
                IRSizeAndAlignment elementSize;
                getNaturalSizeAndAlignment(
                    targetProgram->getOptionSet(),
                    elementType,
                    &elementSize);
                if (elementCount <= 2)
                    return kIROp_CUDALDG;
                else if (elementCount == 4)
                {
                    switch (elementType->getOp())
                    {
                    case kIROp_FloatType:
                    case kIROp_UIntType:
                    case kIROp_IntType:
                    case kIROp_Int8Type:
                    case kIROp_UInt8Type:
                    case kIROp_Int16Type:
                    case kIROp_UInt16Type:
                        return kIROp_CUDALDG;
                    }
                }
                // For other vector types, we need to generate a function to load its content.
                IRParam* ptrParam = nullptr;
                auto func = createLoadFunc(builder, type, ptrParam);
                List<IRInst*> args;
                for (UInt i = 0; i < (UInt)elementCount; i++)
                {
                    auto elementPtr = builder.emitElementAddress(
                        builder.getPtrType(elementType),
                        ptrParam,
                        builder.getIntValue(builder.getIntType(), i));
                    auto loadedElement =
                        builder.emitIntrinsicInst(elementType, kIROp_CUDALDG, 1, &elementPtr);
                    args.add(loadedElement);
                }
                auto result = builder.emitMakeVector(type, args);
                builder.emitReturn(result);
                return func;
            }
            break;
        case kIROp_MatrixType:
            {
                // For matrix types, we should generate a function to load its content by row or
                // column, depending on the layout.
                auto matrixType = as<IRMatrixType>(type);
                auto elementType = matrixType->getElementType();
                auto rowCount = getIntVal(matrixType->getRowCount());
                auto colCount = getIntVal(matrixType->getColumnCount());
                auto layout = (MatrixLayoutMode)getIntVal(matrixType->getLayout());
                IRParam* ptrParam = nullptr;
                auto func = createLoadFunc(builder, type, ptrParam);
                if (layout == kMatrixLayoutMode_ColumnMajor)
                {
                    // For column major matrix, we can load it by column (vector) directly.
                    auto vectorType = builder.getVectorType(elementType, rowCount);
                    auto vectorPtrType = builder.getPtrType(vectorType);
                    auto elementBasePtr = builder.emitBitCast(vectorPtrType, ptrParam);
                    List<IRInst*> args;
                    for (UInt i = 0; i < (UInt)colCount; i++)
                    {
                        auto colPtr = builder.emitGetOffsetPtr(
                            elementBasePtr,
                            builder.getIntValue(builder.getIntType(), i));
                        auto loadedCol = emitImmutableLoad(builder, colPtr);
                        args.add(loadedCol);
                    }
                    // Rearrange loaded vectors in row-major order.
                    List<IRInst*> elements;
                    for (UInt i = 0; i < (UInt)rowCount; i++)
                    {
                        for (UInt j = 0; j < (UInt)colCount; j++)
                        {
                            elements.add(builder.emitElementExtract(
                                elementType,
                                args[j],
                                builder.getIntValue(builder.getIntType(), i)));
                        }
                    }
                    auto result = builder.emitMakeMatrix(
                        type,
                        (UInt)elements.getCount(),
                        elements.getArrayView().getBuffer());
                    builder.emitReturn(result);
                    return func;
                }
                else
                {
                    // For row major matrix, we can load it by row (vector) directly.
                    auto vectorType = builder.getVectorType(elementType, colCount);
                    auto vectorPtrType = builder.getPtrType(vectorType);
                    auto elementBasePtr = builder.emitBitCast(vectorPtrType, ptrParam);
                    List<IRInst*> args;
                    for (UInt i = 0; i < (UInt)rowCount; i++)
                    {
                        auto rowPtr = builder.emitGetOffsetPtr(
                            elementBasePtr,
                            builder.getIntValue(builder.getIntType(), i));
                        auto loadedRow = emitImmutableLoad(builder, rowPtr);
                        args.add(loadedRow);
                    }
                    auto result =
                        builder.emitMakeMatrix(type, (UInt)args.getCount(), args.getBuffer());
                    builder.emitReturn(result);
                    return func;
                }
            }
            break;
        case kIROp_ArrayType:
            {
                // For array types, we need to generate a function to load its content by element.
                auto arrayType = as<IRArrayType>(type);
                auto elementType = arrayType->getElementType();
                auto elementCount = getIntVal(arrayType->getElementCount());
                IRParam* ptrParam = nullptr;
                auto func = createLoadFunc(builder, type, ptrParam);
                List<IRInst*> args;
                for (UInt i = 0; i < (UInt)elementCount; i++)
                {
                    auto elementPtr = builder.emitElementAddress(
                        builder.getPtrType(elementType),
                        ptrParam,
                        builder.getIntValue(builder.getIntType(), i));
                    auto loadedElement = emitImmutableLoad(builder, elementPtr);
                    if (!loadedElement)
                    {
                        func->removeAndDeallocate();
                        return LoadMethod();
                    }
                    args.add(loadedElement);
                }
                auto result = builder.emitMakeArray(type, (UInt)args.getCount(), args.getBuffer());
                builder.emitReturn(result);
                return func;
            }
        case kIROp_StructType:
            {
                // For struct types, we need to generate a function to load its content by field.
                auto structType = as<IRStructType>(type);
                IRParam* ptrParam = nullptr;
                auto func = createLoadFunc(builder, type, ptrParam);
                List<IRInst*> args;
                for (auto field : structType->getFields())
                {
                    auto fieldType = field->getFieldType();
                    auto fieldPtr = builder.emitFieldAddress(
                        builder.getPtrType(fieldType),
                        ptrParam,
                        field->getKey());
                    auto loadedField = emitImmutableLoad(builder, fieldPtr);
                    if (!loadedField)
                    {
                        func->removeAndDeallocate();
                        return LoadMethod();
                    }
                    args.add(loadedField);
                }
                auto result = builder.emitMakeStruct(type, args);
                builder.emitReturn(result);
                return func;
            }
        }
        return LoadMethod();
    }

    LoadMethod getOrCreateLoadFuncForType(IRType* type)
    {
        if (auto func = loadFuncs.tryGetValue(type))
            return *func;
        auto result = createLoadFuncForType(type);
        loadFuncs[type] = result;
        return result;
    }

    IRInst* emitImmutableLoad(IRBuilder& builder, IRInst* ptr)
    {
        IRType* valueType = tryGetPointedToType(&builder, ptr->getDataType());
        if (!valueType)
            return nullptr;
        auto loadFunc = getOrCreateLoadFuncForType(valueType);
        if (!loadFunc)
            return nullptr;
        return loadFunc.apply(builder, valueType, ptr);
    }

    void processInst(IRInst* inst)
    {
        // For every load instruction we see in the module, if the it is loading from
        // an immutable location, try to lower it into a series of __ldg calls.
        // We need to handle both ordinary loads and structured buffer loads.
        //
        switch (inst->getOp())
        {
        case kIROp_Load:
            {
                auto load = as<IRLoad>(inst);
                if (isPointerToImmutableLocation(getRootAddr(load->getPtr())))
                {
                    IRBuilder builder(load);
                    builder.setInsertBefore(load);
                    if (auto newLoad = emitImmutableLoad(builder, load->getPtr()))
                    {
                        load->replaceUsesWith(newLoad);
                        load->removeAndDeallocate();
                    }
                }
            }
            break;
        case kIROp_StructuredBufferLoad:
            {
                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
                auto ptr = builder.emitRWStructuredBufferGetElementPtr(
                    inst->getOperand(0),
                    inst->getOperand(1));
                if (auto newLoad = emitImmutableLoad(builder, ptr))
                {
                    inst->replaceUsesWith(newLoad);
                    inst->removeAndDeallocate();
                }
                else
                {
                    // For some reason this load cannot be lowered, remove the ptr we just created.
                    ptr->removeAndDeallocate();
                }
            }
            break;
        case kIROp_CUDALDG:
            {
                // Does the load needs lowering? If so insert lowered loads.
                IRBuilder builder(inst);
                builder.setInsertBefore(inst);
                auto ptr = inst->getOperand(0);
                auto valueType = tryGetPointedToType(&builder, ptr->getDataType());
                if (!valueType)
                    break;
                auto loadFunc = getOrCreateLoadFuncForType(valueType);
                if (!loadFunc)
                    break;
                // If the type doesn't need further lowering, we don't need to do anything.
                if (loadFunc.kind == LoadMethodKind::Opcode && loadFunc.op == kIROp_CUDALDG)
                    break;
                auto newLoad = loadFunc.apply(builder, valueType, ptr);
                inst->replaceUsesWith(newLoad);
                inst->removeAndDeallocate();
            }
            break;
        }
    }

    void processModule()
    {
        processAllInsts([&](IRInst* inst) { processInst(inst); });
    }

    ImmutableBufferLoadLoweringContext(IRModule* inModule)
        : InstPassBase(inModule)
    {
    }
};

void lowerImmutableBufferLoadForCUDA(TargetProgram* targetProgram, IRModule* module)
{
    ImmutableBufferLoadLoweringContext context(module);
    context.targetProgram = targetProgram;
    context.processModule();
}

} // namespace Slang
