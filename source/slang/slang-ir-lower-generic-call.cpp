// slang-ir-lower-generic-call.cpp
#include "slang-ir-lower-generic-call.h"
#include "slang-ir-generics-lowering-context.h"

namespace Slang
{
    struct GenericCallLoweringContext
    {
        SharedGenericsLoweringContext* sharedContext;

        // Represents a work item for unpacking `inout` or `out` arguments after a generic call.
        struct ArgumentUnpackWorkItem
        {
            // Concrete typed destination.
            IRInst* dstArg = nullptr;
            // Packed argument.
            IRInst* packedArg = nullptr;
        };

        // Packs `arg` into a `IRAnyValue` if necessary, to make it feedable into the parameter.
        // If `arg` represents a concrete typed variable passed in to a generic `out` parameter,
        // this function indicates that it needs to be unpacked after the call by setting
        // `unpackAfterCall`.
        IRInst* maybePackArgument(
            IRBuilder* builder,
            IRType* paramType,
            IRInst* arg,
            ArgumentUnpackWorkItem& unpackAfterCall)
        {
            unpackAfterCall.dstArg = nullptr;
            unpackAfterCall.packedArg = nullptr;

            // If either paramType or argType is a pointer type
            // (because of `inout` or `out` modifiers), we extract
            // the underlying value type first.
            IRType* paramValType = paramType;
            IRType* argValType = arg->getDataType();
            IRInst* argVal = arg;
            bool isParamPointer = false;
            if (auto ptrType = as<IRPtrTypeBase>(paramType))
            {
                isParamPointer = true;
                paramValType = ptrType->getValueType();
            }
            bool isArgPointer = false;
            auto argType = arg->getDataType();
            if (auto argPtrType = as<IRPtrTypeBase>(argType))
            {
                isArgPointer = true;
                argValType = argPtrType->getValueType();
                argVal = builder->emitLoad(arg);
            }

            // Pack `arg` if the parameter expects AnyValue but
            // `arg` is not an AnyValue.
            if (as<IRAnyValueType>(paramValType) && !as<IRAnyValueType>(argValType))
            {
                auto packedArgVal = builder->emitPackAnyValue(paramValType, argVal);
                // if parameter expects an `out` pointer, store the packed val into a
                // variable and pass in a pointer to that variable.
                if (as<IRPtrTypeBase>(paramType))
                {
                    auto tempVar = builder->emitVar(paramValType);
                    builder->emitStore(tempVar, packedArgVal);
                    // tempVar needs to be unpacked into original var after the call.
                    unpackAfterCall.dstArg = arg;
                    unpackAfterCall.packedArg = tempVar;
                    return tempVar;
                }
                else
                {
                    return packedArgVal;
                }
            }
            return arg;
        }

        IRInst* maybeUnpackValue(IRBuilder* builder, IRType* expectedType, IRType* actualType, IRInst* value)
        {
            if (as<IRAnyValueType>(actualType) && !as<IRAnyValueType>(expectedType))
            {
                auto unpack = builder->emitUnpackAnyValue(expectedType, value);
                return unpack;
            }
            return value;
        }

        // Translate `callInst` into a call of `newCallee`, and respect the new `funcType`.
        // If `newCallee` is a lowered generic function, `specializeInst` contains the type
        // arguments used to specialize the callee.
        void translateCallInst(
            IRCall* callInst,
            IRFuncType* funcType,
            IRInst* newCallee,
            IRSpecialize* specializeInst)
        {
            List<IRType*> paramTypes;
            for (UInt i = 0; i < funcType->getParamCount(); i++)
                paramTypes.add(funcType->getParamType(i));

            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(callInst);

            // Process the argument list of the call.
            // For each argument, we test if it needs to be packed into an `AnyValue` for the
            // call. For `out` and `inout` parameters, they may also need to be unpacked after
            // the call, in which case we add such the argument to `argsToUnpack` so it can be
            // processed after the new call inst is emitted.
            List<IRInst*> args;
            List<ArgumentUnpackWorkItem> argsToUnpack;
            for (UInt i = 0; i < callInst->getArgCount(); i++)
            {
                auto arg = callInst->getArg(i);
                ArgumentUnpackWorkItem unpackWorkItem;
                auto newArg = maybePackArgument(builder, paramTypes[i], arg, unpackWorkItem);
                args.add(newArg);
                if (unpackWorkItem.packedArg)
                    argsToUnpack.add(unpackWorkItem);
            }
            if (specializeInst)
            {
                for (UInt i = 0; i < specializeInst->getArgCount(); i++)
                {
                    auto arg = specializeInst->getArg(i);
                    // Translate Type arguments into RTTI object.
                    if (as<IRType>(arg))
                    {
                        // We are using a simple type to specialize a callee.
                        // Generate RTTI for this type.
                        auto rttiObject = sharedContext->maybeEmitRTTIObject(arg);
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
            }

            // If callee returns `AnyValue` but we are expecting a concrete value, unpack it.
            auto calleeRetType = funcType->getResultType();
            auto newCall = builder->emitCallInst(calleeRetType, newCallee, args);
            auto callInstType = callInst->getDataType();
            auto unpackInst = maybeUnpackValue(builder, callInstType, calleeRetType, newCall);
            callInst->replaceUsesWith(unpackInst);
            callInst->removeAndDeallocate();

            // Unpack other `out` arguments.
            for (auto& item : argsToUnpack)
            {
                auto packedVal = builder->emitLoad(item.packedArg);
                auto originalValType = cast<IRPtrTypeBase>(item.dstArg->getDataType())->getValueType();
                auto unpackedVal = builder->emitUnpackAnyValue(originalValType, packedVal);
                builder->emitStore(item.dstArg, unpackedVal);
            }
        }

        void lowerCallToSpecializedFunc(IRCall* callInst, IRSpecialize* specializeInst)
        {
            // If we see a call(specialize(gFunc, Targs), args),
            // translate it into call(gFunc, args, Targs).
            auto loweredFunc = specializeInst->getBase();
            // All callees should have already been lowered in lower-generic-functions pass.
            // For intrinsic generic functions, they are left as is, and we also need to ignore
            // them here.
            if (loweredFunc->op == kIROp_Generic)
            {
                // This is an intrinsic function, don't transform.
                return;
            }
            IRFuncType* funcType = cast<IRFuncType>(loweredFunc->getDataType());
            translateCallInst(callInst, funcType, loweredFunc, specializeInst);
        }

        void lowerCallToInterfaceMethod(IRCall* callInst, IRLookupWitnessMethod* lookupInst)
        {
            // If we see a call(lookup_interface_method(...), ...), we need to translate
            // all occurences of associatedtypes.
            auto funcType = cast<IRFuncType>(lookupInst->getDataType());
            auto loweredFunc = lookupInst;
            translateCallInst(callInst, funcType, loweredFunc, nullptr);
        }

        void lowerCall(IRCall* callInst)
        {
            if (auto specializeInst = as<IRSpecialize>(callInst->getCallee()))
                lowerCallToSpecializedFunc(callInst, specializeInst);
            else if (auto lookupInst = as<IRLookupWitnessMethod>(callInst->getCallee()))
                lowerCallToInterfaceMethod(callInst, lookupInst);
        }

        void processInst(IRInst* inst)
        {
            if (auto callInst = as<IRCall>(inst))
            {
                lowerCall(callInst);
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
    };

    void lowerGenericCalls(SharedGenericsLoweringContext* sharedContext)
    {
        GenericCallLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }

}
