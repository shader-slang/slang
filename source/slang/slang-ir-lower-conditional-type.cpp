#include "slang-ir-lower-conditional-type.h"

#include "slang-ir-insts.h"
#include "slang-ir.h"

namespace Slang
{
struct ConditionalTypeLoweringContext
{
    IRModule* module;
    DiagnosticSink* sink;

    InstWorkList workList;
    InstHashSet workListSet;

    struct LoweredConditionalTypeInfo
    {
        IRType* loweredType;
        bool hasValue;
    };
    Dictionary<IRConditionalType*, LoweredConditionalTypeInfo> loweredConditionalTypes;
    IRType* emptyStructType = nullptr;

    ConditionalTypeLoweringContext(IRModule* inModule)
        : module(inModule), workList(inModule), workListSet(inModule)
    {
    }

    IRType* getEmptyStructType()
    {
        if (!emptyStructType)
        {
            IRBuilder builder(module);
            builder.setInsertInto(module->getModuleInst());
            auto emptyStruct = builder.createStructType();
            builder.addNameHintDecoration(
                emptyStruct,
                UnownedStringSlice("_slang_Conditional_empty"));
            emptyStructType = emptyStruct;
        }
        return emptyStructType;
    }

    void addToWorkList(IRInst* inst)
    {
        if (workListSet.contains(inst))
            return;
        workList.add(inst);
        workListSet.add(inst);
    }

    void processConditionalType(IRConditionalType* condType)
    {
        if (loweredConditionalTypes.containsKey(condType))
            return;

        auto valueType = condType->getValueType();
        auto hasValueInst = condType->getHasValue();

        bool hasValue = false;
        bool resolved = false;

        if (auto boolLit = as<IRBoolLit>(hasValueInst))
        {
            hasValue = boolLit->getValue();
            resolved = true;
        }
        else if (auto intLit = as<IRIntLit>(hasValueInst))
        {
            hasValue = getIntVal(intLit) != 0;
            resolved = true;
        }

        if (!resolved)
            return;

        LoweredConditionalTypeInfo info;
        info.hasValue = hasValue;

        if (hasValue)
        {
            // Lower to the underlying value type.
            IRType* resolvedType = valueType;
            while (auto innerCond = as<IRConditionalType>(resolvedType))
            {
                if (auto innerInfo = loweredConditionalTypes.tryGetValue(innerCond))
                    resolvedType = innerInfo->loweredType;
                else
                    break;
            }
            info.loweredType = resolvedType;
        }
        else
        {
            // Lower to a shared empty struct.
            info.loweredType = getEmptyStructType();
        }

        loweredConditionalTypes[condType] = info;
    }

    void processMakeConditionalValue(IRMakeConditionalValue* inst)
    {
        auto condType = as<IRConditionalType>(inst->getDataType());
        if (!condType)
            return;
        auto info = loweredConditionalTypes.tryGetValue(condType);
        if (!info)
            return;

        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        if (info->hasValue)
        {
            inst->replaceUsesWith(inst->getValue());
        }
        else
        {
            auto emptyVal = builder.emitMakeStruct(info->loweredType, 0, nullptr);
            inst->replaceUsesWith(emptyVal);
        }
        inst->removeAndDeallocate();
    }

    void processGetConditionalValue(IRGetConditionalValue* inst)
    {
        auto condType = as<IRConditionalType>(inst->getConditionalOperand()->getDataType());
        if (!condType)
        {
            // Already lowered.
            auto operand = inst->getConditionalOperand();
            IRBuilder builder(module);
            builder.setInsertBefore(inst);
            if (operand->getDataType() == inst->getDataType())
                inst->replaceUsesWith(operand);
            else
                inst->replaceUsesWith(builder.emitPoison(inst->getDataType()));
            inst->removeAndDeallocate();
            return;
        }
        auto info = loweredConditionalTypes.tryGetValue(condType);
        if (!info)
            return;

        IRBuilder builder(module);
        builder.setInsertBefore(inst);

        if (info->hasValue)
        {
            inst->replaceUsesWith(inst->getConditionalOperand());
        }
        else
        {
            auto poisonVal = builder.emitPoison(inst->getDataType());
            inst->replaceUsesWith(poisonVal);
        }
        inst->removeAndDeallocate();
    }

    void processInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_ConditionalType:
            processConditionalType(as<IRConditionalType>(inst));
            break;
        case kIROp_MakeConditionalValue:
            processMakeConditionalValue(as<IRMakeConditionalValue>(inst));
            break;
        case kIROp_GetConditionalValue:
            processGetConditionalValue(as<IRGetConditionalValue>(inst));
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

        // Replace all conditional types with lowered types.
        for (const auto& [key, value] : loweredConditionalTypes)
            key->replaceUsesWith(value.loweredType);
    }
};

void lowerConditionalType(IRModule* module, DiagnosticSink* sink)
{
    ConditionalTypeLoweringContext context(module);
    context.sink = sink;
    context.processModule();
}
} // namespace Slang
