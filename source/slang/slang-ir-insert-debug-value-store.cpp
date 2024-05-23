#include "slang-ir-insert-debug-value-store.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
    struct DebugValueStoreContext
    {
        Dictionary<IRType*, bool> m_mapTypeToDebugability;
        bool isTypeKind(IRInst* inst)
        {
            if (!inst)
                return true;
            switch (inst->getOp())
            {
            case kIROp_TypeKind:
            case kIROp_TypeType:
                return true;
            default:
                return false;
            }
        }
        bool isDebuggableType(IRType* type)
        {
            if (bool* result = m_mapTypeToDebugability.tryGetValue(type))
                return *result;

            bool debuggable = false;
            switch (type->getOp())
            {
            case kIROp_VoidType:
                break;
            case kIROp_StructType:
            {
                auto structType = static_cast<IRStructType*>(type);
                bool structDebuggable = true;
                for (auto field : structType->getFields())
                {
                    if (!isDebuggableType(field->getFieldType()))
                    {
                        structDebuggable = false;
                        break;
                    }
                }
                debuggable = structDebuggable;
                break;
            }
            case kIROp_ArrayType:
            case kIROp_UnsizedArrayType:
            {
                auto arrayType = static_cast<IRArrayTypeBase*>(type);
                debuggable = isDebuggableType(arrayType->getElementType());
                break;
            }
            case kIROp_VectorType:
            case kIROp_MatrixType:
            case kIROp_PtrType:
                debuggable = true;
                break;
            case kIROp_Param:
                // Assume generic parameters are debuggable.
                debuggable = true;
                break;
            case kIROp_Specialize:
            {
                auto specType = as<IRSpecialize>(type);
                auto specTypeDebuggable = isDebuggableType((IRType*)getResolvedInstForDecorations(specType));
                if (!specTypeDebuggable)
                    break;
                for (UInt i = 0; i < specType->getArgCount(); i++)
                {
                    auto arg = specType->getArg(i);
                    if (isTypeKind(arg->getDataType()) && !isDebuggableType((IRType*)specType->getArg(i)))
                    {
                        specTypeDebuggable = false;
                        break;
                    }
                }
                debuggable = false;// specTypeDebuggable;
                break;
            }
            default:
                if (as<IRBasicType>(type))
                    debuggable = true;
                break;
            }
            m_mapTypeToDebugability[type] = debuggable;
            return debuggable;
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
                if (!isDebuggableType(paramType))
                    continue;
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
                            auto varType = tryGetPointedToType(&builder, varInst->getDataType());
                            builder.setInsertBefore(varInst);
                            if (!isDebuggableType(varType))
                                continue;
                            auto debugVar = builder.emitDebugVar(
                                varType,
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

            // Helper func to insert debugValue updates.
            auto setDebugValue = [&](
                IRInst* debugVar, IRInst* rootVar, IRInst* newValue,
                ArrayView<IRInst*> accessChain, ArrayView<IRInst*> types)
                {
                    // SPIRV does not allow dynamic indices in DebugValue,
                    // so we need to stop the access chain at the first dynamic index.
                    Index i = 0;
                    for (; i < accessChain.getCount(); i++)
                    {
                        if (as<IRStructKey>(accessChain[i]))
                        {
                            continue;
                        }
                        if (as<IRIntLit>(accessChain[i]))
                        {
                            continue;
                        }
                        break;
                    }
                    // If everything is static on the access chain, we can simply emit a DebugValue.
                    if (i == accessChain.getCount())
                    {
                        builder.emitDebugValue(debugVar, newValue, accessChain);
                        return;
                    }

                    // Otherwise we need to load the entire composite value starting at the dynamic index access chain
                    // and set it.
                    auto compositePtr = builder.emitElementAddress(rootVar, accessChain.head(i), types.head(i));
                    auto compositeVal = builder.emitLoad(compositePtr);
                    builder.emitDebugValue(debugVar, compositeVal, accessChain.head(i));
                };
            for (auto block : func->getBlocks())
            {
                IRInst* nextInst = nullptr;
                for (auto inst = block->getFirstInst(); inst; inst = nextInst)
                {
                    nextInst = inst->getNextInst();

                    if (auto storeInst = as<IRStore>(inst))
                    {
                        List<IRInst*> accessChain;
                        List<IRInst*> types;
                        auto varInst = getRootAddr(storeInst->getPtr(), accessChain, &types);
                        IRInst* debugVar = nullptr;
                        if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                        {
                            builder.setInsertAfter(storeInst);
                            setDebugValue(debugVar, varInst, storeInst->getVal(), accessChain.getArrayView(), types.getArrayView());
                        }
                    }
                    else if (auto swizzledStore = as<IRSwizzledStore>(inst))
                    {
                        List<IRInst*> accessChain;
                        List<IRInst*> types;
                        auto varInst = getRootAddr(swizzledStore->getDest(), accessChain, &types);
                        IRInst* debugVar = nullptr;
                        if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                        {
                            builder.setInsertAfter(swizzledStore);
                            auto loadVal = builder.emitLoad(swizzledStore->getDest());
                            setDebugValue(debugVar, varInst, loadVal, accessChain.getArrayView(), types.getArrayView());
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
                            List<IRInst*> types;
                            auto varInst = getRootAddr(arg, accessChain, &types);
                            IRInst* debugVar = nullptr;
                            if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                            {
                                builder.setInsertAfter(callInst);
                                auto loadVal = builder.emitLoad(arg);
                                setDebugValue(debugVar, varInst, loadVal, accessChain.getArrayView(), types.getArrayView());
                            }
                        }
                    }
                }
            }
        }
    };

    void insertDebugValueStore(IRModule* module)
    {
        DebugValueStoreContext context;
        for (auto globalInst : module->getGlobalInsts())
        {
            if (auto genericInst = as<IRGeneric>(globalInst))
            {
                if (auto func = as<IRFunc>(findGenericReturnVal(genericInst)))
                {
                    context.insertDebugValueStore(func);
                }
            }
            else if (auto func = as<IRFunc>(globalInst))
            {
                context.insertDebugValueStore(func);
            }
        }
    }
}
