#include "slang-ir-insert-debug-value-store.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
    bool isValidDebugType(IRType* type)
    {
        type = (IRType*)unwrapAttributedType(type);
        if (as<IRBasicType>(type))
            return true;
        switch (type->getOp())
        {
        case kIROp_StructType:
        case kIROp_VectorType:
        case kIROp_MatrixType:
        case kIROp_InterfaceType:
        case kIROp_ExtractExistentialType:
        case kIROp_OptionalType:
        case kIROp_ArrayType:
        case kIROp_UnsizedArrayType:
        case kIROp_PtrType:
        case kIROp_HLSLStructuredBufferType:
        case kIROp_HLSLRWStructuredBufferType:
        case kIROp_ConstantBufferType:
        case kIROp_ParameterBlockType:
        case kIROp_GLSLShaderStorageBufferType:
            return true;
        default:
            return false;
        }
    }
    void insertDebugValueStore(IRFunc* func)
    {
        IRBuilder builder(func);
        Dictionary<IRInst*, IRInst*> mapVarToDebugVar;
        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;
        auto funcDebugLoc = func->findDecoration<IRDebugLocationDecoration>();
        if (!funcDebugLoc)
            return;
        List<IRInst*> params;
        for (auto param : firstBlock->getParams())
        {
            params.add(param);
        }
        Index paramIndex = 0;
        for (auto param : params)
        {
            builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
            auto paramType = param->getDataType();
            bool isRefParam = false;
            if (auto outType = as<IROutTypeBase>(paramType))
            {
                isRefParam = true;
                paramType = outType->getValueType();
            }
            if (!isValidDebugType(paramType))
            {
                paramIndex++;
                continue;
            }
            auto debugVar = builder.emitDebugVar(
                paramType,
                funcDebugLoc->getSource(),
                funcDebugLoc->getLine(),
                funcDebugLoc->getCol(),
                builder.getIntValue(builder.getUIntType(), paramIndex));
            copyNameHintAndDebugDecorations(debugVar, param);

            mapVarToDebugVar[param] = debugVar;
            
            // Store the initial value of the parameter into the debug var.
            IRInst* paramVal = nullptr;
            if (!isRefParam)
                paramVal = param;
            else if (as<IRInOutType>(param->getDataType()))
                paramVal = builder.emitLoad(param);
            if (paramVal)
            {
                ArrayView<IRInst*> accessChain;
                builder.emitDebugValue(debugVar, paramVal, accessChain);
            }
            paramIndex++;
        }

        for (auto block : func->getBlocks())
        {
            IRInst* nextInst = nullptr;
            for (auto inst = block->getFirstInst(); inst; inst = nextInst)
            {
                nextInst = inst->getNextInst();
                if (auto varInst = as<IRVar>(inst))
                {
                    if (auto debugLoc = varInst->findDecoration<IRDebugLocationDecoration>())
                    {
                        if (!isValidDebugType(varInst->getDataType()))
                        {
                            continue;
                        }
                        builder.setInsertBefore(varInst);
                        auto debugVar = builder.emitDebugVar(
                            tryGetPointedToType(&builder, varInst->getDataType()),
                            debugLoc->getSource(),
                            debugLoc->getLine(),
                            debugLoc->getCol());
                        copyNameHintAndDebugDecorations(debugVar, varInst);
                        mapVarToDebugVar[varInst] = debugVar;
                    }   
                }
            }
        }

        // Collect all stores and insert debug value insts to update debug vars.
        for (auto block : func->getBlocks())
        {
            IRInst* nextInst = nullptr;
            for (auto inst = block->getFirstInst(); inst; inst = nextInst)
            {
                nextInst = inst->getNextInst();

                if (auto storeInst = as<IRStore>(inst))
                {
                    List<IRInst*> accessChain;
                    auto varInst = getRootAddr(storeInst->getPtr(), accessChain);
                    IRInst* debugVar = nullptr;
                    if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                    {
                        builder.setInsertAfter(storeInst);
                        builder.emitDebugValue(debugVar, storeInst->getVal(), accessChain.getArrayView());
                    }
                }
                else if (auto callInst = as<IRCall>(inst))
                {
                    auto funcValue = getResolvedInstForDecorations(callInst->getCallee());
                    if (!funcValue)
                        continue;
                    for (UInt i = 0; i < callInst->getArgCount(); i++)
                    {
                        auto arg = callInst->getArg(i);
                        if (!as<IRPtrTypeBase>(arg->getDataType()))
                            continue;
                        List<IRInst*> accessChain;
                        auto varInst = getRootAddr(arg, accessChain);
                        IRInst* debugVar = nullptr;
                        if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                        {
                            builder.setInsertAfter(callInst);
                            auto loadVal = builder.emitLoad(arg);
                            builder.emitDebugValue(debugVar, loadVal, accessChain.getArrayView());
                        }
                    }
                }
            }
        }
    }

    void insertDebugValueStore(IRModule* module)
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto genericInst = as<IRGeneric>(globalInst))
            {
                if (auto func = as<IRFunc>(findGenericReturnVal(genericInst)))
                {
                    insertDebugValueStore(func);
                }
            }
            else if (auto func = as<IRFunc>(globalInst))
            {
                insertDebugValueStore(func);
            }
        }
    }
}
