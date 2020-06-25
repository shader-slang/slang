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
        HashSet<IRInterfaceType*> loweredInterfaceTypes;

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

        bool isPolymorphicType(IRInst* typeInst)
        {
            if (as<IRParam>(typeInst) && as<IRTypeType>(typeInst->getFullType()))
                return true;
            switch (typeInst->op)
            {
            case kIROp_AssociatedType:
            case kIROp_InterfaceType:
                return true;
            default:
                return false;
            }
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
            loweredFunc->setFullType(lowerGenericFuncType(&builder, cast<IRGeneric>(genericParent->getFullType())));
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
                if (isPolymorphicType(param->getFullType()))
                {
                    param->setFullType(builder.getRawPointerType());
                }
            }
            addToWorkList(loweredFunc);
            return loweredFunc;
        }

        IRType* lowerGenericFuncType(IRBuilder* builder, IRGeneric* genericVal)
        {
            List<IRInst*> genericParamTypes;
            for (auto genericParam : genericVal->getParams())
            {
                if (isPolymorphicType(genericParam->getFullType()))
                {
                    genericParamTypes.add(builder->getRawPointerType());
                }
                else
                {
                    genericParamTypes.add(genericParam->getFullType());
                }
            }

            auto innerType = (IRFuncType*)lowerFuncType(
                builder,
                cast<IRFuncType>(findGenericReturnVal(genericVal)),
                genericParamTypes.getCount());

            for (int i = 0; i < genericParamTypes.getCount(); i++)
            {
                innerType->setOperand(
                    innerType->getOperandCount() - genericParamTypes.getCount() + i,
                    genericParamTypes[i]);
            }

            return innerType;
        }

        IRType* lowerFuncType(IRBuilder* builder, IRFuncType* funcType, UInt additionalParamCount = 0)
        {
            List<IRInst*> newOperands;
            bool translated = false;
            for (UInt i = 0; i < funcType->getOperandCount(); i++)
            {
                auto paramType = funcType->getOperand(i);
                if (isPolymorphicType(paramType))
                {
                    newOperands.add(builder->getRawPointerType());
                    translated = true;
                }
                else if (paramType->op == kIROp_Specialize)
                {
                    // TODO: handle static specialized type here.
                    // For now treat all specialized types as dynamic.
                    // In the future, we need to turn things like Array<IDynamic> into Array<void*>.
                    newOperands.add(builder->getRawPointerType());
                    translated = true;
                }
                else
                {
                    newOperands.add(paramType);
                }
            }
            if (!translated && additionalParamCount == 0)
                return funcType;
            for (UInt i = 0; i < additionalParamCount; i++)
            {
                newOperands.add(nullptr);
            }
            auto newFuncType = builder->getFuncType(
                newOperands.getCount() - 1,
                (IRType**)(newOperands.begin() + 1),
                (IRType*)newOperands[0]);

            IRCloneEnv cloneEnv;
            cloneInstDecorationsAndChildren(&cloneEnv, &sharedBuilderStorage, funcType, newFuncType);
            return newFuncType;
        }

        IRInterfaceType* maybeLowerInterfaceType(IRInterfaceType* interfaceType)
        {
            if (loweredInterfaceTypes.Contains(interfaceType))
                return interfaceType;

            IRBuilder builder;
            builder.sharedBuilder = &sharedBuilderStorage;
            builder.setInsertBefore(interfaceType);

            // Translate IRFuncType in interface requirements.
            for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
            {
                if (auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i)))
                {
                    if (auto funcType = as<IRFuncType>(entry->getRequirementVal()))
                    {
                        entry->setRequirementVal(lowerFuncType(&builder, funcType));
                    }
                    else if (auto genericFuncType = as<IRGeneric>(entry->getRequirementVal()))
                    {
                        entry->setRequirementVal(lowerGenericFuncType(&builder, genericFuncType));
                    }
                }
            }

            loweredInterfaceTypes.Add(interfaceType);
            return interfaceType;
        }

        void processInst(IRInst* inst)
        {
            if (auto callInst = as<IRCall>(inst))
            {
                // If we see a call(specialize(gFunc, Targs), args),
                // translate it into call(gFunc, args, Targs).
                auto funcOperand = callInst->getOperand(0);
                IRInst* loweredFunc = nullptr;
                if (auto specializeInst = as<IRSpecialize>(funcOperand))
                {
                    auto funcToSpecialize = specializeInst->getOperand(0);
                    List<IRType*> paramTypes;
                    if (auto interfaceLookup = as<IRLookupWitnessMethod>(funcToSpecialize))
                    {
                        // The callee is a result of witness table lookup, we will only
                        // translate the call.
                        IRInst* callee = nullptr;
                        auto interfaceType = maybeLowerInterfaceType(cast<IRInterfaceType>(interfaceLookup->getInterfaceType()));
                        for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
                        {
                            auto entry = cast<IRInterfaceRequirementEntry>(interfaceType->getOperand(i));
                            if (entry->getRequirementKey() == interfaceLookup->getOperand(1))
                            {
                                callee = entry->getRequirementVal();
                                break;
                            }
                        }
                        auto funcType = cast<IRFuncType>(callee);
                        for (UInt i = 0; i < funcType->getParamCount(); i++)
                            paramTypes.add(funcType->getParamType(i));
                        loweredFunc = funcToSpecialize;
                    }
                    else
                    {
                        loweredFunc = lowerGenericFunction(specializeInst->getOperand(0));
                        if (loweredFunc == specializeInst->getOperand(0))
                        {
                            // This is an intrinsic function, don't transform.
                            return;
                        }
                        for (auto param : as<IRFunc>(loweredFunc)->getParams())
                            paramTypes.add(param->getDataType());
                    }

                    IRBuilder builderStorage;
                    auto builder = &builderStorage;
                    builder->sharedBuilder = &sharedBuilderStorage;
                    builder->setInsertBefore(inst);
                    List<IRInst*> args;
                    auto rawPtrType = builder->getRawPointerType();
                    for (UInt i = 0; i < callInst->getArgCount(); i++)
                    {
                        auto arg = callInst->getArg(i);
                        if (paramTypes[i] == rawPtrType &&
                            arg->getDataType() != rawPtrType)
                        {
                            // We are calling a generic function that with an argument of
                            // concrete type. We need to convert this argument o void*.

                            // Ideally this should just be a GetElementAddress inst.
                            // However the current code emitting logic for this instruction
                            // doesn't truly respect the pointerness and does not produce
                            // what we needed. For now we use another instruction here
                            // to keep changes minimal.
                            arg = builder->emitGetAddress(
                                rawPtrType,
                                arg);
                        }
                        args.add(arg);
                    }
                    for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                        args.add(specializeInst->getArg(i));
                    auto newCall = builder->emitCallInst(callInst->getFullType(), loweredFunc, args);
                    callInst->replaceUsesWith(newCall);
                    callInst->removeAndDeallocate();
                }
            }
            else if (auto witnessTable = as<IRWitnessTable>(inst))
            {
                // Lower generic functions in witness table.
                for (auto child : witnessTable->getChildren())
                {
                    auto entry = as<IRWitnessTableEntry>(child);
                    if (!entry)
                        continue;
                    if (auto genericVal = as<IRGeneric>(entry->getSatisfyingVal()))
                    {
                        if (findGenericReturnVal(genericVal)->op == kIROp_Func)
                        {
                            auto loweredFunc = lowerGenericFunction(genericVal);
                            entry->satisfyingVal.set(loweredFunc);
                        }
                    }
                }
            }
            else if (auto interfaceType = as<IRInterfaceType>(inst))
            {
                maybeLowerInterfaceType(interfaceType);
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
