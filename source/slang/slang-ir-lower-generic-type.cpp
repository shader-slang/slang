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
    struct GenericTypeLoweringContext
    {
        SharedGenericsLoweringContext* sharedContext;

        void processInst(IRInst* inst)
        {
            // Ensure public struct types has RTTI object defined.
            if (as<IRStructType>(inst))
            {
                if (inst->findDecoration<IRPublicDecoration>())
                {
                    sharedContext->maybeEmitRTTIObject(inst);
                }
            }

            // Don't modify type insts themselves.
            if (as<IRType>(inst))
                return;

            IRBuilder builderStorage(sharedContext->sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);
           
            auto newType = sharedContext->lowerType(builder, inst->getFullType());
            if (newType != inst->getFullType())
                inst->setFullType((IRType*)newType);

            switch (inst->getOp())
            {
            default:
                break;
            case kIROp_StructField:
                {
                    // Translate the struct field type.
                    auto structField = static_cast<IRStructField*>(inst);
                    auto loweredFieldType =
                        sharedContext->lowerType(builder, structField->getFieldType());
                    structField->setOperand(1, loweredFieldType);
                }
                break;
            }
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
            sharedContext->sharedBuilderStorage.deduplicateAndRebuildGlobalNumberingMap();
            sharedContext->mapInterfaceRequirementKeyValue.Clear();
        }
    };

    void lowerGenericType(SharedGenericsLoweringContext* sharedContext)
    {
        GenericTypeLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }
}

