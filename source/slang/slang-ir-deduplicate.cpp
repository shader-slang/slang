#include "slang-ir-insts.h"

namespace Slang
{
    struct DeduplicateContext
    {
        SharedIRBuilder* builder;
        IRInst* addValue(IRInst* value)
        {
            if (!value) return nullptr;
            if (as<IRType>(value))
                return addTypeValue(value);
            if (auto constValue = as<IRConstant>(value))
                return addConstantValue(constValue);
            return value;
        }
        IRInst* addConstantValue(IRConstant* value)
        {
            IRConstantKey key = { value };
            if (auto newValue = builder->constantMap.TryGetValue(key))
                return *newValue;
            builder->constantMap[key] = value;
            return value;
        }
        IRInst* addTypeValue(IRInst* value)
        {
            // Do not deduplicate struct types.
            switch (value->op)
            {
            case kIROp_StructType:
                return value;
            default:
                break;
            }

            IRInstKey key = { value };
            if (auto newValue = builder->globalValueNumberingMap.TryGetValue(key))
                return *newValue;

            for (UInt i = 0; i < value->getOperandCount(); i++)
            {
                value->setOperand(i, addValue(value->getOperand(i)));
            }
            value->setFullType((IRType*)addValue(value->getFullType()));
            builder->globalValueNumberingMap[key] = value;
            return value;
        }
    };
    void SharedIRBuilder::deduplicateAndRebuildGlobalNumberingMap()
    {
        DeduplicateContext context;
        context.builder = this;
        bool changed = true;
        constantMap.Clear();
        for (auto inst : module->getGlobalInsts())
        {
            if (auto constVal = as<IRConstant>(inst))
            {
                context.addConstantValue(constVal);
            }
        }
        while (changed)
        {
            globalValueNumberingMap.Clear();
            changed = false;
            for (auto inst : module->getGlobalInsts())
            {
                if (as<IRType>(inst))
                {
                    auto newInst = context.addTypeValue(inst);
                    if (newInst != inst)
                    {
                        changed = true;
                        inst->replaceUsesWith(newInst);
                        inst->removeAndDeallocate();
                        break;
                    }
                }
            }
        }
    }
}
