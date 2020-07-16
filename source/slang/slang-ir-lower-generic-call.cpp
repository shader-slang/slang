// slang-ir-lower-generic-function.cpp
#include "slang-ir-lower-generic-function.h"
#include "slang-ir-generics-lowering-context.h"

namespace Slang
{
    struct GenericCallLoweringContext
    {
        SharedGenericsLoweringContext* sharedContext;

        // Translate `callInst` into a call of `newCallee`, and respect the new `funcType`.
        // If `funcType` involve lowered generic parameters or return values, this function
        // also translates the argument list to match with that.
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

            List<IRInst*> args;

            // Indicates whether the caller should allocate space for return value.
            // If the lowered callee returns void and this call inst has a type that is not void,
            // then we are calling a transformed function that expects caller allocated return value
            // as the first argument.
            bool shouldCallerAllocateReturnValue = (funcType->getResultType()->op == kIROp_VoidType &&
                callInst->getDataType() != funcType->getResultType());

            IRVar* retVarInst = nullptr;
            int startParamIndex = 0;
            if (shouldCallerAllocateReturnValue)
            {
                // Declare a var for the return value.
                retVarInst = builder->emitVar(callInst->getFullType());
                args.add(retVarInst);
                startParamIndex = 1;
            }

            for (UInt i = 0; i < callInst->getArgCount(); i++)
            {
                auto arg = callInst->getArg(i);
                if (as<IRRawPointerTypeBase>(paramTypes[i] + startParamIndex) &&
                    !as<IRRawPointerTypeBase>(arg->getDataType()) &&
                    !as<IRPtrTypeBase>(arg->getDataType()))
                {
                    // We are calling a generic function that with an argument of
                    // some concrete value type. We need to convert this argument to void*.
                    // We do so by defining a local variable, store the SSA value
                    // in the variable, and use the pointer of this variable as argument.
                    auto localVar = builder->emitVar(arg->getDataType());
                    builder->emitStore(localVar, arg);
                    arg = localVar;
                }
                args.add(arg);
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
            auto callInstType = retVarInst ? builder->getVoidType() : callInst->getFullType();
            auto newCall = builder->emitCallInst(callInstType, newCallee, args);
            if (retVarInst)
            {
                auto loadInst = builder->emitLoad(retVarInst);
                callInst->replaceUsesWith(loadInst);
            }
            else
            {
                callInst->replaceUsesWith(newCall);
            }
            callInst->removeAndDeallocate();
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

    void lowerGenericCalls(SharedGenericsLoweringContext* sharedContext)
    {
        GenericCallLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }

}
