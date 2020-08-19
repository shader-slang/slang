// slang-ir-lower-generic-type.cpp
#include "slang-ir-lower-generic-type.h"

#include "slang-ir-generics-lowering-context.h"
#include "slang-ir.h"
#include "slang-ir-clone.h"
#include "slang-ir-insts.h"

namespace Slang
{
    // This is a subpass of generics lowering IR transformation.
    // This pass lowers all generic/polymorphic types into IRAnyValueType.
    struct GenericVarLoweringContext
    {
        SharedGenericsLoweringContext* sharedContext;

        void processInst(IRInst* inst)
        {
            // If inst is a type itself, keep its type.
            if (as<IRType>(inst))
                return;

            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(inst);
           
            auto newType = sharedContext->lowerType(builder, inst->getFullType());
            if (newType != inst->getFullType())
                inst->setFullType((IRType*)newType);
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
            sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();
        }
    };

    void lowerGenericType(SharedGenericsLoweringContext* sharedContext)
    {
        GenericVarLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }
}

