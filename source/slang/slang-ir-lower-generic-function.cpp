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

        IRInst* lowerGenericFunction(IRInst* genericValue)
        {
            IRInst* result = nullptr;
            if (sharedContext->loweredGenericFunctions.TryGetValue(genericValue, result))
                return result;
            // Do not lower intrinsic functions.
            if (genericValue->findDecoration<IRTargetIntrinsicDecoration>())
                return genericValue;
            auto genericParent = as<IRGeneric>(genericValue);
            SLANG_ASSERT(genericParent);
            SLANG_ASSERT(genericParent->getDataType());
            auto func = as<IRFunc>(findGenericReturnVal(genericParent));
            if (!func)
            {
                // Nested generic functions are supposed to be flattened before entering
                // this pass. The reason we are still seeing them must be that they are
                // intrinsic functions. In this case we ignore the function.
                SLANG_ASSERT(findInnerMostGenericReturnVal(genericParent)
                                 ->findDecoration<IRTargetIntrinsicDecoration>() != nullptr);
                return genericValue;
            }
            SLANG_ASSERT(func);
            // Do not lower intrinsic functions.
            if (!func->isDefinition() || func->findDecoration<IRTargetIntrinsicDecoration>())
            {
                sharedContext->loweredGenericFunctions[genericValue] = genericValue;
                return genericValue;
            }
            IRCloneEnv cloneEnv;
            IRBuilder builder(sharedContext->sharedBuilderStorage);
            builder.setInsertBefore(genericParent);
            auto loweredFunc = cast<IRFunc>(cloneInstAndOperands(&cloneEnv, &builder, func));
            auto loweredGenericType =
                lowerGenericFuncType(&builder, cast<IRGeneric>(genericParent->getFullType()));
            SLANG_ASSERT(loweredGenericType);
            loweredFunc->setFullType(loweredGenericType);
            List<IRInst*> clonedParams;

            for (auto genericChild : genericParent->getFirstBlock()->getChildren())
            {
                if (genericChild == func)
                    continue;
                if (genericChild->getOp() == kIROp_Return)
                    continue;
                // Process all generic parameters and local type definitions.
                auto clonedChild = cloneInst(&cloneEnv, &builder, genericChild);
                if (clonedChild->getOp() == kIROp_Param)
                {
                    auto paramType = clonedChild->getFullType();
                    auto loweredParamType = sharedContext->lowerType(&builder, paramType);
                    if (loweredParamType != paramType)
                    {
                        clonedChild->setFullType((IRType*)loweredParamType);
                    }
                    clonedParams.add(clonedChild);
                }
            }
            cloneInstDecorationsAndChildren(&cloneEnv, &sharedContext->sharedBuilderStorage, func, loweredFunc);

            auto block = as<IRBlock>(loweredFunc->getFirstChild());
            for (auto param : clonedParams)
            {
                param->removeFromParent();
                block->addParam(as<IRParam>(param));
            }
            // Lower generic typed parameters into AnyValueType.
            auto firstInst = loweredFunc->getFirstOrdinaryInst();
            builder.setInsertBefore(firstInst);
            sharedContext->loweredGenericFunctions[genericValue] = loweredFunc;
            sharedContext->addToWorkList(loweredFunc);
            return loweredFunc;
        }

        IRType* lowerGenericFuncType(IRBuilder* builder, IRGeneric* genericVal)
        {
            ShortList<IRInst*> genericParamTypes;
            Dictionary<IRInst*, IRInst*> typeMapping;
            for (auto genericParam : genericVal->getParams())
            {
                genericParamTypes.add(sharedContext->lowerType(builder, genericParam->getFullType()));
                if (auto anyValueSizeDecor = genericParam->findDecoration<IRTypeConstraintDecoration>())
                {
                    auto anyValueSize = sharedContext->getInterfaceAnyValueSize(anyValueSizeDecor->getConstraintType(), genericParam->sourceLoc);
                    auto anyValueType = builder->getAnyValueType(anyValueSize);
                    typeMapping[genericParam] = anyValueType;
                }
            }

            auto innerType = (IRFuncType*)lowerFuncType(
                builder,
                cast<IRFuncType>(findGenericReturnVal(genericVal)),
                typeMapping,
                genericParamTypes.getArrayView().arrayView);

            return innerType;
        }

        IRType* lowerFuncType(IRBuilder* builder, IRFuncType* funcType,
            const Dictionary<IRInst*, IRInst*>& typeMapping,
            ArrayView<IRInst*> additionalParams)
        {
            List<IRInst*> newOperands;
            bool translated = false;
            for (UInt i = 0; i < funcType->getOperandCount(); i++)
            {
                auto paramType = funcType->getOperand(i);
                auto loweredParamType = sharedContext->lowerType(builder, paramType, typeMapping, nullptr);
                translated = translated || (loweredParamType != paramType);
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
            IRInterfaceType* loweredType = nullptr;
            if (sharedContext->loweredInterfaceTypes.TryGetValue(interfaceType, loweredType))
                return loweredType;
            if (sharedContext->mapLoweredInterfaceToOriginal.ContainsKey(interfaceType))
                return interfaceType;
            // Do not lower intrinsic interfaces.
            if (isBuiltin(interfaceType))
                return interfaceType;
            // Do not lower COM interfaces.
            if (isComInterfaceType(interfaceType))
                return interfaceType;

            List<IRInterfaceRequirementEntry*> newEntries;

            IRBuilder builder(sharedContext->sharedBuilderStorage);
            builder.setInsertBefore(interfaceType);

            // Translate IRFuncType in interface requirements.
            for (UInt i = 0; i < interfaceType->getOperandCount(); i++)
            {
                if (auto entry = as<IRInterfaceRequirementEntry>(interfaceType->getOperand(i)))
                {
                    // Note: The logic that creates the `IRInterfaceRequirementEntry`s does
                    // not currently guarantee that the *value* part of each key-value pair
                    // gets filled in. We thus need to defend against a null `requirementVal`
                    // here, at least until the underlying issue gets resolved.
                    //
                    IRInst* requirementVal = entry->getRequirementVal();
                    IRInst* loweredVal = nullptr;
                    if(!requirementVal)
                    {}
                    else if (auto funcType = as<IRFuncType>(requirementVal))
                    {
                        loweredVal = lowerFuncType(&builder, funcType, Dictionary<IRInst*, IRInst*>(), ArrayView<IRInst*>());
                    }
                    else if (auto genericFuncType = as<IRGeneric>(requirementVal))
                    {
                        loweredVal = lowerGenericFuncType(&builder, genericFuncType);
                    }
                    else if (requirementVal->getOp() == kIROp_AssociatedType)
                    {
                        loweredVal = builder.getRTTIHandleType();
                    }
                    else
                    {
                        loweredVal = requirementVal;
                    }
                    auto newEntry = builder.createInterfaceRequirementEntry(entry->getRequirementKey(), loweredVal);
                    newEntries.add(newEntry);
                }
            }
            loweredType = builder.createInterfaceType(newEntries.getCount(), (IRInst**)newEntries.getBuffer());
            IRCloneEnv cloneEnv;
            cloneInstDecorationsAndChildren(&cloneEnv, &sharedContext->sharedBuilderStorage,
                interfaceType, loweredType);
            sharedContext->loweredInterfaceTypes.Add(interfaceType, loweredType);
            sharedContext->mapLoweredInterfaceToOriginal[loweredType] = interfaceType;
            return loweredType;
        }

        bool isTypeKindVal(IRInst* inst)
        {
            auto type = inst->getDataType();
            if (!type) return false;
            return type->getOp() == kIROp_TypeKind;
        }

        // Lower items in a witness table. This triggers lowering of generic functions,
        // and emission of wrapper functions.
        void lowerWitnessTable(IRWitnessTable* witnessTable)
        {
            auto interfaceType = maybeLowerInterfaceType(cast<IRInterfaceType>(witnessTable->getConformanceType()));
            IRBuilder builderStorage(sharedContext->sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(witnessTable);
            if (interfaceType != witnessTable->getConformanceType())
            {
                auto newWitnessTableType = builder->getWitnessTableType(interfaceType);
                witnessTable->setFullType(newWitnessTableType);
            }
            if (isBuiltin(interfaceType))
                return;
            for (auto child : witnessTable->getChildren())
            {
                auto entry = as<IRWitnessTableEntry>(child);
                if (!entry)
                    continue;
                if (auto genericVal = as<IRGeneric>(entry->getSatisfyingVal()))
                {
                    // Lower generic functions in witness table.
                    if (findGenericReturnVal(genericVal)->getOp() == kIROp_Func)
                    {
                        auto loweredFunc = lowerGenericFunction(genericVal);
                        entry->satisfyingVal.set(loweredFunc);
                    }
                }
                else if (isTypeKindVal(entry->getSatisfyingVal()))
                {
                    // Translate a Type value to an RTTI object pointer.
                    auto rttiObject = sharedContext->maybeEmitRTTIObject(entry->getSatisfyingVal());
                    auto rttiObjectPtr = builder->emitGetAddress(
                        builder->getRTTIHandleType(),
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
            auto witnessTableType = as<IRWitnessTableType>(lookupInst->getWitnessTable()->getDataType());
            if (!witnessTableType) return;
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
            if (funcToSpecialize->getOp() == kIROp_Generic)
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

        void replaceLoweredInterfaceTypes()
        {
            for (auto lowered : sharedContext->loweredInterfaceTypes)
            {
                 lowered.Key->replaceUsesWith(lowered.Value);
            }
            // Update hash keys of globalNumberingMap, since the types are modified.
            sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();
            sharedContext->mapInterfaceRequirementKeyValue.Clear();
        }

        void processModule()
        {
            // We start by initializing our shared IR building state,
            // since we will re-use that state for any code we
            // generate along the way.
            //
            SharedIRBuilder* sharedBuilder = &sharedContext->sharedBuilderStorage;
            sharedBuilder->init(sharedContext->module);

            sharedContext->addToWorkList(sharedContext->module->getModuleInst());

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

            replaceLoweredInterfaceTypes();
        }
    };
    void lowerGenericFunctions(SharedGenericsLoweringContext* sharedContext)
    {
        GenericFunctionLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }
}

