#include "slang-ir-pytorch-cpp-binding.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-diagnostics.h"

namespace Slang
{
static bool getHostReturnTypeImpl(List<IRType*>& elementTypes, IRBuilder& builder, IRType* type)
{
    bool isValid = true;
    if (as<IRVoidType>(type))
        return true;
    if (as<IRBasicType>(type))
        elementTypes.add(type);
    else if (as<IRTorchTensorType>(type))
        elementTypes.add(type);
    else if (auto vectorType = as<IRVectorType>(type))
    {
        auto count = as<IRIntLit>(vectorType->getElementCount());
        if (!count)
        {
            return false;
        }
        for (IRIntegerValue i = 0; i < count->getValue(); i++)
        {
            elementTypes.addRange(vectorType->getElementType());
        }
    }
    else if (auto arrayType = as<IRArrayType>(type))
    {
        auto arraySize = as<IRIntLit>(arrayType->getElementCount());
        if (!arraySize)
        {
            return false;
        }
        List<IRType*> subElementTypes;
        isValid &= getHostReturnTypeImpl(subElementTypes, builder, arrayType->getElementType());
        for (IRIntegerValue i = 0; i < arraySize->getValue(); i++)
        {
            elementTypes.addRange(subElementTypes);
        }
    }
    else if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            isValid &= getHostReturnTypeImpl(elementTypes, builder, field->getFieldType());
        }
    }
    else
    {
        return false;
    }
    return isValid;
}

static IRType* getHostReturnType(IRBuilder& builder, IRType* type)
{
    List<IRType*> types;
    bool isValid = getHostReturnTypeImpl(types, builder, type);
    if (isValid)
        return builder.getTargetTupleType((UInt)types.getCount(), types.getBuffer());
    return nullptr;
}

static void flattenToTupleImpl(List<IRInst*>& result, IRBuilder& builder, IRInst* val)
{
    auto type = val->getDataType();
    if (as<IRVoidType>(type))
        return;
    if (as<IRBasicType>(type))
        result.add(val);
    else if (as<IRTorchTensorType>(type))
        result.add(val);
    else if (auto vectorType = as<IRVectorType>(type))
    {
        auto count = as<IRIntLit>(vectorType->getElementCount());
        if (!count)
        {
            return;
        }
        for (IRIntegerValue i = 0; i < count->getValue(); i++)
        {
            result.add(builder.emitElementExtract(vectorType->getElementType(), builder.getIntValue(builder.getIntType(), i)));
        }
    }
    else if (auto arrayType = as<IRArrayType>(type))
    {
        auto arraySize = as<IRIntLit>(arrayType->getElementCount());
        if (!arraySize)
        {
            return;
        }
        for (IRIntegerValue i = 0; i < arraySize->getValue(); i++)
        {
            auto elementVal = builder.emitElementExtract(val, builder.getIntValue(builder.getIntType(), i));
            flattenToTupleImpl(result, builder, elementVal);
        }
    }
    else if (auto structType = as<IRStructType>(type))
    {
        for (auto field : structType->getFields())
        {
            auto elementVal = builder.emitFieldExtract(field->getFieldType(), val, field->getKey());
            flattenToTupleImpl(result, builder, elementVal);
        }
    }
}

static IRInst* flattenToHostReturnTuple(IRBuilder& builder, IRType* type, IRInst* val)
{
    List<IRInst*> vals;
    flattenToTupleImpl(vals, builder, val);
    return builder.emitMakeTargetTuple(type, (UInt)vals.getCount(), vals.getBuffer());
}

static void generateCppBindingForFunc(IRFunc* func, DiagnosticSink* sink)
{
    IRBuilder builder(func);

    builder.setInsertBefore(func);
    auto hostReturnType = getHostReturnType(builder, func->getResultType());
    if (!hostReturnType)
    {
        sink->diagnose(func->sourceLoc, Diagnostics::invalidTorchKernelReturnType, func->getResultType());
        return;
    }
    List<IRType*> hostParamTypes;
    auto funcType = as<IRFuncType>(func->getDataType());
    for (UInt i = 0; i < funcType->getParamCount(); i++)
    {
        hostParamTypes.add(funcType->getParamType(i));
    }
    auto bindingFuncType = builder.getFuncType(hostParamTypes, hostReturnType);
    func->setFullType(bindingFuncType);

    builder.setInsertBefore(func->getFirstBlock()->getFirstOrdinaryInst());
    auto allocator = builder.emitVar(builder.getType(kIROp_TorchKernelMemoryAllocatorType));

    List<IRInst*> instsToRemove;
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
                auto retVal = flattenToHostReturnTuple(builder, hostReturnType, ret->getVal());
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
