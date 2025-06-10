// slang-ir-lower-optional-type.cpp

#include "slang-ir-lower-optional-type.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
enum LoweredOptionalTypeKind
{
    Struct,
    PtrValue,
    ExistentialValue,
};

struct OptionalTypeLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    OptionalTypeLoweringContext(IRModule* inModule)
        : module(inModule), workList(inModule), workListSet(inModule)
    {
    }

    struct LoweredOptionalTypeInfo : public RefObject
    {
        IRType* optionalType = nullptr;
        IRType* valueType = nullptr;
        IRType* loweredType = nullptr;
        IRStructKey* hasValueKey = nullptr;
        IRStructKey* valueKey = nullptr;
        LoweredOptionalTypeKind kind = LoweredOptionalTypeKind::Struct;
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

    IRInst* createOptionalStruct(IRType* type, LoweredOptionalTypeInfo* info)
    {
        IRBuilder builder(module);
        builder.setInsertInto(module->getModuleInst());

        info->valueKey = builder.createStructKey();
        builder.addNameHintDecoration(info->valueKey, UnownedStringSlice("value"));
        info->hasValueKey = builder.createStructKey();
        builder.addNameHintDecoration(info->hasValueKey, UnownedStringSlice("hasValue"));

        auto structType = builder.createStructType();
        StringBuilder sb;
        sb << "_slang_Optional_";
        getTypeNameHint(sb, type);
        builder.addNameHintDecoration(structType, sb.getUnownedSlice());
        builder.createStructField(structType, info->valueKey, type);
        builder.createStructField(structType, info->hasValueKey, builder.getBoolType());

        info->kind = LoweredOptionalTypeKind::Struct;
        return structType;
    }

    bool typeHasNullValue(IRInst* type, LoweredOptionalTypeKind& outKind)
    {
        switch (type->getOp())
        {
        case kIROp_ComPtrType:
        case kIROp_NativePtrType:
        case kIROp_NativeStringType:
        case kIROp_PtrType:
        case kIROp_ClassType:
            outKind = LoweredOptionalTypeKind::PtrValue;
            return true;
        case kIROp_InterfaceType:
            if (isComInterfaceType((IRType*)type))
                outKind = LoweredOptionalTypeKind::PtrValue;
            else
                outKind = LoweredOptionalTypeKind::ExistentialValue;
            return true;
        default:
            return false;
        }
    }

    LoweredOptionalTypeInfo* getLoweredOptionalType(IRBuilder*, IRInst* type)
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
        while (auto valueOptionalType = as<IROptionalType>(valueType))
        {
            // If the value type is also an Optional, we need to keep lowering it.
            valueType = valueOptionalType->getValueType();
        }

        info->optionalType = (IRType*)type;
        info->valueType = valueType;
        if (typeHasNullValue(valueType, info->kind))
        {
            info->loweredType = valueType;
        }
        else
        {
            info->loweredType = (IRType*)createOptionalStruct(valueType, info);
        }
        mapLoweredTypeToOptionalTypeInfo[info->loweredType] = info;
        loweredOptionalTypes[type] = info;
        return info.Ptr();
    }

    void addToWorkList(IRInst* inst)
    {
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
        else if (info->kind == LoweredOptionalTypeKind::ExistentialValue)
        {
            auto zero = builder->emitDefaultConstruct(info->loweredType);
            inst->replaceUsesWith(zero);
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
        switch (loweredOptionalTypeInfo->kind)
        {
        case LoweredOptionalTypeKind::Struct:
            result = builder->emitFieldExtract(
                builder->getBoolType(),
                optionalInst,
                loweredOptionalTypeInfo->hasValueKey);
            break;
        case LoweredOptionalTypeKind::PtrValue:
            result = builder->emitCastPtrToBool(optionalInst);
            break;
        case LoweredOptionalTypeKind::ExistentialValue:
            result = builder->emitIsNullExistential(optionalInst);
            break;
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
        if (loweredOptionalTypeInfo->kind == LoweredOptionalTypeKind::Struct)
        {
            SLANG_ASSERT(loweredOptionalTypeInfo);
            auto getElement = builder->emitFieldExtract(
                loweredOptionalTypeInfo->valueType,
                base,
                loweredOptionalTypeInfo->valueKey);
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
} // namespace Slang
