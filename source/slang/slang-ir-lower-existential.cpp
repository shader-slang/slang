// slang-ir-lower-generic-existential.cpp

#include "slang-ir-lower-existential.h"
#include "slang-ir-generics-lowering-context.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
    bool isCPUTarget(TargetRequest* targetReq);
    bool isCUDATarget(TargetRequest* targetReq);

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
            auto witnessTableType = cast<IRWitnessTableTypeBase>(inst->getWitnessTable()->getDataType());
            auto interfaceType = witnessTableType->getConformanceType();
            auto witnessTableIdType = builder->getWitnessTableIDType((IRType*)interfaceType);
            auto anyValueSize = sharedContext->getInterfaceAnyValueSize(interfaceType, inst->sourceLoc);
            auto anyValueType = builder->getAnyValueType(anyValueSize);
            auto rttiType = builder->getRTTIHandleType();
            auto tupleType = builder->getTupleType(rttiType, witnessTableIdType, anyValueType);

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

        void processGetValueFromBoundInterface(IRGetValueFromBoundInterface* inst)
        {
            IRBuilder builderStorage;
            auto builder = &builderStorage;
            builder->sharedBuilder = &sharedContext->sharedBuilderStorage;
            builder->setInsertBefore(inst);

            // A value of interface will lower as a tuple, and
            // the third element of that tuple represents the
            // concrete value that was put into the existential.
            //
            auto element = extractTupleElement(builder, inst->getOperand(0), 2);
            auto elementType = element->getDataType();

            // There are two cases we expect to see for that
            // tuple element.
            //
            IRInst* replacement = nullptr;
            if(as<IRPseudoPtrType>(elementType))
            {
                // The first case is when legacy static specialization
                // is applied, and the element is a "pseudo-pointer."
                //
                // Semantically, we should emit a (pseudo-)load from the pseudo-pointer
                // to go from `PseudoPtr<T>` to `T`.
                //
                // TODO: Actually introduce and emit a "psedudo-load" instruction
                // here. For right now we are just using the value directly and
                // downstream passes seem okay with it, but it isn't really
                // type-correct to be doing this.
                //
                replacement = element;
            }
            else
            {
                // The second case is when the dynamic-dispatch layout is
                // being used, and the element is an "any-value."
                //
                // In this case we need to emit an unpacking operation
                // to get from `AnyValue` to `T`.
                //
                SLANG_ASSERT(as<IRAnyValueType>(elementType));
                replacement = builder->emitUnpackAnyValue(inst->getFullType(), element);
            }

            inst->replaceUsesWith(replacement);
            inst->removeAndDeallocate();
        }

        void processInst(IRInst* inst)
        {
            if (auto makeExistential = as<IRMakeExistentialWithRTTI>(inst))
            {
                processMakeExistential(makeExistential);
            }
            else if (auto getValueFromBoundInterface = as<IRGetValueFromBoundInterface>(inst))
            {
                processGetValueFromBoundInterface(getValueFromBoundInterface);
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
