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
            value->setFullType((IRType*)addValue(value->getFullType()));
            if (auto newValue = builder->constantMap.TryGetValue(key))
                return *newValue;
            builder->constantMap[key] = value;
            return value;
        }
        IRInst* addTypeValue(IRInst* value)
        {
            // Do not deduplicate struct or interface types.
            switch (value->getOp())
            {
            case kIROp_StructType:
            case kIROp_InterfaceType:
                return value;
            default:
                break;
            }

            for (UInt i = 0; i < value->getOperandCount(); i++)
            {
                value->setOperand(i, addValue(value->getOperand(i)));
            }
            value->setFullType((IRType*)addValue(value->getFullType()));
            IRInstKey key = { value };
            if (auto newValue = builder->globalValueNumberingMap.TryGetValue(key))
                return *newValue;
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
        globalValueNumberingMap.Clear();
        for (auto inst : module->getGlobalInsts())
        {
            if (auto constVal = as<IRConstant>(inst))
            {
                context.addConstantValue(constVal);
            }
        }
        List<IRInst*> instToRemove;
        for (auto inst : module->getGlobalInsts())
        {
            if (as<IRType>(inst))
            {
                auto newInst = context.addTypeValue(inst);
                if (newInst != inst)
                {
                    changed = true;
                    inst->replaceUsesWith(newInst);
                    instToRemove.add(inst);
                }
            }
        }
        for (auto inst : instToRemove)
            inst->removeAndDeallocate();
    }
}
