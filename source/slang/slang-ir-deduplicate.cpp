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
            if (auto newValue = builder->getConstantMap().TryGetValue(key))
                return *newValue;
            builder->getConstantMap()[key] = value;
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
            if (auto newValue = builder->getGlobalValueNumberingMap().TryGetValue(key))
                return *newValue;
            builder->getGlobalValueNumberingMap()[key] = value;
            return value;
        }
    };
    void SharedIRBuilder::deduplicateAndRebuildGlobalNumberingMap()
    {
        DeduplicateContext context;
        context.builder = this;
        m_constantMap.Clear();
        m_globalValueNumberingMap.Clear();
        for (auto inst : m_module->getGlobalInsts())
        {
            if (auto constVal = as<IRConstant>(inst))
            {
                context.addConstantValue(constVal);
            }
        }
        List<IRInst*> instToRemove;
        for (auto inst : m_module->getGlobalInsts())
        {
            if (as<IRType>(inst))
            {
                auto newInst = context.addTypeValue(inst);
                if (newInst != inst)
                {
                    inst->replaceUsesWith(newInst);
                    instToRemove.add(inst);
                }
            }
        }
        for (auto inst : instToRemove)
            inst->removeAndDeallocate();
    }

    void SharedIRBuilder::replaceGlobalInst(IRInst* oldInst, IRInst* newInst)
    {
        List<IRUse*> uses;
        for (auto use = oldInst->firstUse; use; use = use->nextUse)
        {
            uses.add(use);
        }

        bool shouldUpdateGlobalNumberedCache = false;
        for (auto use : uses)
        {
            use->set(newInst);
            // depending on the type of the user inst, we may need to rebuild and update the global
            // numbering cache.
            if (isGloballyNumberedInst(use->getUser()))
            {
                shouldUpdateGlobalNumberedCache = true;
            }
        }
        oldInst->removeAndDeallocate();
        if (shouldUpdateGlobalNumberedCache)
        {
            deduplicateAndRebuildGlobalNumberingMap();
        }
    }

    bool SharedIRBuilder::isGloballyNumberedInst(IRInst* inst)
    {
        if (!inst->getParent() || inst->getParent()->getOp() != kIROp_Module)
            return false;
        return m_globalValueNumberingMap.ContainsKey(IRInstKey{inst});
    }
}
