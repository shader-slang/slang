// slang-ir-lower-generics.cpp
#include "slang-ir-lower-generics.h"

#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{
    struct GenericsLoweringContext;

    struct GenericsLoweringContext
    {
        // For convenience, we will keep a pointer to the module
        // we are processing.
        IRModule* module;

        Dictionary<IRInst*, IRInst*> loweredGenericFunctions;

        SharedIRBuilder sharedBuilderStorage;

        // We will use a single work list of instructions that need
        // to be considered for lowering.
        //
        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;

        void addToWorkList(
            IRInst* inst)
        {
            // We will ignore any code that is nested under a generic,
            // because they will be recursively processed through specialized
            // call sites.
            //
            for (auto ii = inst->getParent(); ii; ii = ii->getParent())
            {
                if (as<IRGeneric>(ii))
                    return;
            }

            if (workListSet.Contains(inst))
                return;

            workList.add(inst);
            workListSet.Add(inst);
        }

        IRInst* lowerGenericFunction(IRInst* genericValue)
        {
            IRInst* result = nullptr;
            if (loweredGenericFunctions.TryGetValue(genericValue, result))
                return result;
            auto genericParent = as<IRGeneric>(genericValue);
            SLANG_ASSERT(genericParent);
            auto func = as<IRFunc>(findGenericReturnVal(genericParent));
            SLANG_ASSERT(func);
            if (!func->isDefinition())
            {
                loweredGenericFunctions[genericValue] = genericValue;
                return genericValue;
            }
            IRCloneEnv cloneEnv;
            IRBuilder builder;
            builder.sharedBuilder = &sharedBuilderStorage;
            builder.setInsertBefore(genericParent);
            auto loweredFunc = cloneInstAndOperands(&cloneEnv, &builder, func);
            List<IRInst*> clonedParams;
            for (auto genericParam : genericParent->getParams())
            {
                auto clonedParam = cloneInst(&cloneEnv, &builder, genericParam);
                cloneEnv.mapOldValToNew[genericParam] = clonedParam;
                clonedParams.add(clonedParam);
            }
            cloneInstDecorationsAndChildren(&cloneEnv, &sharedBuilderStorage, func, loweredFunc);
            auto block = as<IRBlock>(loweredFunc->getFirstChild());
            for (auto param : clonedParams)
            {
                param->removeFromParent();
                block->addParam(as<IRParam>(param));
            }
            loweredGenericFunctions[genericValue] = loweredFunc;
            // Turn generic parameters into void pointers.
            for (auto param : cast<IRFunc>(loweredFunc)->getParams())
            {
                if (param->findDecoration<IRPolymorphicDecoration>())
                {
                    param->setFullType(builder.getPtrType(builder.getVoidType()));
                }
            }
            addToWorkList(loweredFunc);
            return loweredFunc;
        }

        void processInst(IRInst* inst)
        {
            if (auto callInst = as<IRCall>(inst))
            {
                // If we see a call(specialize(gFunc, Targs), args),
                // translate it into call(gFunc, args, Targs).
                auto funcOperand = callInst->getOperand(0);
                if (auto specializeInst = as<IRSpecialize>(funcOperand))
                {
                    auto loweredFunc = lowerGenericFunction(specializeInst->getOperand(0));
                    if (loweredFunc == specializeInst->getOperand(0))
                    {
                        // This is an intrinsic function, don't transform.
                        return;
                    }
                    IRBuilder builderStorage;
                    auto builder = &builderStorage;
                    builder->sharedBuilder = &sharedBuilderStorage;
                    builder->setInsertBefore(inst);
                    List<IRInst*> args;
                    auto pp = as<IRFunc>(loweredFunc)->getParams().begin();
                    auto voidPtrType = builder->getPtrType(builder->getVoidType());
                    for (UInt i = 0; i < callInst->getArgCount(); i++)
                    {
                        auto arg = callInst->getArg(i);
                        if ((*pp)->getDataType() == voidPtrType &&
                            arg->getDataType() != voidPtrType)
                        {
                            // We are calling a generic function that with an argument of
                            // concrete type. We need to convert this argument o void*.

                            // Ideally this should just be a GetElementAddress inst.
                            // However the current code emitting logic for this instruction
                            // doesn't truly respect the pointerness and does not produce
                            // what we needed. For now we use another instruction here
                            // to keep changes minimal.
                            arg = builder->emitGetAddress(
                                voidPtrType,
                                arg);
                        }
                        args.add(arg);
                        ++pp;
                    }
                    for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                        args.add(specializeInst->getArg(i));
                    auto newCall = builder->emitCallInst(callInst->getFullType(), loweredFunc, args);
                    callInst->replaceUsesWith(newCall);
                    callInst->removeAndDeallocate();
                }
            }
        }

        void processModule()
        {
            // We start by initializing our shared IR building state,
            // since we will re-use that state for any code we
            // generate along the way.
            //
            SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
            sharedBuilder->module = module;
            sharedBuilder->session = module->session;

            addToWorkList(module->getModuleInst());

            while (workList.getCount() != 0)
            {
                // We will then iterate until our work list goes dry.
                //
                while (workList.getCount() != 0)
                {
                    IRInst* inst = workList.getLast();

                    workList.removeLast();
                    workListSet.Remove(inst);

                    processInst(inst);

                    for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
                    {
                        addToWorkList(child);
                    }
                }
            }
        }
    };

    void lowerGenerics(
        IRModule* module)
    {
        GenericsLoweringContext context;
        context.module = module;
        context.processModule();
    }
} // namespace Slang
