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
    struct GenericVarLoweringContext
    {
        SharedGenericsLoweringContext* sharedContext;

        void processVarInst(IRInst* varInst)
        {
            // We process only var declarations that have type
            // `Ptr<IRParam>` or `Ptr<IRLookupInterfaceMethod>`.
            //
            // Due to the processing of `lowerGenericFunction`,
            // A local variable of generic type now appears as
            // `var X:Ptr<y:Ptr<RTTIType>>`,
            // where y can be an IRParam if it is a generic type,
            // or an `lookup_interface_method` if it is an associated type.
            // We match this pattern and turn this inst into
            // `X:RTTIPtr(irParam) = alloca(irParam)`
            auto varTypeInst = varInst->getDataType();
            if (!varTypeInst)
                return;
            auto ptrType = as<IRPtrType>(varTypeInst);
            if (!ptrType)
                return;

            // `varTypeParam` represents a pointer to the RTTI object.
            auto varTypeParam = ptrType->getValueType();
            if (varTypeParam->op != kIROp_Param && varTypeParam->op != kIROp_lookup_interface_method)
                return;
            if (!varTypeParam->getDataType())
                return;
            if (varTypeParam->getDataType()->op != kIROp_PtrType)
                return;
            if (as<IRPtrType>(varTypeParam->getDataType())->getValueType()->op != kIROp_RTTIType)
                return;


            // A local variable of generic type has a type that is an IRParam.
            // This parameter represents the RTTI that tells us the size of the type.
            // We need to transform the variable into an `alloca` call to allocate its
            // space based on the provided RTTI object.

            // Initialize IRBuilder for emitting instructions.
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(varInst);

            // The result of `alloca` is an RTTIPointer(rttiObject).
            auto type = builder->getRTTIPointerType(varTypeParam);
            auto newVarInst = builder->emitAlloca(type, varTypeParam);
            varInst->replaceUsesWith(newVarInst);
            varInst->removeAndDeallocate();
        }

        void processStoreInst(IRStore* storeInst)
        {
            auto rttiType = as<IRRTTIPointerType>(storeInst->ptr.get()->getDataType());
            if (!rttiType)
                return;
            // All stores of generic typed variables needs to be translated
            // to `IRCopy`s.
            auto valPtr = storeInst->val.get();
            if (valPtr->getDataType()->op == kIROp_RTTIPointerType)
            {
                // If `value` of the store is from another generic variable, it should
                // have already been replaced with the pointer to that variable by now.
                // So we don't need to do anything here.
            }
            else if (valPtr->op == kIROp_undefined)
            {
                // We don't need to store an undef value.
                storeInst->removeAndDeallocate();
                return;
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
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(storeInst);
            auto copy = builder->emitCopy(
                storeInst->ptr.get(),
                valPtr,
                rttiType->getRTTIOperand());
            storeInst->replaceUsesWith(copy);
            storeInst->removeAndDeallocate();
        }

        void processLoadInst(IRLoad* loadInst)
        {
            auto rttiType = as<IRRTTIPointerType>(loadInst->ptr.get()->getDataType());
            if (!rttiType)
                return;
            // There are only two possible uses of a load(genericVar):
            // 1. store(x, load(genVar)), which will be handled by processStoreInst.
            // 2. call(f, load(genVar)) when calling a generic function or a member function
            //    via an interface witness lookup. In this case, we need to replace with
            //    just `genVar`, since that function has already been lowered to take
            //    raw pointers.
            // In both cases, we can simply replace the use side with a pointer instead
            // and never need to represent a "value" typed object explicitly.
            // However, to preserve the ordering, we must make a copy of every load so
            // we don't change the meaning of the code if there are `store`s between the
            // `load` and the use site.

            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(loadInst);

            // Allocate a copy of the value.
            auto allocaInst = builder->emitAlloca(rttiType, rttiType->getRTTIOperand());
            builder->emitCopy(allocaInst, loadInst->ptr.get(), rttiType->getRTTIOperand());

            // Here we replace all uses of load to just the pointer to the copy.
            // After this, all arguments in `call`s will be in its correct form.
            // All `store`s will become `store(x, genVar)`, and still need
            // to be translated into another `copy`, we leave that step when we get to
            // process the `store` inst.
            loadInst->replaceUsesWith(allocaInst);
            loadInst->removeAndDeallocate();
        }

        void processInst(IRInst* inst)
        {
            if (inst->op == kIROp_Var)
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

    void lowerGenericVar(SharedGenericsLoweringContext* sharedContext)
    {
        GenericVarLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }
}

