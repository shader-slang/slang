// slang-ir-lower-generic-function.cpp
#include "slang-ir-lower-generic-function.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{
    // This is a subpass of generics lowering IR transformation.
    // This pass lowers all generic function types and function definitions, including
    // the function types used in interface types, to ordinary functions that takes
    // raw pointers in place of generic types.
    struct GenericFunctionLoweringContext
    {
        SharedGenericsLoweringContext* sharedContext;
        IRInst* lowerParameterType(IRBuilder* builder, IRInst* paramType)
        {
            if (isTypeValue(paramType))
            {
                return builder->getPtrType(builder->getRTTIType());
            }
            if (isPolymorphicType(paramType))
            {
                return builder->getRawPointerType();
            }
            return paramType;
        }

        IRInst* lowerGenericFunction(IRInst* genericValue)
        {
            IRInst* result = nullptr;
            if (sharedContext->loweredGenericFunctions.TryGetValue(genericValue, result))
                return result;
            auto genericParent = as<IRGeneric>(genericValue);
            SLANG_ASSERT(genericParent);
            auto func = as<IRFunc>(findGenericReturnVal(genericParent));
            SLANG_ASSERT(func);
            if (!func->isDefinition())
            {
                sharedContext->loweredGenericFunctions[genericValue] = genericValue;
                return genericValue;
            }
            IRCloneEnv cloneEnv;
            IRBuilder builder;
            builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder.setInsertBefore(genericParent);
            auto loweredFunc = cast<IRFunc>(cloneInstAndOperands(&cloneEnv, &builder, func));
            loweredFunc->setFullType(lowerGenericFuncType(&builder, cast<IRGeneric>(genericParent->getFullType())));
            List<IRInst*> clonedParams;
            for (auto genericChild : genericParent->getFirstBlock()->getChildren())
            {
                if (genericChild == func)
                    continue;
                if (genericChild->op == kIROp_ReturnVal)
                    continue;
                // Process all generic parameters and local type definitions.
                auto clonedChild = cloneInst(&cloneEnv, &builder, genericChild);
                if (clonedChild->op == kIROp_Param)
                {
                    auto paramType = clonedChild->getFullType();
                    auto loweredParamType = lowerParameterType(&builder, paramType);
                    if (loweredParamType != paramType)
                    {
                        clonedChild->setFullType((IRType*)loweredParamType);
                    }
                    clonedParams.add(clonedChild);
                }
            }
            cloneInstDecorationsAndChildren(&cloneEnv, &sharedContext->sharedBuilderStorage, func, loweredFunc);

            // If the function returns a generic typed value, we need to turn it
            // into an `out` parameter, since only the caller can allocate space
            // for it.
            auto oldFuncType = cast<IRFuncType>(func->getDataType());
            if (isPolymorphicType(oldFuncType->getResultType()))
            {
                builder.setInsertBefore(loweredFunc->getFirstBlock()->getFirstOrdinaryInst());
                // We defer creation of the returnVal parameter until we see the first
                // `return` instruction, because we can only obtain the cloned return type
                // of this function by checking the type of the cloned return inst.
                IRParam* retValParam = nullptr;
                // Translate all return insts to `store`s.
                // Those `store`s will be processed and translated into `copy`s when we
                // get to process them via workList.
                for (auto bb : loweredFunc->getBlocks())
                {
                    auto retInst = as<IRReturnVal>(bb->getTerminator());
                    if (!retInst)
                        continue;
                    if (!retValParam)
                    {
                        // Now we have the return type, emit the returnVal parameter.
                        // The type of this parameter is still not translated to RawPointer yet,
                        // and will be processed together with all the other existing parameters.
                        retValParam = builder.emitParamAtHead(
                            builder.getOutType(retInst->getVal()->getDataType()));
                    }
                    builder.setInsertBefore(retInst);
                    builder.emitStore(retValParam, retInst->getVal());
                    builder.emitReturn();
                    retInst->removeAndDeallocate();
                }
            }

            auto block = as<IRBlock>(loweredFunc->getFirstChild());
            for (auto param : clonedParams)
            {
                param->removeFromParent();
                block->addParam(as<IRParam>(param));
            }
            // Lower generic typed parameters into RTTIPointers.
            auto firstInst = loweredFunc->getFirstOrdinaryInst();
            builder.setInsertBefore(firstInst);

            for (IRInst* param = loweredFunc->getFirstParam();
                param && param->op == kIROp_Param;
                param = param->getNextInst())
            {
                // Generic typed parameters have a type that is a param itself.
                auto paramType = param->getDataType();
                if (auto ptrType = as<IRPtrTypeBase>(paramType))
                    paramType = ptrType->getValueType();
                if (isPointerOfType(paramType->getDataType(), kIROp_RTTIType) ||
                    paramType->op == kIROp_lookup_interface_method)
                {
                    // Lower into a function parameter of raw pointer type.
                    param->setFullType(builder.getRawPointerType());
                    auto newType = builder.getRTTIPointerType(paramType);
                    // Cast the raw pointer parameter into a RTTIPointer with RTTI info from the type parameter.
                    auto typedPtr = builder.emitBitCast(newType, param);
                    // Replace all uses of param with typePtr.
                    param->replaceUsesWith(typedPtr);
                    typedPtr->setOperand(0, param);
                }
            }
            sharedContext->loweredGenericFunctions[genericValue] = loweredFunc;
            sharedContext->addToWorkList(loweredFunc);
            return loweredFunc;
        }

        IRType* lowerGenericFuncType(IRBuilder* builder, IRGeneric* genericVal)
        {
            ShortList<IRInst*> genericParamTypes;
            for (auto genericParam : genericVal->getParams())
            {
                genericParamTypes.add(lowerParameterType(builder, genericParam->getFullType()));
            }

            auto innerType = (IRFuncType*)lowerFuncType(
                builder,
                cast<IRFuncType>(findGenericReturnVal(genericVal)),
                genericParamTypes.getArrayView().arrayView);

            return innerType;
        }

        IRType* lowerFuncType(IRBuilder* builder, IRFuncType* funcType, ArrayView<IRInst*> additionalParams)
        {
            List<IRInst*> newOperands;
            bool translated = false;
            for (UInt i = 0; i < funcType->getOperandCount(); i++)
            {
                auto paramType = funcType->getOperand(i);
                auto loweredParamType = lowerParameterType(builder, paramType);
                translated = translated || (loweredParamType != paramType);
                if (translated && i == 0)
                {
                    // We are translating the return value, this means that
                    // the return value must be passed explicitly via an `out` parameter.
                    // In this case, the new return value will be `void`, and the
                    // translated return value type will be the first parameter type;
                    newOperands.add(builder->getVoidType());
                }
                newOperands.add(loweredParamType);
            }
            if (!translated && additionalParams.getCount() == 0)
                return funcType;
            for (Index i = 0; i < additionalParams.getCount(); i++)
            {
                newOperands.add(additionalParams[i]);
            }
            auto newFuncType = builder->getFuncType(
                newOperands.getCount() - 1,
                (IRType**)(newOperands.begin() + 1),
                (IRType*)newOperands[0]);

            IRCloneEnv cloneEnv;
            cloneInstDecorationsAndChildren(&cloneEnv, &sharedContext->sharedBuilderStorage, funcType, newFuncType);
            return newFuncType;
        }

        IRInterfaceType* maybeLowerInterfaceType(IRInterfaceType* interfaceType)
        {
            if (sharedContext->loweredInterfaceTypes.Contains(interfaceType))
                return interfaceType;

            IRBuilder builder;
            builder.sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder.setInsertBefore(interfaceType);

            // Translate IRFuncType in interface requirements.
            for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
            {
                if (auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i)))
                {
                    if (auto funcType = as<IRFuncType>(entry->getRequirementVal()))
                    {
                        entry->setRequirementVal(lowerFuncType(&builder, funcType, ArrayView<IRInst*>()));
                    }
                    else if (auto genericFuncType = as<IRGeneric>(entry->getRequirementVal()))
                    {
                        entry->setRequirementVal(lowerGenericFuncType(&builder, genericFuncType));
                    }
                    else if (entry->getRequirementVal()->op == kIROp_AssociatedType)
                    {
                        entry->setRequirementVal(builder.getPtrType(builder.getRTTIType()));
                    }
                }
            }

            sharedContext->loweredInterfaceTypes.Add(interfaceType);
            return interfaceType;
        }

        bool isTypeKindVal(IRInst* inst)
        {
            auto type = inst->getDataType();
            if (!type) return false;
            return type->op == kIROp_TypeKind;
        }

        // Lower items in a witness table. This triggers lowering of generic functions,
        // and emission of wrapper functions.
        void lowerWitnessTable(IRWitnessTable* witnessTable)
        {
            auto interfaceType = maybeLowerInterfaceType(cast<IRInterfaceType>(witnessTable->getConformanceType()));
            if (interfaceType != witnessTable->getConformanceType())
                witnessTable->setConformanceType(interfaceType);
            for (auto child : witnessTable->getChildren())
            {
                auto entry = as<IRWitnessTableEntry>(child);
                if (!entry)
                    continue;
                if (auto genericVal = as<IRGeneric>(entry->getSatisfyingVal()))
                {
                    // Lower generic functions in witness table.
                    if (findGenericReturnVal(genericVal)->op == kIROp_Func)
                    {
                        auto loweredFunc = lowerGenericFunction(genericVal);
                        entry->satisfyingVal.set(loweredFunc);
                    }
                }
                else if (isTypeKindVal(entry->getSatisfyingVal()))
                {
                    // Translate a Type value to an RTTI object pointer.
                    auto rttiObject = sharedContext->maybeEmitRTTIObject(entry->getSatisfyingVal());
                    IRBuilder builderStorage;
                    auto builder = &builderStorage;
                    builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
                    builder->setInsertBefore(witnessTable);
                    auto rttiObjectPtr = builder->emitGetAddress(
                        builder->getPtrType(builder->getRTTIType()),
                        rttiObject);
                    entry->satisfyingVal.set(rttiObjectPtr);
                }
                else if (as<IRWitnessTable>(entry->getSatisfyingVal()))
                {
                    // No processing needed here.
                    // The witness table will be processed from the work list.
                }
            }
        }

        void lowerLookupInterfaceMethodInst(IRLookupWitnessMethod* lookupInst)
        {
            // Update the type of lookupInst to the lowered type of the corresponding interface requirement val.

            // If the requirement is a function, interfaceRequirementVal will be the lowered function type.
            // If the requirement is an associatedtype, interfaceRequirementVal will be Ptr<RTTIObject>.
            IRInst* interfaceRequirementVal = nullptr;
            auto witnessTableType = cast<IRWitnessTableType>(lookupInst->getWitnessTable()->getDataType());
            auto interfaceType = maybeLowerInterfaceType(cast<IRInterfaceType>(witnessTableType->getConformanceType()));
            interfaceRequirementVal = sharedContext->findInterfaceRequirementVal(interfaceType, lookupInst->getRequirementKey());
            lookupInst->setFullType((IRType*)interfaceRequirementVal);
        }

        void lowerSpecialize(IRSpecialize* specializeInst)
        {
            // If we see a call(specialize(gFunc, Targs), args),
            // translate it into call(gFunc, args, Targs).
            IRInst* loweredFunc = nullptr;
            auto funcToSpecialize = specializeInst->getBase();
            if (funcToSpecialize->op == kIROp_Generic)
            {
                loweredFunc = lowerGenericFunction(funcToSpecialize);
                if (loweredFunc != funcToSpecialize)
                {
                    specializeInst->setOperand(0, loweredFunc);
                }
            }
        }

        void processInst(IRInst* inst)
        {
            if (auto specializeInst = as<IRSpecialize>(inst))
            {
                lowerSpecialize(specializeInst);
            }
            else if (auto lookupInterfaceMethod = as<IRLookupWitnessMethod>(inst))
            {
                lowerLookupInterfaceMethodInst(lookupInterfaceMethod);
            }
            else if (auto witnessTable = as<IRWitnessTable>(inst))
            {
                lowerWitnessTable(witnessTable);
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
    void lowerGenericFunctions(SharedGenericsLoweringContext* sharedContext)
    {
        GenericFunctionLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }
}

