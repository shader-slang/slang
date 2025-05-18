// slang-ir-lower-result-type.cpp

#include "slang-ir-lower-result-type.h"

#include "slang-ir-insts.h"
#include "slang-ir-any-value-marshalling.h"
#include "slang-ir.h"

namespace Slang
{
struct ResultTypeLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    ResultTypeLoweringContext(IRModule* inModule)
        : module(inModule), workList(inModule), workListSet(inModule)
    {
    }

    struct LoweredResultTypeInfo : public RefObject
    {
        IRType* resultType = nullptr;
        IRType* loweredType = nullptr;
        IRType* tagType = nullptr;
        IRType* valueType = nullptr;
        IRType* errorType = nullptr;
        IRType* anyValueType = nullptr;
        IRStructField* tagField = nullptr;
        IRStructField* anyValueField = nullptr;
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
        if (auto loweredInfo = loweredResultTypes.tryGetValue(type))
            return loweredInfo->Ptr();
        if (auto loweredInfo = mapLoweredTypeToResultTypeInfo.tryGetValue(type))
            return loweredInfo->Ptr();

        if (!type)
            return nullptr;
        if (type->getOp() != kIROp_ResultType)
            return nullptr;

        RefPtr<LoweredResultTypeInfo> info = new LoweredResultTypeInfo();
        auto resultType = cast<IRResultType>(type);
        info->resultType = (IRType*)type;

        auto structType = builder->createStructType();
        info->loweredType = structType;
        builder->addNameHintDecoration(structType, UnownedStringSlice("ResultType"));

        info->tagType = builder->getBoolType();
        auto tagKey = builder->createStructKey();
        builder->addNameHintDecoration(tagKey, UnownedStringSlice("tag"));
        info->tagField = builder->createStructField(structType, tagKey, info->tagType);

        SlangInt anyValueSize = 0;
        auto valueType = resultType->getValueType();
        if (valueType->getOp() != kIROp_VoidType)
        {
            anyValueSize = getAnyValueSize(valueType);
            info->valueType = valueType;
        }

        auto errorType = resultType->getErrorType();
        info->errorType = errorType;

        auto errSize = getAnyValueSize(errorType);
        if (errSize > anyValueSize)
            anyValueSize = errSize;

        info->anyValueType = builder->getAnyValueType(builder->getIntValue(builder->getUIntType(), anyValueSize));
        auto anyValueKey = builder->createStructKey();
        builder->addNameHintDecoration(anyValueKey, UnownedStringSlice("anyValue"));
        info->anyValueField = builder->createStructField(structType, anyValueKey, info->anyValueType);

        mapLoweredTypeToResultTypeInfo[info->loweredType] = info;
        loweredResultTypes[type] = info;
        return info.Ptr();
    }

    void addToWorkList(IRInst* inst)
    {
        for (auto ii = inst->getParent(); ii; ii = ii->getParent())
        {
            if (as<IRGeneric>(ii))
                return;
        }

        if (workListSet.contains(inst))
            return;

        workList.add(inst);
        workListSet.add(inst);
    }

    void processMakeResultValue(IRMakeResultValue* inst)
    {
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(inst);

        auto info = getLoweredResultType(builder, inst->getDataType());

        List<IRInst*> operands;
        operands.add(builder->getBoolValue(false));
        auto packInst = builder->emitPackAnyValue(
            info->anyValueType,
            info->valueType ? inst->getOperand(0) : builder->emitDefaultConstruct(info->errorType));
        operands.add(packInst);

        auto makeStruct = builder->emitMakeStruct(info->loweredType, operands);
        inst->replaceUsesWith(makeStruct);
        inst->removeAndDeallocate();
    }

    void processMakeResultError(IRMakeResultError* inst)
    {
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(inst);

        auto info = getLoweredResultType(builder, inst->getDataType());

        auto packInst = builder->emitPackAnyValue(info->anyValueType, inst->getErrorValue());

        List<IRInst*> operands;
        operands.add(builder->getBoolValue(true));
        operands.add(packInst);

        auto makeStruct = builder->emitMakeStruct(info->loweredType, operands);
        inst->replaceUsesWith(makeStruct);
        inst->removeAndDeallocate();
    }

    IRInst* getResultError(IRBuilder* builder, IRInst* resultInst)
    {
        auto loweredResultTypeInfo = getLoweredResultType(builder, resultInst->getDataType());
        SLANG_ASSERT(loweredResultTypeInfo);
        if (loweredResultTypeInfo->valueType)
        {
            auto value = builder->emitFieldExtract(
                loweredResultTypeInfo->anyValueType,
                resultInst,
                loweredResultTypeInfo->anyValueField->getKey());
            auto unpackInst = builder->emitUnpackAnyValue(
                loweredResultTypeInfo->errorType, value);
            return unpackInst;
        }
        else
        {
            return resultInst;
        }
    }

    void processGetResultError(IRGetResultError* inst)
    {
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(inst);

        auto resultValue = inst->getResultOperand();
        auto errValue = getResultError(builder, resultValue);
        inst->replaceUsesWith(errValue);
        inst->removeAndDeallocate();
    }

    void processGetResultValue(IRGetResultValue* inst)
    {
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(inst);

        auto base = inst->getResultOperand();
        auto loweredResultTypeInfo = getLoweredResultType(builder, base->getDataType());
        SLANG_ASSERT(loweredResultTypeInfo);
        SLANG_ASSERT(loweredResultTypeInfo->valueType);

        auto getElement = builder->emitFieldExtract(
            loweredResultTypeInfo->anyValueType,
            base,
            loweredResultTypeInfo->anyValueField->getKey());
        auto unpackInst = builder->emitUnpackAnyValue(
            loweredResultTypeInfo->valueType, getElement);
        inst->replaceUsesWith(unpackInst);
        inst->removeAndDeallocate();
    }

    void processIsResultError(IRIsResultError* inst)
    {
        IRBuilder builderStorage(module);
        auto builder = &builderStorage;
        builder->setInsertBefore(inst);

        auto base = inst->getResultOperand();
        auto loweredResultTypeInfo = getLoweredResultType(builder, base->getDataType());
        SLANG_ASSERT(loweredResultTypeInfo);

        auto isFailure = builder->emitFieldExtract(
            loweredResultTypeInfo->tagType,
            base,
            loweredResultTypeInfo->tagField->getKey());

        inst->replaceUsesWith(isFailure);
        inst->removeAndDeallocate();
    }

    void processResultType(IRResultType* inst)
    {
        IRBuilder builderStorage(module);
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
        addToWorkList(module->getModuleInst());

        while (workList.getCount() != 0)
        {
            IRInst* inst = workList.getLast();

            workList.removeLast();
            workListSet.remove(inst);

            processInst(inst);

            for (auto child = inst->getLastChild(); child; child = child->getPrevInst())
            {
                addToWorkList(child);
            }
        }

        // Replace all result types with lowered struct types.
        for (const auto& [key, value] : loweredResultTypes)
            key->replaceUsesWith(value->loweredType);
    }
};

void lowerResultType(IRModule* module, DiagnosticSink* sink)
{
    ResultTypeLoweringContext context(module);
    context.sink = sink;
    context.processModule();
}
} // namespace Slang
