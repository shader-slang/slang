// slang-ir-lower-result-type.cpp

#include "slang-ir-lower-result-type.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"

namespace Slang
{
    struct ResultTypeLoweringContext
    {
        IRModule* module;
        DiagnosticSink* sink;

        SharedIRBuilder sharedBuilderStorage;

        List<IRInst*> workList;
        HashSet<IRInst*> workListSet;

        struct LoweredResultTypeInfo : public RefObject
        {
            IRType* resultType = nullptr;
            IRType* errorType = nullptr;
            IRType* valueType = nullptr;
            IRType* loweredType = nullptr;
            IRStructField* valueField = nullptr;
            IRStructField* errorField = nullptr;
        };
        Dictionary<IRInst*, RefPtr<LoweredResultTypeInfo>> mapLoweredTypeToResultTypeInfo;
        Dictionary<IRInst*, RefPtr<LoweredResultTypeInfo>> loweredResultTypes;

        IRType* maybeLowerResultType(IRBuilder* builder, IRType* type)
        {
            if (auto info = getLoweredResultType(builder, type))
                return info->loweredType;
            else
                return type;
        }

        LoweredResultTypeInfo* getLoweredResultType(IRBuilder* builder, IRInst* type)
        {
            if (auto loweredInfo = loweredResultTypes.TryGetValue(type))
                return loweredInfo->Ptr();
            if (auto loweredInfo = mapLoweredTypeToResultTypeInfo.TryGetValue(type))
                return loweredInfo->Ptr();

            if (!type)
                return nullptr;
            if (type->getOp() != kIROp_ResultType)
                return nullptr;

            RefPtr<LoweredResultTypeInfo> info = new LoweredResultTypeInfo();
            info->resultType = (IRType*)type;
            info->errorType = cast<IRResultType>(type)->getErrorType();
            
            auto resultType = cast<IRResultType>(type);
            auto valueType = resultType->getValueType();
            if (valueType->getOp() != kIROp_VoidType)
            {
                auto structType = builder->createStructType();
                info->loweredType = structType;
                builder->addNameHintDecoration(structType, UnownedStringSlice("ResultType"));

                info->valueType = valueType;
                auto valueKey = builder->createStructKey();
                builder->addNameHintDecoration(valueKey, UnownedStringSlice("value"));
                info->valueField = builder->createStructField(structType, valueKey, (IRType*)valueType);

                auto errorType = resultType->getErrorType();
                auto errorKey = builder->createStructKey();
                builder->addNameHintDecoration(errorKey, UnownedStringSlice("error"));
                info->errorField = builder->createStructField(structType, errorKey, (IRType*)errorType);
            }
            else
            {
                info->loweredType = resultType->getErrorType();
            }
            mapLoweredTypeToResultTypeInfo[info->loweredType] = info;
            loweredResultTypes[type] = info;
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

        IRInst* getSuccessErrorValue(IRType* type)
        {
            switch (type->getOp())
            {
            case kIROp_Int8Type:
            case kIROp_Int16Type:
            case kIROp_IntType:
            case kIROp_Int64Type:
            case kIROp_UInt8Type:
            case kIROp_UInt16Type:
            case kIROp_UIntType:
            case kIROp_UInt64Type:
                break;
            default:
                SLANG_ASSERT_FAILURE("error type is not lowered to an integer type.");
            }
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertInto(module);
            return builder->getIntValue(type, 0);
        }

        void processMakeResultValue(IRMakeResultValue* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto info = getLoweredResultType(builder, inst->getDataType());
            if (info->loweredType->getOp() == kIROp_StructType)
            {
                List<IRInst*> operands;
                operands.add(inst->getOperand(0));
                operands.add(getSuccessErrorValue(info->errorType));
                auto makeStruct = builder->emitMakeStruct(info->loweredType, operands);
                inst->replaceUsesWith(makeStruct);
            }
            else
            {
                auto errCode = getSuccessErrorValue(info->errorType);
                inst->replaceUsesWith(errCode);
            }
            inst->removeAndDeallocate();
        }

        void processMakeResultError(IRMakeResultError* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto info = getLoweredResultType(builder, inst->getDataType());
            if (info->valueField)
            {
                List<IRInst*> operands;
                operands.add(builder->emitConstructorInst(info->valueType, 0, nullptr));
                operands.add(inst->getErrorValue());
                auto makeStruct = builder->emitMakeStruct(info->loweredType, operands);
                inst->replaceUsesWith(makeStruct);
            }
            else
            {
                inst->replaceUsesWith(inst->getErrorValue());
            }
            inst->removeAndDeallocate();
        }

        IRInst* getResultError(IRBuilder* builder, IRInst* resultInst)
        {
            auto loweredResultTypeInfo = getLoweredResultType(builder, resultInst->getDataType());
            SLANG_ASSERT(loweredResultTypeInfo);
            if (loweredResultTypeInfo->valueField)
            {
                auto value = builder->emitFieldExtract(
                    loweredResultTypeInfo->errorType,
                    resultInst,
                    loweredResultTypeInfo->errorField->getKey());
                return value;
            }
            else
            {
                return resultInst;
            }
        }

        void processGetResultError(IRGetResultError* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto resultValue = inst->getResultOperand();
            auto errValue = getResultError(builder, resultValue);
            inst->replaceUsesWith(errValue);
            inst->removeAndDeallocate();
        }

        void processGetResultValue(IRGetResultValue* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto base = inst->getResultOperand();
            auto loweredResultTypeInfo = getLoweredResultType(builder, base->getDataType());
            SLANG_ASSERT(loweredResultTypeInfo);
            SLANG_ASSERT(loweredResultTypeInfo->valueField);
            auto getElement = builder->emitFieldExtract(
                loweredResultTypeInfo->errorType,
                base,
                loweredResultTypeInfo->valueField->getKey());
            inst->replaceUsesWith(getElement);
            inst->removeAndDeallocate();
        }

        void processIsResultError(IRIsResultError* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto base = inst->getResultOperand();
            auto loweredResultTypeInfo = getLoweredResultType(builder, base->getDataType());
            SLANG_ASSERT(loweredResultTypeInfo);
            SLANG_ASSERT(loweredResultTypeInfo->valueField);

            auto resultValue = inst->getResultOperand();
            auto errValue = getResultError(builder, resultValue);
            auto isSuccess =
                builder->emitNeq(errValue, getSuccessErrorValue(loweredResultTypeInfo->errorType));
            inst->replaceUsesWith(isSuccess);
            inst->removeAndDeallocate();
        }

        void processResultType(IRResultType* inst)
        {
            IRBuilder builderStorage(sharedBuilderStorage);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto loweredResultTypeInfo = getLoweredResultType(builder, inst);
            SLANG_ASSERT(loweredResultTypeInfo);
            SLANG_UNUSED(loweredResultTypeInfo);
        }

        void processInst(IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_MakeResultValue:
                processMakeResultValue((IRMakeResultValue*)inst);
                break;
            case kIROp_MakeResultError:
                processMakeResultError((IRMakeResultError*)inst);
                break;
            case kIROp_GetResultError:
                processGetResultError((IRGetResultError*)inst);
                break;
            case kIROp_GetResultValue:
                processGetResultValue((IRGetResultValue*)inst);
                break;
            case kIROp_IsResultError:
                processIsResultError((IRIsResultError*)inst);
                break;
            case kIROp_ResultType:
                processResultType((IRResultType*)inst);
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

            // Replace all result types with lowered struct types.
            for (auto kv : loweredResultTypes)
            {
                kv.Key->replaceUsesWith(kv.Value->loweredType);
            }
        }
    };
    
    void lowerResultType(IRModule* module, DiagnosticSink* sink)
    {
        ResultTypeLoweringContext context;
        context.module = module;
        context.sink = sink;
        context.processModule();
    }
}
