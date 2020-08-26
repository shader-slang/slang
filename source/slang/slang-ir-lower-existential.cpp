// slang-ir-lower-generic-existential.cpp

#include "slang-ir-lower-existential.h"
#include "slang-ir-generics-lowering-context.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
    struct ExistentialLoweringContext
    {
        SharedGenericsLoweringContext* sharedContext;

        void processMakeExistential(IRMakeExistentialWithRTTI* inst)
        {
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(inst);

            auto value = inst->getWrappedValue();
            auto valueType = sharedContext->lowerType(builder, value->getDataType());
            auto witnessTableType = cast<IRWitnessTableType>(inst->getWitnessTable()->getDataType());
            auto interfaceType = witnessTableType->getConformanceType();
            auto anyValueSize = sharedContext->getInterfaceAnyValueSize(interfaceType, inst->sourceLoc);
            auto anyValueType = builder->getAnyValueType(anyValueSize);
            auto rttiType = builder->getPtrType(builder->getRTTIType());
            auto tupleType = builder->getTupleType(rttiType, witnessTableType, anyValueType);

            IRInst* rttiObject = inst->getRTTI();
            if (auto type = as<IRType>(rttiObject))
            {
                rttiObject = sharedContext->maybeEmitRTTIObject(type);
                rttiObject = builder->emitGetAddress(rttiType, rttiObject);
            }
            IRInst* packedValue = value;
            if (valueType->op != kIROp_AnyValueType)
                packedValue = builder->emitPackAnyValue(anyValueType, value);
            IRInst* tupleArgs[] = {rttiObject, inst->getWitnessTable(), packedValue};
            auto tuple = builder->emitMakeTuple(tupleType, 3, tupleArgs);
            inst->replaceUsesWith(tuple);
            inst->removeAndDeallocate();
        }

        IRInst* extractTupleElement(IRBuilder* builder, IRInst* value, UInt index)
        {
            auto tupleType = cast<IRTupleType>(sharedContext->lowerType(builder, value->getDataType()));
            auto getElement = builder->emitGetTupleElement(
                (IRType*)tupleType->getOperand(index),
                value,
                index);
            return getElement;
        }

        void processExtractExistentialElement(IRInst* extractInst, UInt elementId)
        {
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(extractInst);

            auto element = extractTupleElement(builder, extractInst->getOperand(0), elementId);
            extractInst->replaceUsesWith(element);
            extractInst->removeAndDeallocate();
        }

        void processExtractExistentialValue(IRExtractExistentialValue* inst)
        {
            processExtractExistentialElement(inst, 2);
        }

        void processExtractExistentialWitnessTable(IRExtractExistentialWitnessTable* inst)
        {
            processExtractExistentialElement(inst, 1);
        }

        void processExtractExistentialType(IRExtractExistentialType* inst)
        {
            processExtractExistentialElement(inst, 0);
        }

        void processInst(IRInst* inst)
        {
            if (auto makeExistential = as<IRMakeExistentialWithRTTI>(inst))
            {
                processMakeExistential(makeExistential);
            }
            else if (auto extractExistentialVal = as<IRExtractExistentialValue>(inst))
            {
                processExtractExistentialValue(extractExistentialVal);
            }
            else if (auto extractExistentialType = as<IRExtractExistentialType>(inst))
            {
                processExtractExistentialType(extractExistentialType);
            }
            else if (auto extractExistentialWitnessTable = as<IRExtractExistentialWitnessTable>(inst))
            {
                processExtractExistentialWitnessTable(extractExistentialWitnessTable);
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


    void lowerExistentials(SharedGenericsLoweringContext* sharedContext)
    {
        ExistentialLoweringContext context;
        context.sharedContext = sharedContext;
        context.processModule();
    }
}
