// slang-ir-lower-generics.cpp
#include "slang-ir-lower-generics.h"

#include "slang-ir.h"
#include "slang-ir-layout.h"
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

        // RTTI objects for each type used to call a generic function.
        Dictionary<IRInst*, IRInst*> rttiObjects;

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
            case kIROp_ThisType:
            case kIROp_AssociatedType:
            case kIROp_InterfaceType:
                return true;
            default:
                break;
            }
            if (auto ptrType = as<IRPtrTypeBase>(typeInst))
            {
                return isPolymorphicType(ptrType->getValueType());
            }
            return false;
        }

        IRInst* lowerParameterType(IRBuilder* builder, IRInst* paramType)
        {
            if (paramType && paramType->op == kIROp_TypeType)
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
            cloneInstDecorationsAndChildren(&cloneEnv, &sharedBuilderStorage, func, loweredFunc);
            auto block = as<IRBlock>(loweredFunc->getFirstChild());
            for (auto param : clonedParams)
            {
                param->removeFromParent();
                block->addParam(as<IRParam>(param));
            }
            // Lower generic typed parameters into RTTIPointers.
            auto firstInst = loweredFunc->getFirstNonParamInst();
            builder.setInsertBefore(firstInst);

            for (IRInst* param = loweredFunc->getFirstParam();
                param && param->op == kIROp_Param;
                param = param->getNextInst())
            {
                // Generic typed parameters have a type that is a param itself.
                if (auto rttiParam = as<IRParam>(param->getFullType()))
                {
                    SLANG_ASSERT(isPointerOfType(rttiParam->getFullType(), kIROp_RTTIType));
                    // Lower into a function parameter of raw pointer type.
                    param->setFullType(builder.getRawPointerType());
                    auto newType = builder.getRTTIPointerType(rttiParam);
                    // Cast the raw pointer parameter into a RTTIPointer with RTTI info from the type parameter.
                    auto typedPtr = builder.emitBitCast(newType, param);
                    // Replace all uses of param with typePtr.
                    param->replaceUsesWith(typedPtr);
                    typedPtr->setOperand(0, param);
                }
            }
            loweredGenericFunctions[genericValue] = loweredFunc;
            addToWorkList(loweredFunc);
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
                if (paramType->op == kIROp_Specialize)
                {
                    newOperands.add(builder->getRawPointerType());
                    translated = true;
                }
                else
                {
                    auto loweredParamType = lowerParameterType(builder, paramType);
                    translated = translated || (loweredParamType != paramType);
                    newOperands.add(loweredParamType);
                }
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
                        entry->setRequirementVal(lowerFuncType(&builder, funcType, ArrayView<IRInst*>()));
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

        void processVarInst(IRInst* varInst)
        {
            // We process only var declarations that have type
            // `Ptr<IRParam>`.
            // Due to the processing of `lowerGenericFunction`,
            // A local variable of generic type now appears as
            // `var X:Ptr<irParam:Ptr<RTTIType>>`
            // We match this pattern and turn this inst into
            // `X:RawPtr = alloca(rtti_extract_size(irParam))`
            auto varTypeInst = varInst->getFullType();
            if (!varTypeInst)
                return;
            auto ptrType = as<IRPtrType>(varTypeInst);
            if (!ptrType)
                return;
            auto varTypeParam = ptrType->getValueType();
            if (varTypeParam->op != kIROp_Param)
                return;
            if (!varTypeParam->getFullType())
                return;
            if (varTypeParam->getFullType()->op != kIROp_PtrType)
                return;
            if (as<IRPtrType>(varTypeParam->getFullType())->getValueType()->op != kIROp_RTTIType)
                return;

            // A local variable of generic type has a type that is an IRParam.
            // This parameter represents the RTTI that tells us the size of the type.
            // We need to transform the variable into an `alloca` call to allocate its
            // space based on the provided RTTI object.

            // Initialize IRBuilder for emitting instructions.
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedBuilderStorage;
            builder->setInsertBefore(varInst);

            // The result of `alloca` is an RTTIPointer(rttiObject).
            auto type = builder->getRTTIPointerType(varTypeParam);
            auto typeSize = builder->emitRTTIExtractSize(varTypeParam);
            auto newVarInst = builder->emitAlloca(type, typeSize);
            varInst->replaceUsesWith(newVarInst);
            varInst->removeAndDeallocate();
        }

        void processStoreInst(IRStore* storeInst)
        {
            auto rttiType = as<IRRTTIPointerType>(storeInst->ptr.get()->getFullType());
            if (!rttiType)
                return;
            // All stores of generic typed variables needs to be translated
            // to `IRCopy`s.
            auto valPtr = storeInst->val.get();
            if (valPtr->getFullType()->op == kIROp_RTTIPointerType)
            {
                // If `value` of the store is from another generic variable, it should
                // have already been replaced with the pointer to that variable by now.
                // So we don't need to do anything here.
            }
            else
            {
                // If value does not come from another generic variable, then it must be
                // a param. In this case, the parameter is a bitCast of the parameter to an
                // RTTIPointer type, so we just use the original parameter pointer and get
                // rid of the bitcast.
                SLANG_ASSERT(valPtr->op == kIROp_BitCast);
                valPtr = valPtr->getOperand(0);
                SLANG_ASSERT(valPtr->op == kIROp_Param);
            }
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedBuilderStorage;
            builder->setInsertBefore(storeInst);
            auto size = builder->emitRTTIExtractSize(rttiType->getRTTIOperand());
            auto copy = builder->emitCopy(storeInst->ptr.get(), valPtr, size);
            storeInst->replaceUsesWith(copy);
            storeInst->removeAndDeallocate();
        }

        void processLoadInst(IRLoad* loadInst)
        {
            auto rttiType = as<IRRTTIPointerType>(loadInst->ptr.get()->getFullType());
            if (!rttiType)
                return;
            // All loads of generic typed variables must be eliminated.
            // There are only two possible uses of a load(genericVar):
            // 1. store(x, load(genVar)), which will be handled by processStoreInst.
            // 2. call(f, load(genVar)) when calling a generic function or a member function
            //    via an interface witness lookup. In this case, we need to replace with
            //    just `genVar`, since that function has already been lowered to take
            //    raw pointers.
            // Here we replace all uses of load to just the pointer itself.
            // After this, all arguments in `call`s will be in its correct form.
            // All `store`s will become `store(x, genVar)`, and still need
            // to be translated into a `copy`, we leave that step when we get to
            // process the `store` inst.
            loadInst->replaceUsesWith(loadInst->getOperand(0));
            loadInst->removeAndDeallocate();
        }

        // Emits an IRRTTIObject containing type information for a given type.
        IRInst* maybeEmitRTTIObject(IRInst* typeInst)
        {
            IRInst* result = nullptr;
            if (rttiObjects.TryGetValue(typeInst, result))
                return result;
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedBuilderStorage;
            builder->setInsertBefore(typeInst);

            // For now the only type info we encapsualte is type size.
            IRSizeAndAlignment sizeAndAlignment;
            getNaturalSizeAndAlignment((IRType*)typeInst, &sizeAndAlignment);
            auto typeSize = builder->getIntValue(builder->getIntType(), sizeAndAlignment.size);
            auto sizeEntry = builder->emitRTTIEntry(kRTTISize, typeSize);
            result = builder->emitMakeRTTIObject(makeArrayView(sizeEntry));

            // Give a name to the rtti object.
            if (auto exportDecoration = typeInst->findDecoration<IRExportDecoration>())
            {
                String rttiObjName = "__rtti_" + String(exportDecoration->getMangledName());
                builder->addExportDecoration(result, rttiObjName.getUnownedSlice());
            }
            rttiObjects[typeInst] = result;
            return result;
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
                        auto witnessTableType = cast<IRWitnessTableType>(interfaceLookup->getWitnessTable()->getFullType());
                        auto interfaceType = maybeLowerInterfaceType(cast<IRInterfaceType>(witnessTableType->getConformanceType()));
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
                        if (as<IRRawPointerType>(paramTypes[i]) &&
                            !as<IRRawPointerType>(arg->getDataType()))
                        {
                            // We are calling a generic function that with an argument of
                            // concrete type. We need to convert this argument to void*.

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
                    {
                        auto arg = specializeInst->getArg(i);
                        // Translate Type arguments into RTTI object.
                        if (as<IRType>(arg))
                        {
                            // We are using a simple type to specialize a callee.
                            // Generate RTTI for this type.
                            auto rttiObject = maybeEmitRTTIObject(arg);
                            arg = builder->emitGetAddress(
                                builder->getPtrType(builder->getRTTIType()),
                                rttiObject);
                        }
                        else if (arg->op == kIROp_Specialize)
                        {
                            // The type argument used to specialize a callee is itself a
                            // specialization of some generic type.
                            // TODO: generate RTTI object for specializations of generic types.
                            SLANG_UNIMPLEMENTED_X("RTTI object generation for generic types");
                        }
                        else if (arg->op == kIROp_RTTIObject)
                        {
                            // We are inside a generic function and using a generic parameter
                            // to specialize another callee. The generic parameter of the caller
                            // has already been translated into an RTTI object, so we just need
                            // to pass this object down.
                        }
                        args.add(arg);
                    }
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
            else if (inst->op == kIROp_Var || inst->op == kIROp_undefined)
            {
                processVarInst(inst);
            }
            else if (inst->op == kIROp_Load)
            {
                processLoadInst(cast<IRLoad>(inst));
            }
            else if (inst->op == kIROp_Store)
            {
                processStoreInst(cast<IRStore>(inst));
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
