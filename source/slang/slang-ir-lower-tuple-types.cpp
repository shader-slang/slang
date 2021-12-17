// slang-ir-lower-tuple-types.cpp

#include "slang-ir-lower-tuple-types.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
    struct TupleLoweringContext
    {
        IRModule* module;
        DiagnosticSink* sink;

        SharedIRBuilder sharedBuilderStorage;

        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;

        struct LoweredTupleInfo : public RefObject
        {
            IRType* tupleType;
            IRStructType* structType;
            List<IRStructField*> fields;
        };
        Dictionary<IRInst*, RefPtr<LoweredTupleInfo>> mapLoweredStructToTupleInfo;
        Dictionary<IRInst*, RefPtr<LoweredTupleInfo>> loweredTuples;

        IRType* maybeLowerTupleType(IRBuilder* builder, IRType* type)
        {
            if (auto info = getLoweredTupleType(builder, type))
                return info->structType;
            else
                return type;
        }

        LoweredTupleInfo* getLoweredTupleType(IRBuilder* builder, IRInst* type)
        {
            if (auto loweredInfo = loweredTuples.TryGetValue(type))
                return loweredInfo->Ptr();
            if (auto loweredInfo = mapLoweredStructToTupleInfo.TryGetValue(type))
                return loweredInfo->Ptr();

            if (!type)
                return nullptr;
            if (type->getOp() != kIROp_TupleType)
                return nullptr;

            RefPtr<LoweredTupleInfo> info = new LoweredTupleInfo();
            info->tupleType = (IRType*)type;
            auto structType = builder->createStructType();
            info->structType = structType;
            builder->addNameHintDecoration(structType, UnownedStringSlice("Tuple"));

            StringBuilder fieldNameSb;
            for (UInt i = 0; i < type->getOperandCount(); i++)
            {
                auto elementType = maybeLowerTupleType(builder, (IRType*)(type->getOperand(i)));
                auto key = builder->createStructKey();
                fieldNameSb.Clear();
                fieldNameSb << "value" << i;
                builder->addNameHintDecoration(key, fieldNameSb.getUnownedSlice());
                auto field = builder->createStructField(structType, key, (IRType*)elementType);
                info->fields.add(field);
            }
            mapLoweredStructToTupleInfo[structType] = info;
            loweredTuples[type] = info;
            return info.Ptr();
        }

        void addToWorkList(
            IRInst* inst)
        {
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

        void processMakeTuple(IRMakeTuple* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto info = getLoweredTupleType(builder, inst->getDataType());
            List<IRInst*> operands;
            for (Index i = 0; i < info->fields.getCount(); i++)
            {
                SLANG_ASSERT(i < (Index)inst->getOperandCount());
                operands.add(inst->getOperand(i));
            }
            auto makeStruct = builder->emitMakeStruct(info->structType, operands);
            inst->replaceUsesWith(makeStruct);
            inst->removeAndDeallocate();
        }

        void processGetTupleElement(IRGetTupleElement* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto base = inst->getTuple();
            auto loweredTupleInfo = getLoweredTupleType(builder, base->getDataType());
            SLANG_ASSERT(loweredTupleInfo);
            auto elementIndex = getIntVal(inst->getElementIndex());
            SLANG_ASSERT((Index)elementIndex < loweredTupleInfo->fields.getCount());

            auto field = loweredTupleInfo->fields[(Index)elementIndex];
            auto getElement = builder->emitFieldExtract(field->getFieldType(), base, field->getKey());
            inst->replaceUsesWith(getElement);
            inst->removeAndDeallocate();
        }

        void processTupleType(IRTupleType* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto loweredTupleInfo = getLoweredTupleType(builder, inst);
            SLANG_ASSERT(loweredTupleInfo);
            SLANG_UNUSED(loweredTupleInfo);
        }

        void processInst(IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_MakeTuple:
                processMakeTuple((IRMakeTuple*)inst);
                break;
            case kIROp_GetTupleElement:
                processGetTupleElement((IRGetTupleElement*)inst);
                break;
            case kIROp_TupleType:
                processTupleType((IRTupleType*)inst);
                break;
            default:
                break;
            }
        }

        void processModule()
        {
            SharedIRBuilder* sharedBuilder = &sharedBuilderStorage;
            sharedBuilder->init(module);

            // Deduplicate equivalent types.
            sharedBuilder->deduplicateAndRebuildGlobalNumberingMap();

            addToWorkList(module->getModuleInst());

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

            // Replace all tuple types with lowered struct types.
            for (auto kv : loweredTuples)
            {
                kv.Key->replaceUsesWith(kv.Value->structType);
            }
        }
    };
    
    void lowerTuples(IRModule* module, DiagnosticSink* sink)
    {
        TupleLoweringContext context;
        context.module = module;
        context.sink = sink;
        context.processModule();
    }
}
