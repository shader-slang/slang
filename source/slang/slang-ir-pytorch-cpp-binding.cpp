#include "slang-ir-pytorch-cpp-binding.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-diagnostics.h"

namespace Slang
{
// Convert a type to a target tuple type.
static IRType* translateToTupleType(IRBuilder& builder, IRType* type)
{
    if (as<IRVoidType>(type))
        return type;
    if (as<IRBasicType>(type))
        return type;
    else if (as<IRTorchTensorType>(type))
        return type;
    else if (auto vectorType = as<IRVectorType>(type))
    {
        auto count = as<IRIntLit>(vectorType->getElementCount());
        if (!count)
        {
            return nullptr;
        }
        List<IRType*> elementTypes;
        for (IRIntegerValue i = 0; i < count->getValue(); i++)
        {
            elementTypes.addRange(vectorType->getElementType());
        }
        return builder.getTargetTupleType((UInt)elementTypes.getCount(), elementTypes.getBuffer());
    }
    else if (auto arrayType = as<IRArrayType>(type))
    {
        auto arraySize = as<IRIntLit>(arrayType->getElementCount());
        if (!arraySize)
        {
            return nullptr;
        }
        List<IRType*> subElementTypes;
        auto subElementType = translateToTupleType(builder, arrayType->getElementType());
        for (IRIntegerValue i = 0; i < arraySize->getValue(); i++)
        {
            subElementTypes.addRange(subElementType);
        }
        return builder.getTargetTupleType((UInt)subElementTypes.getCount(), subElementTypes.getBuffer());
    }
    else if (auto structType = as<IRStructType>(type))
    {
        List<IRType*> elementTypes;
        for (auto field : structType->getFields())
        {
            auto fieldType = translateToTupleType(builder, field->getFieldType());
            if (!fieldType)
            {
                return nullptr;
            }
            elementTypes.addRange(fieldType);
        }
        return builder.getTargetTupleType((UInt)elementTypes.getCount(), elementTypes.getBuffer());
    }
    else
    {
        return nullptr;
    }
}

// Convert a value to a target tuple type.
static IRInst* makeTargetTuple(IRBuilder& builder, IRInst* val)
{
    auto type = val->getDataType();
    if (as<IRVoidType>(type))
        return val;
    if (as<IRBasicType>(type))
        return val;
    else if (as<IRTorchTensorType>(type))
        return val;
    else if (auto vectorType = as<IRVectorType>(type))
    {
        auto count = as<IRIntLit>(vectorType->getElementCount());
        if (!count)
        {
            return nullptr;
        }
        List<IRInst*> resultElements;
        List<IRType*> elementTypes;
        for (IRIntegerValue i = 0; i < count->getValue(); i++)
        {
            auto elementVal = builder.emitElementExtract(val, builder.getIntValue(builder.getIntType(), i));
            auto tupleElement = makeTargetTuple(builder, elementVal);
            if (!tupleElement)
                return nullptr;
            resultElements.add(tupleElement);
            elementTypes.add(tupleElement->getFullType());
        }
        auto resultType = builder.getTargetTupleType((UInt)elementTypes.getCount(), elementTypes.getBuffer());
        return builder.emitMakeTargetTuple(resultType, (UInt)resultElements.getCount(), resultElements.getBuffer());
    }
    else if (auto arrayType = as<IRArrayType>(type))
    {
        auto arraySize = as<IRIntLit>(arrayType->getElementCount());
        if (!arraySize)
        {
            return nullptr;
        }
        List<IRInst*> resultElements;
        List<IRType*> elementTypes;
        for (IRIntegerValue i = 0; i < arraySize->getValue(); i++)
        {
            auto elementVal = builder.emitElementExtract(val, builder.getIntValue(builder.getIntType(), i));
            auto tupleElement = makeTargetTuple(builder, elementVal);
            if (!tupleElement)
                return nullptr;
            resultElements.add(tupleElement);
            elementTypes.add(tupleElement->getFullType());
        }
        auto resultType = builder.getTargetTupleType((UInt)elementTypes.getCount(), elementTypes.getBuffer());
        return builder.emitMakeTargetTuple(resultType, (UInt)resultElements.getCount(), resultElements.getBuffer());
    }
    else if (auto structType = as<IRStructType>(type))
    {
        List<IRInst*> resultElements;
        List<IRType*> elementTypes;
        for (auto field : structType->getFields())
        {
            auto elementVal = builder.emitFieldExtract(field->getFieldType(), val, field->getKey());
            auto tupleElement = makeTargetTuple(builder, elementVal);
            if (!tupleElement)
                return nullptr;
            resultElements.add(tupleElement);
            elementTypes.add(tupleElement->getFullType());
        }
        auto resultType = builder.getTargetTupleType((UInt)elementTypes.getCount(), elementTypes.getBuffer());
        return builder.emitMakeTargetTuple(resultType, (UInt)resultElements.getCount(), resultElements.getBuffer());
    }
    else
    {
        return nullptr;
    }
}

// Convert a target tuple type to a value.
static IRInst* makeValueFromTargetTuple(IRBuilder& builder, IRType* type, IRInst* val)
{
    if (as<IRVoidType>(type))
        return val;
    if (as<IRBasicType>(type))
        return val;
    else if (as<IRTorchTensorType>(type))
        return val;
    else if (auto vectorType = as<IRVectorType>(type))
    {
        auto count = as<IRIntLit>(vectorType->getElementCount());
        if (!count)
        {
            return nullptr;
        }
        List<IRInst*> resultElements;
        auto elementType = vectorType->getElementType();
        for (IRIntegerValue i = 0; i < count->getValue(); i++)
        {
            auto tupleElement = builder.emitTargetTupleGetElement(elementType, val, builder.getIntValue(builder.getIntType(), i));
            auto convertedElement = makeValueFromTargetTuple(builder, elementType, tupleElement);
            if (!convertedElement)
                return nullptr;
            resultElements.add(convertedElement);
        }
        return builder.emitMakeVector(type, (UInt)resultElements.getCount(), resultElements.getBuffer());
    }
    else if (auto arrayType = as<IRArrayType>(type))
    {
        auto arraySize = as<IRIntLit>(arrayType->getElementCount());
        if (!arraySize)
        {
            return nullptr;
        }
        List<IRInst*> resultElements;
        auto elementType = arrayType->getElementType();
        for (IRIntegerValue i = 0; i < arraySize->getValue(); i++)
        {
            auto tupleElement = builder.emitTargetTupleGetElement(elementType, val, builder.getIntValue(builder.getIntType(), i));
            auto convertedElement = makeValueFromTargetTuple(builder, elementType, tupleElement);
            if (!convertedElement)
                return nullptr;
            resultElements.add(convertedElement);
        }
        return builder.emitMakeArray(type, (UInt)resultElements.getCount(), resultElements.getBuffer());
    }
    else if (auto structType = as<IRStructType>(type))
    {
        List<IRInst*> resultElements;
        IRIntegerValue i = 0;
        for (auto field : structType->getFields())
        {
            auto tupleElement = builder.emitTargetTupleGetElement(field->getFieldType(), val, builder.getIntValue(builder.getIntType(), i));
            auto convertedElement = makeValueFromTargetTuple(builder, field->getFieldType(), tupleElement);
            if (!convertedElement)
                return nullptr;
            resultElements.add(convertedElement);
            i++;
        }
        return builder.emitMakeStruct(type, (UInt)resultElements.getCount(), resultElements.getBuffer());
    }
    else
    {
        return nullptr;
    }
}

static void generateCppBindingForFunc(IRFunc* func, DiagnosticSink* sink)
{
    IRBuilder builder(func);

    builder.setInsertBefore(func);
    auto hostReturnType = translateToTupleType(builder, func->getResultType());
    if (!hostReturnType)
    {
        sink->diagnose(func->sourceLoc, Diagnostics::invalidTorchKernelReturnType, func->getResultType());
        return;
    }
    List<IRType*> hostParamTypes;
    auto funcType = as<IRFuncType>(func->getDataType());
    for (UInt i = 0; i < funcType->getParamCount(); i++)
    {
        hostParamTypes.add(translateToTupleType(builder, funcType->getParamType(i)));
    }
    auto bindingFuncType = builder.getFuncType(hostParamTypes, hostReturnType);
    func->setFullType(bindingFuncType);

    builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());

    List<IRInst*> instsToRemove;
    List<IRInst*> oldParams;
    for (auto param : func->getFirstBlock()->getParams())
    {
        oldParams.add(param);
    }

    List<IRInst*> newParams;
    for (auto param : oldParams)
    {
        auto paramType = param->getFullType();
        auto newParamType = translateToTupleType(builder, paramType);
        if (!newParamType)
        {
            sink->diagnose(param->sourceLoc, Diagnostics::invalidTorchKernelParamType, paramType);
            return;
        }
        auto newParam = builder.emitParam(newParamType);
        newParams.add(newParam);
    }

    // Convert all new parameters from tuples to their original types.
    for (Index i = 0; i < newParams.getCount(); i++)
    {
        auto oldParam = oldParams[i];
        auto newParam = newParams[i];
        auto convertedParam = makeValueFromTargetTuple(builder, oldParam->getFullType(), newParam);
        if (!convertedParam)
        {
            return;
        }
        oldParam->replaceUsesWith(convertedParam);
        oldParam->removeAndDeallocate();
    }

    auto allocator = builder.emitVar(builder.getType(kIROp_TorchKernelMemoryAllocatorType));

    for (auto block : func->getBlocks())
    {
        for (auto inst : block->getChildren())
        {
            if (auto kernelDispatch = as<IRDispatchKernel>(inst))
            {
                builder.setInsertBefore(kernelDispatch);
                List<IRInst*> kernelArgs;
                auto kernelArgCount = kernelDispatch->getArgCount();
                auto argArrayType = builder.getArrayType(builder.getPtrType(builder.getVoidType()),
                    builder.getIntValue(builder.getIntType(), kernelArgCount));
                auto argArrayVar = builder.emitVar(argArrayType);
                for (UInt i = 0; i < kernelArgCount; i++)
                {
                    auto arg = kernelDispatch->getArg(i);
                    auto argVar = builder.emitVar(arg->getFullType());
                    builder.emitStore(argVar, arg);
                    auto addr = builder.emitElementAddress(argArrayVar, builder.getIntValue(builder.getIntType(), i));
                    builder.emitStore(addr, argVar);
                }
                auto argArrayPtr = builder.emitElementAddress(argArrayVar, builder.getIntValue(builder.getIntType(), 0));
                builder.emitCudaKernelLaunch(
                    kernelDispatch->getBaseFn(),
                    kernelDispatch->getDispatchSize(),
                    kernelDispatch->getThreadGroupSize(),
                    argArrayPtr,
                    builder.emitGetTorchCudaStream());
                instsToRemove.add(inst);
            }
            else if (auto getView = as<IRTorchTensorGetView>(inst))
            {
                builder.setInsertBefore(getView);
                auto makeView = builder.emitMakeTensorView(getView->getFullType(), allocator, inst->getOperand(0));
                getView->replaceUsesWith(makeView);
                instsToRemove.add(getView);
            }
            else if (auto ret = as<IRReturn>(inst))
            {
                builder.setInsertBefore(ret);
                auto retVal = makeTargetTuple(builder, ret->getVal());
                ret->setOperand(0, retVal);
            }
        }
    }

    for (auto inst : instsToRemove)
        inst->removeAndDeallocate();
}

void generatePyTorchCppBinding(IRModule* module, DiagnosticSink* sink)
{
    List<IRFunc*> workList;
    List<IRFunc*> cudaKernels;
    for (auto globalInst : module->getGlobalInsts())
    {
        auto func = as<IRFunc>(globalInst);
        if (!func)
            continue;
        if (func->findDecoration<IRTorchEntryPointDecoration>())
        {
            workList.add(func);
        }
        else if (func->findDecoration<IRCudaKernelDecoration>())
        {
            cudaKernels.add(func);
        }
        else
        {
            // Remove all other export decorations if this is not a cuda host func.
            if (auto decor = func->findDecoration<IRPublicDecoration>())
                decor->removeAndDeallocate();
            if (auto decor = func->findDecoration<IRHLSLExportDecoration>())
                decor->removeAndDeallocate();
            if (auto decor = func->findDecoration<IRKeepAliveDecoration>())
                decor->removeAndDeallocate();
            if (auto decor = func->findDecoration<IRDllExportDecoration>())
                decor->removeAndDeallocate();
        }
    }

    for (auto func : workList)
        generateCppBindingForFunc(func, sink);

    for (auto func : cudaKernels)
    {
        for (auto block = func->getFirstBlock(); block;)
        {
            auto nextBlock = block->getNextBlock();
            block->removeAndDeallocate();
            block = nextBlock;
        }
    }
}

// Remove all [TorchEntryPoint] functions when emitting CUDA source.
void removeTorchKernels(IRModule* module)
{
    for (auto globalInst : module->getGlobalInsts())
    {
        if (!as<IRFunc>(globalInst))
            continue;
        if (globalInst->findDecoration<IRTorchEntryPointDecoration>())
            globalInst->removeAndDeallocate();
    }

}

}
