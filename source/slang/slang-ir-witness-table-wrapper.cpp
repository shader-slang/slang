// slang-ir-witness-table-wrapper.cpp
#include "slang-ir-witness-table-wrapper.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{
    struct GenericsLoweringContext;

    struct GenerateWitnessTableWrapperContext
    {
        SharedGenericsLoweringContext* sharedContext;

        IRStringLit* _getWitnessTableWrapperFuncName(IRFunc* func)
        {
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(func);
            if (auto linkageDecoration = func->findDecoration<IRLinkageDecoration>())
            {
                return builder->getStringValue((String(linkageDecoration->getMangledName()) + "_wtwrapper").getUnownedSlice());
            }
            if (auto namehintDecoration = func->findDecoration<IRNameHintDecoration>())
            {
                return builder->getStringValue((String(namehintDecoration->getName()) + "_wtwrapper").getUnownedSlice());
            }
            return nullptr;
        }

        IRFunc* emitWitnessTableWrapper(IRFunc* func, IRInst* interfaceRequirementVal)
        {
            auto funcTypeInInterface = cast<IRFuncType>(interfaceRequirementVal);

            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(func);

            auto wrapperFunc = builder->createFunc();
            wrapperFunc->setFullType((IRType*)interfaceRequirementVal);
            if (auto name = _getWitnessTableWrapperFuncName(func))
                builder->addNameHintDecoration(wrapperFunc, name);

            builder->setInsertInto(wrapperFunc);
            auto block = builder->emitBlock();
            builder->setInsertInto(block);

            ShortList<IRParam*> params;
            for (UInt i = 0; i < funcTypeInInterface->getParamCount(); i++)
            {
                params.add(builder->emitParam(funcTypeInInterface->getParamType(i)));
            }

            List<IRInst*> args;
            bool callerAllocatesReturnVal = funcTypeInInterface->getResultType()->op == kIROp_VoidType
                && func->getResultType()->op != kIROp_VoidType;
            IRVar* retVar = nullptr;
            if (callerAllocatesReturnVal)
            {
                // If return value is allocated by caller, we need to write the result
                // of the call into a local variable, and copy from that local variable
                // to the address passed in by the caller.
                retVar = builder->emitVar(func->getResultType());
                SLANG_ASSERT(params.getCount() == (Index)(func->getParamCount() + 1));
            }
            else
            {
                SLANG_ASSERT(params.getCount() == (Index)func->getParamCount());
            }
            for (UInt i = 0; i < func->getParamCount(); i++)
            {
                auto wrapperParam = params[i + (callerAllocatesReturnVal ? 1 : 0)];
                // Type of the parameter in interface requirement.
                auto reqParamType = wrapperParam->getDataType();
                // Type of the parameter in the callee.
                auto funcParamType = func->getParamType(i);

                // If the implementation expects a concrete type
                // (either in the form of a pointer for `out`/`inout` parameters,
                // or in the form a a value for `in` parameters, while
                // the interface exposes a raw pointer type (void*),
                // we need to cast the raw pointer type to the appropriate
                // concerete type. (void*->Concrete* / void*->Concrete&).
                if (as<IRRawPointerTypeBase>(reqParamType) &&
                    !as<IRRawPointerTypeBase>(funcParamType))
                {
                    if (as<IRPtrTypeBase>(funcParamType))
                    {
                        // The implementation function expects a pointer to the
                        // concrete type. This is the case for inout/out parameters.
                        auto bitCast = builder->emitBitCast(funcParamType, wrapperParam);
                        args.add(bitCast);
                    }
                    else
                    {
                        // The implementation function expects just a value of the
                        // concrete type. We need to insert a load in this case.
                        auto bitCast = builder->emitBitCast(
                            builder->getPtrType(funcParamType),
                            wrapperParam);
                        auto load = builder->emitLoad(bitCast);
                        args.add(load);
                    }
                }
                else
                {
                    args.add(wrapperParam);
                }
            }
            auto call = builder->emitCallInst(func->getResultType(), func, args);
            if (retVar)
            {
                // If the caller of the wrapper function allocates space,
                // we need to store the result of the call into a local varaible,
                // and then copy the local variable into the caller-provided
                // buffer (params[0]).
                builder->emitStore(retVar, call);
                // The result type of the inner function can only be a concrete type
                // if we reach here. If it is a generic type or generic associated type,
                // it would have already been lowered out during interface lowering and
                // lowerGenericFunction.
                // This means that we can just grab the rtti object from the type directly.
                auto rttiObject = sharedContext->maybeEmitRTTIObject(func->getResultType());
                auto rttiPtr = builder->emitGetAddress(
                    builder->getPtrType(builder->getRTTIType()),
                    rttiObject);
                builder->emitCopy(params[0], retVar, rttiPtr);
                builder->emitReturn();
            }
            else
            {
                if (call->getDataType()->op == kIROp_VoidType)
                    builder->emitReturn();
                else
                    builder->emitReturn(call);
            }
            return wrapperFunc;
        }

        void lowerWitnessTable(IRWitnessTable* witnessTable)
        {
            auto interfaceType = cast<IRInterfaceType>(witnessTable->getConformanceType());
            for (auto child : witnessTable->getChildren())
            {
                auto entry = as<IRWitnessTableEntry>(child);
                if (!entry)
                    continue;
                auto interfaceRequirementVal = sharedContext->findInterfaceRequirementVal(interfaceType, entry->getRequirementKey());
                if (auto ordinaryFunc = as<IRFunc>(entry->getSatisfyingVal()))
                {
                    auto wrapper = emitWitnessTableWrapper(ordinaryFunc, interfaceRequirementVal);
                    entry->satisfyingVal.set(wrapper);
                    sharedContext->addToWorkList(wrapper);
                }
            }
        }

        void processInst(IRInst* inst)
        {
            if (auto witnessTable = as<IRWitnessTable>(inst))
            {
                lowerWitnessTable(witnessTable);
            }
        }

        void processModule()
        {
            // We start by initializing our shared IR building state,
            // since we will re-use that state for any code we
            // generate along the way.
            //
            SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
            sharedBuilder->module = sharedContext->module;
            sharedBuilder->session = sharedContext->module->session;

            sharedContext->addToWorkList(sharedContext->module->getModuleInst());

            while (sharedContext->workList.getCount() != 0)
            {
                // We will then iterate until our work list goes dry.
                //
                while (sharedContext->workList.getCount() != 0)
                {
                    IRInst* inst = sharedContext->workList.getLast();

                    sharedContext->workList.removeLast();
                    sharedContext->workListSet.Remove(inst);

                    processInst(inst);

                    for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                    {
                        sharedContext->addToWorkList(child);
                    }
                }
            }
        }
    };

    void generateWitnessTableWrapperFunctions(SharedGenericsLoweringContext* sharedContext)
    {
        GenerateWitnessTableWrapperContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }

}
