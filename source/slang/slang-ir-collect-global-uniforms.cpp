// slang-ir-collect-global-uniforms.cpp
#include "slang-ir-collect-global-uniforms.h"

#include "slang-ir-insts.h"

namespace Slang
{

struct CollectGlobalUniformParametersContext
{
    IRModule* module;

    IRVarLayout* globalScopeVarLayout;

    void processModule()
    {
        auto globalScopeTypeLayout = globalScopeVarLayout->getTypeLayout();
        auto globalParamsTypeLayout = globalScopeTypeLayout;

        IRParameterGroupTypeLayout* globalParameterGroupTypeLayout = as<IRParameterGroupTypeLayout>(globalParamsTypeLayout);
        if( globalParameterGroupTypeLayout )
        {
            globalParamsTypeLayout = globalParameterGroupTypeLayout->getElementVarLayout()->getTypeLayout();
        }

        // If the global scope did not require a parameter group to be allocated for it,
        // and/or the contents of the global scope do not include any uniform/ordinary
        // data, then we can safely skip this entire process.
        //
        if(!globalParameterGroupTypeLayout && !globalParamsTypeLayout->findSizeAttr(LayoutResourceKind::Uniform))
            return;

        auto globalParamsStructTypeLayout = as<IRStructTypeLayout>(globalParamsTypeLayout);
        SLANG_ASSERT(globalParamsStructTypeLayout);


        SharedIRBuilder sharedBuilder;
        sharedBuilder.module = this->module;
        sharedBuilder.session = module->session;

        IRBuilder builderStorage;
        IRBuilder* builder = &builderStorage;

        builder->sharedBuilder = &sharedBuilder;
        builder->setInsertInto(module->getModuleInst());

        auto wrapperStructType = builder->createStructType();
        builder->addNameHintDecoration(wrapperStructType, UnownedTerminatedStringSlice("GlobalParams"));

        IRType* wrapperParamType = wrapperStructType;
        if( globalParameterGroupTypeLayout )
        {
            auto wrapperParamGroupType = builder->getConstantBufferType(wrapperStructType);
            wrapperParamType = wrapperParamGroupType;
        }

        IRGlobalParam* wrapperParam = builder->createGlobalParam(wrapperParamType);
        builder->addLayoutDecoration(wrapperParam, globalScopeVarLayout);
        builder->addNameHintDecoration(wrapperParam, UnownedTerminatedStringSlice("globalParams"));

        for( auto fieldLayoutAttr : globalParamsStructTypeLayout->getFieldLayoutAttrs() )
        {
            auto globalParam = as<IRGlobalParam>(fieldLayoutAttr->getFieldKey());
            SLANG_ASSERT(globalParam);

            auto globalParamLayout = fieldLayoutAttr->getLayout();

            // TODO: attach the layout to the parameter if it doesn't already have one...

            // If the given parameter doesn't contribute to uniform/ordinary usage, then
            // we can safely leave it at the global scope and potentially avoid a lot
            // of complications that might otherwise arise (that is, we don't need to worry
            // about downstream passes that might have worked for a simple global parameter,
            // but that would not work for one nested inside a structure.
            //
            // TODO: It would probably be more consistent and robust to *always* wrap up
            // these global parameters appropriately, and ensure that all the downstream
            // passes can handle that case, since they would need to do so in general.
            //
            if(!globalParamLayout->getTypeLayout()->findSizeAttr(LayoutResourceKind::Uniform) )
                continue;

            builder->setInsertBefore(globalParam);

            auto globalParamType = globalParam->getFullType();

            // This global parameter needs to be turned into a field of the global
            // parameter structure type.
            //
            auto fieldKey = builder->createStructKey();
            globalParam->transferDecorationsTo(fieldKey);

            // TODO: transfer name related decorations over to the key

            fieldLayoutAttr->setOperand(0, fieldKey);

            builder->createStructField(wrapperStructType, fieldKey, globalParamType);

            List<IRUse*> uses;
            for(auto use = globalParam->firstUse; use; use = use->nextUse)
            {
                uses.add(use);
            }
            for( auto use : uses )
            {
                auto user = use->user;
                builder->setInsertBefore(user);

                IRInst* value = nullptr;
                if( globalParameterGroupTypeLayout )
                {
                    auto ptrType = builder->getPtrType(globalParamType);
                    auto fieldAddr = builder->emitFieldAddress(ptrType, wrapperParam, fieldKey);
                    value = builder->emitLoad(globalParamType, fieldAddr);
                }
                else
                {
                    value = builder->emitFieldExtract(globalParamType, wrapperParam, fieldKey);
                }

                use->set(value);
            }

            globalParam->removeAndDeallocate();
        }
    }
};

void collectGlobalUniformParameters(
    IRModule*       module,
    IRVarLayout*    globalScopeVarLayout)
{
    CollectGlobalUniformParametersContext context;
    context.module = module;
    context.globalScopeVarLayout = globalScopeVarLayout;

    context.processModule();
}

}
