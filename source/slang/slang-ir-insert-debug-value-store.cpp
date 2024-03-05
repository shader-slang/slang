#include "slang-ir-insert-debug-value-store.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
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

        // Helper func to insert debugValue updates.
        auto setDebugValue = [&](IRInst* debugVar, IRInst* rootVar, IRInst* newValue, ArrayView<IRInst*> accessChain)
            {
                // SPIRV does not allow dynamic indices in DebugValue,
                // so we need to stop the access chain at the first dynamic index.
                Index i = 0;
                for (; i < accessChain.getCount(); i++)
                {
                    if (auto key = as<IRStructKey>(accessChain[i]))
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
                auto compositePtr = builder.emitElementAddress(rootVar, accessChain.head(i));
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
                    auto varInst = getRootAddr(storeInst->getPtr(), accessChain);
                    IRInst* debugVar = nullptr;
                    if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                    {
                        builder.setInsertAfter(storeInst);
                        setDebugValue(debugVar, varInst, storeInst->getVal(), accessChain.getArrayView());
                    }
                }
                else if (auto swizzledStore = as<IRSwizzledStore>(inst))
                {
                    List<IRInst*> accessChain;
                    auto varInst = getRootAddr(swizzledStore->getDest(), accessChain);
                    IRInst* debugVar = nullptr;
                    if (mapVarToDebugVar.tryGetValue(varInst, debugVar))
                    {
                        builder.setInsertAfter(swizzledStore);
                        auto loadVal = builder.emitLoad(swizzledStore->getDest());
                        setDebugValue(debugVar, varInst, loadVal, accessChain.getArrayView());
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
                            setDebugValue(debugVar, varInst, loadVal, accessChain.getArrayView());
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
