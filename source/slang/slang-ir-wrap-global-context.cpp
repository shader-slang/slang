#include "slang-ir-wrap-global-context.h"

#include "slang-ir-util.h"

namespace Slang
{
    struct WrapGlobalScopeContext
    {
        List<IRFunc*> entryPoints;
        IRStructType* contextType;
        struct GlobalVarInfo
        {
            IRStructKey* key;
        };
        Dictionary<IRInst*, GlobalVarInfo> mapGlobalVarToInfo;
        struct FuncInfo
        {
            IRInst* contextArg;
        };
        Dictionary<IRFunc*, FuncInfo> mapFuncToInfo;
        IRStringLit* findNameHint(IRInst* inst)
        {
            if (auto nameDecor = inst->findDecoration<IRNameHintDecoration>())
                return nameDecor->getNameOperand();
            if (auto linkageDecor = inst->findDecoration<IRLinkageDecoration>())
                return linkageDecor->getMangledNameOperand();
            return nullptr;
        }

        // Move all global parameters to the entry point parameters,
        // and replace them with global variables that are initialized with
        // the entry point parameters.
        void moveGlobalParametersToEntryPoint(IRModule* module)
        {
            Dictionary<IRInst*, IRInst*> mapGlobalParamToGlobalVar;

            IRBuilder builder(module);

            for (auto globalInst : module->getGlobalInsts())
            {
                if (auto globalParam = as<IRGlobalParam>(globalInst))
                {
                    builder.setInsertBefore(globalParam);
                    auto globalVar = builder.createGlobalVar(
                        globalParam->getFullType(),
                        (int)AddressSpace::ThreadLocal);
                    if (auto name = findNameHint(globalParam))
                        builder.addNameHintDecoration(globalVar, name);
                    mapGlobalParamToGlobalVar[globalParam] = globalVar;
                }
            }

            // For every entry point, we need to add a new parameter for each global parameter.
            for (auto entryPoint : entryPoints)
            {
                auto firstBlock = entryPoint->getFirstBlock();
                auto paramInsertPoint = firstBlock->getFirstInst();
                struct ParamInfo
                {
                    IRInst* newParam;
                    IRInst* globalVar;
                };
                List<ParamInfo> newParams;
                for (auto globalParam : mapGlobalParamToGlobalVar)
                {
                    auto newParam = builder.createParam(globalParam.first->getFullType());
                    newParam->insertBefore(paramInsertPoint);

                    newParams.add({newParam, globalParam.second});
                }

                // Insert assignments to the global variables at the start of the entry point.
                builder.setInsertBefore(firstBlock->getFirstOrdinaryInst());
                for (auto& paramInfo : newParams)
                {
                    auto globalVar = paramInfo.globalVar;
                    auto newParam = paramInfo.newParam;
                    builder.emitStore(globalVar, newParam);
                }
            }

            // Replace all uses of global parameters with a load from the global variable.
            for (auto globalParam : mapGlobalParamToGlobalVar)
            {
                auto globalVar = globalParam.second;
                traverseUses(globalParam.first, [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        builder.setInsertBefore(user);
                        auto load = builder.emitLoad(globalParam.first->getFullType(), globalVar);
                        builder.replaceOperand(use, load);
                    });
                globalParam.first->removeAndDeallocate();
            }
        }

        void processModule(IRModule* module)
        {
            IRBuilder builder(module);
            List<IRInst*> instsToRemove;

            List<IRFunc*> functions;

            // Collect all entry points and functions.
            for (auto globalInst : module->getGlobalInsts())
            {
                if (globalInst->findDecoration<IREntryPointDecoration>())
                    entryPoints.add(as<IRFunc>(globalInst));
                if (auto func = as<IRFunc>(globalInst))
                    functions.add(func);
            }

            // Before everything, we need to move all global parameters to the entry point parameters.
            // For each global parameter, e.g. `uniform float4 g;`, with will replace it with a global
            // variable, e.g. `float4 _g;`, and add a new parameter to the each entry point, and copy
            // the value from the entry point parameter to the global variable.
            moveGlobalParametersToEntryPoint(module);

            // The next step is to wrap all global variables in a context type, and pass them around
            // with explicit function parameters.

            // Collect all global variables.
            for (auto globalInst : module->getGlobalInsts())
            {
                if (auto globalVar = as<IRGlobalVar>(globalInst))
                {
                    auto key = builder.createStructKey();

                    if (auto name = findNameHint(globalVar))
                        builder.addNameHintDecoration(key, name);

                    GlobalVarInfo info;
                    info.key = key;
                    mapGlobalVarToInfo[globalVar] = info;
                }
            }
            if (mapGlobalVarToInfo.getCount() == 0)
                return;

            // Create the context type for the global scope.
            contextType = builder.createStructType();
            builder.addNameHintDecoration(contextType, toSlice("_SlangGlobalContext"));
            for (auto& fieldKV : mapGlobalVarToInfo)
            {
                auto ptrType = as<IRPtrTypeBase>(fieldKV.first->getFullType());
                if (!ptrType)
                    continue;
                builder.createStructField(
                    contextType, fieldKV.second.key, ptrType->getValueType());
            }

            // Identify all functions that requires the global scope context.

            // First, add all functions to the work list if it directly uses a global variable.
            List<IRFunc*> funcWorkList;
            HashSet<IRFunc*> funcWorkListSet;
            for (auto& fieldKV : mapGlobalVarToInfo)
            {
                auto globalVar = fieldKV.first;
                for (auto use = globalVar->firstUse; use; use = use->nextUse)
                {
                    if (auto userFunc = getParentFunc(use->getUser()))
                    {
                        if (funcWorkListSet.add(userFunc))
                            funcWorkList.add(userFunc);
                    }
                }
            }

            // Next, propagate the call graph and add all functions that transitively uses a global variable.
            for (Index i = 0; i < funcWorkList.getCount(); i++)
            {
                auto func = funcWorkList[i];
                for (auto use = func->firstUse; use; use = use->nextUse)
                {
                    if (auto call = as<IRCall>(use->getUser()))
                    {
                        if (call->getCallee() != func)
                            continue;
                        if (auto callerFunc = as<IRFunc>(getParentFunc(call)))
                        {
                            if (funcWorkListSet.add(callerFunc))
                                funcWorkList.add(callerFunc);
                        }
                    }
                }
            }

            // Now, everything in funcWorkListSet is a function that requires the global scope context.
            // We go ahead and add the context type as the first parameter to these functions.
            List<IRInst*> newCallArgs;

            auto threadPtrType = builder.getPtrType(kIROp_PtrType, contextType, (int)AddressSpace::ThreadLocal);
            for (auto func : funcWorkListSet)
            {
                auto firstBlock = func->getFirstBlock();
                if (!firstBlock)
                    continue;
                bool isEntryPoint = func->findDecoration<IREntryPointDecoration>() != nullptr;
                FuncInfo funcInfo = {};
                if (isEntryPoint)
                {
                    // If the function is an entry point, we need to declare a local variable to hold the context.
                    setInsertBeforeOrdinaryInst(&builder, firstBlock->getFirstOrdinaryInst());
                    funcInfo.contextArg = builder.emitVar(contextType, (int)AddressSpace::ThreadLocal);
                }
                else
                {
                    // For other functions, we just add the context as the first parameter.
                    builder.setInsertBefore(firstBlock->getFirstInst());
                    funcInfo.contextArg = builder.emitParamAtHead(threadPtrType);
                }
                builder.addNameHintDecoration(funcInfo.contextArg, toSlice("_globalCtx"));

                mapFuncToInfo[func] = funcInfo;

                // Now go through the body of the function and insert the context as the first argument to all calls.
                for (auto block : func->getBlocks())
                {
                    for (auto inst : block->getChildren())
                    {
                        if (auto call = as<IRCall>(inst))
                        {
                            if (funcWorkListSet.contains((IRFunc*)getResolvedInstForDecorations(call->getCallee())))
                            {
                                builder.setInsertBefore(call);
                                newCallArgs.clear();
                                newCallArgs.add(funcInfo.contextArg);
                                for (auto arg : call->getArgsList())
                                    newCallArgs.add(arg);
                                auto newCall = builder.emitCallInst(call->getFullType(), call->getCallee(), newCallArgs);
                                call->replaceUsesWith(newCall);
                                instsToRemove.add(call);
                            }
                        }
                    }
                }
            }

            // Next, we need to replace all accesses to global variables with accesses to the context.
            for (auto globalVarKV : mapGlobalVarToInfo)
            {
                auto globalVar = globalVarKV.first;
                auto key = globalVarKV.second.key;
                traverseUses(globalVar, [&](IRUse* use)
                    {
                        auto user = use->getUser();
                        auto parentFunc = getParentFunc(user);
                        if (!parentFunc)
                            return;
                        auto funcInfo = mapFuncToInfo.tryGetValue(parentFunc);
                        SLANG_ASSERT(funcInfo);

                        auto contextArg = funcInfo->contextArg;
                        builder.setInsertBefore(user);
                        auto replacement = builder.emitFieldAddress(
                            builder.getPtrType(
                                kIROp_PtrType,
                                tryGetPointedToType(&builder, globalVar->getFullType()),
                                (int)AddressSpace::ThreadLocal),
                            contextArg,
                            key);
                        builder.replaceOperand(use, replacement);
                    });
                SLANG_ASSERT(!globalVar->hasUses());
                instsToRemove.add(globalVar);
            }

            // Fix up all function types.
            for (auto func : functions)
            {
                fixUpFuncType(func);
            }

            // Finally, cleanup the IR by removing all the insts scheduled for removal.
            for (auto inst : instsToRemove)
                inst->removeAndDeallocate();
        }
    };

    void wrapGlobalScopeInContextType(IRModule* module)
    {
        WrapGlobalScopeContext context;
        context.processModule(module);
    }
}
