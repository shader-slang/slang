// slang-ir-lower-optional-type.cpp

#include "slang-ir-lower-optional-type.h"
#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"

namespace Slang
{
    struct OptionalTypeLoweringContext
    {
        IRModule* module;
        DiagnosticSink* sink;

        InstWorkList workList;
        InstHashSet workListSet;

        OptionalTypeLoweringContext(IRModule* inModule)
            :module(inModule), workList(inModule), workListSet(inModule)
        {}

        struct LoweredOptionalTypeInfo : public RefObject
        {
            IRType* optionalType = nullptr;
            IRType* valueType = nullptr;
            IRType* loweredType = nullptr;
            IRStructField* valueField = nullptr;
            IRStructField* hasValueField = nullptr;
        };
        Dictionary<IRInst*, RefPtr<LoweredOptionalTypeInfo>> mapLoweredTypeToOptionalTypeInfo;
        Dictionary<IRInst*, RefPtr<LoweredOptionalTypeInfo>> loweredOptionalTypes;

        IRType* maybeLowerOptionalType(IRBuilder* builder, IRType* type)
        {
            if (auto info = getLoweredOptionalType(builder, type))
                return info->loweredType;
            else
                return type;
        }

        bool typeHasNullValue(IRInst* type)
        {
            switch (type->getOp())
            {
            case kIROp_ComPtrType:
            case kIROp_NativePtrType:
            case kIROp_NativeStringType:
            case kIROp_PtrType:
            case kIROp_ClassType:
                return true;
            case kIROp_InterfaceType:
                return isComInterfaceType((IRType*)type);
            default:
                return false;
            }
        }

        LoweredOptionalTypeInfo* getLoweredOptionalType(IRBuilder* builder, IRInst* type)
        {
            if (auto loweredInfo = loweredOptionalTypes.tryGetValue(type))
                return loweredInfo->Ptr();
            if (auto loweredInfo = mapLoweredTypeToOptionalTypeInfo.tryGetValue(type))
                return loweredInfo->Ptr();

            if (!type)
                return nullptr;
            if (type->getOp() != kIROp_OptionalType)
                return nullptr;

            RefPtr<LoweredOptionalTypeInfo> info = new LoweredOptionalTypeInfo();
            auto optionalType = cast<IROptionalType>(type);
            auto valueType = optionalType->getValueType();
            info->optionalType = (IRType*)type;
            info->valueType = valueType;
            if (typeHasNullValue(valueType))
            {
                info->loweredType = valueType;
            }
            else
            {
                auto structType = builder->createStructType();
                info->loweredType = structType;
                builder->addNameHintDecoration(structType, UnownedStringSlice("OptionalType"));

                info->valueType = valueType;
                auto valueKey = builder->createStructKey();
                builder->addNameHintDecoration(valueKey, UnownedStringSlice("value"));
                info->valueField = builder->createStructField(structType, valueKey, (IRType*)valueType);

                auto boolType = builder->getBoolType();
                auto hasValueKey = builder->createStructKey();
                builder->addNameHintDecoration(hasValueKey, UnownedStringSlice("hasValue"));
                info->hasValueField = builder->createStructField(structType, hasValueKey, (IRType*)boolType);
            }
            mapLoweredTypeToOptionalTypeInfo[info->loweredType] = info;
            loweredOptionalTypes[type] = info;
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

            if (workListSet.contains(inst))
                return;

            workList.add(inst);
            workListSet.add(inst);
        }

        void processMakeOptionalValue(IRMakeOptionalValue* inst)
        {
            IRBuilder builderStorage(module);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto info = getLoweredOptionalType(builder, inst->getDataType());
            if (info->loweredType != info->valueType)
            {
                List<IRInst*> operands;
                operands.add(inst->getOperand(0));
                operands.add(builder->getBoolValue(true));
                auto makeStruct = builder->emitMakeStruct(info->loweredType, operands);
                inst->replaceUsesWith(makeStruct);
                inst->removeAndDeallocate();
            }
            else
            {
                inst->replaceUsesWith(inst->getOperand(0));
                inst->removeAndDeallocate();
            }
        }

        void processMakeOptionalNone(IRMakeOptionalNone* inst)
        {
            IRBuilder builderStorage(module);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto info = getLoweredOptionalType(builder, inst->getDataType());
            if (info->loweredType != info->valueType)
            {
                List<IRInst*> operands;
                operands.add(inst->getDefaultValue());
                operands.add(builder->getBoolValue(false));
                auto makeStruct = builder->emitMakeStruct(info->loweredType, operands);
                inst->replaceUsesWith(makeStruct);
                inst->removeAndDeallocate();
            }
            else
            {
                inst->replaceUsesWith(builder->getNullPtrValue(info->valueType));
                inst->removeAndDeallocate();
            }
        }

        IRInst* getOptionalHasValue(IRBuilder* builder, IRInst* optionalInst)
        {
            auto loweredOptionalTypeInfo = getLoweredOptionalType(builder, optionalInst->getDataType());
            SLANG_ASSERT(loweredOptionalTypeInfo);
            IRInst* result = nullptr;
            if (loweredOptionalTypeInfo->loweredType != loweredOptionalTypeInfo->valueType)
            {
                result = builder->emitFieldExtract(
                    builder->getBoolType(),
                    optionalInst,
                    loweredOptionalTypeInfo->hasValueField->getKey());
            }
            else
            {
                result = builder->emitCastPtrToBool(optionalInst);
            }
            return result;
        }

        void processGetOptionalHasValue(IROptionalHasValue* inst)
        {
            IRBuilder builderStorage(module);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto optionalValue = inst->getOptionalOperand();
            auto hasVal = getOptionalHasValue(builder, optionalValue);
            inst->replaceUsesWith(hasVal);
            inst->removeAndDeallocate();
        }

        void processGetOptionalValue(IRGetOptionalValue* inst)
        {
            IRBuilder builderStorage(module);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto base = inst->getOptionalOperand();
            auto loweredOptionalTypeInfo = getLoweredOptionalType(builder, base->getDataType());
            if (loweredOptionalTypeInfo->loweredType != loweredOptionalTypeInfo->valueType)
            {
                SLANG_ASSERT(loweredOptionalTypeInfo);
                SLANG_ASSERT(loweredOptionalTypeInfo->valueField);
                auto getElement = builder->emitFieldExtract(
                    loweredOptionalTypeInfo->valueType,
                    base,
                    loweredOptionalTypeInfo->valueField->getKey());
                inst->replaceUsesWith(getElement);
            }
            else
            {
                inst->replaceUsesWith(base);
            }
            inst->removeAndDeallocate();
        }

        void processOptionalType(IROptionalType* inst)
        {
            IRBuilder builderStorage(module);
            auto builder = &builderStorage;
            builder->setInsertBefore(inst);

            auto loweredOptionalTypeInfo = getLoweredOptionalType(builder, inst);
            SLANG_ASSERT(loweredOptionalTypeInfo);
            SLANG_UNUSED(loweredOptionalTypeInfo);
        }

        void processInst(IRInst* inst)
        {
            switch (inst->getOp())
            {
            case kIROp_MakeOptionalValue:
                processMakeOptionalValue((IRMakeOptionalValue*)inst);
                break;
            case kIROp_MakeOptionalNone:
                processMakeOptionalNone((IRMakeOptionalNone*)inst);
                break;
            case kIROp_OptionalHasValue:
                processGetOptionalHasValue((IROptionalHasValue*)inst);
                break;
            case kIROp_GetOptionalValue:
                processGetOptionalValue((IRGetOptionalValue*)inst);
                break;
            case kIROp_OptionalType:
                processOptionalType((IROptionalType*)inst);
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

            // Replace all optional types with lowered struct types.
            for (const auto& [key, value] : loweredOptionalTypes)
                key->replaceUsesWith(value->loweredType);
        }
    };
    
    void lowerOptionalType(IRModule* module, DiagnosticSink* sink)
    {
        OptionalTypeLoweringContext context(module);
        context.sink = sink;
        context.processModule();
    }
}
