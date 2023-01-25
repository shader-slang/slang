#include "slang-ir-addr-inst-elimination.h"
#include "slang-ir-insts.h"

namespace Slang
{

// Rewrites address load/store into value extract/updates to allow SSA transform to apply to struct and array elements.
// For example,
//  load(elementPtr(arr, 1)) ==> elementExtract(load(arr), 1)
//  store(fieldAddr(s, field_key), val) ==> store(s, updateField(load(s), fieldKey, val))
// After this transform, all address operands of `load` and `store` insts will be either a var or a param. 

struct AddressInstEliminationContext
{
    SharedIRBuilder* sharedBuilder;
    DiagnosticSink* sink;

    IRInst* getValue(IRBuilder& builder, IRInst* addr)
    {
        switch (addr->getOp())
        {
        default:
            return builder.emitLoad(addr);
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
            {
                IRInst* args[] = {getValue(builder, addr->getOperand(0)), addr->getOperand(1)};
                return builder.emitIntrinsicInst(
                    cast<IRPtrTypeBase>(addr->getFullType())->getValueType(),
                    (addr->getOp() == kIROp_GetElementPtr ? kIROp_GetElement : kIROp_FieldExtract),
                    2,
                    args);
            }
        }
    }

    void storeValue(IRBuilder& builder, IRInst* addr, IRInst* val)
    {
        List<IRInst*> baseAddrs;

        for (auto inst = addr; inst;)
        {
            switch (inst->getOp())
            {
            default:
                baseAddrs.add(inst);
                goto endLoop;
            case kIROp_GetElementPtr:
            case kIROp_FieldAddress:
                baseAddrs.add(inst);
                inst = inst->getOperand(0);
                break;
            }
        }
    endLoop:;
        List<IRInst*> values;
        values.setCount(baseAddrs.getCount());
        if (values.getCount() > 1)
        {
            IRInst* currentVal = builder.emitLoad(baseAddrs.getLast());
            values.getLast() = currentVal;
            for (Index i = baseAddrs.getCount() - 2; i >= 1; i--)
            {
                auto inst = baseAddrs[i];
                switch (inst->getOp())
                {
                default:
                    sink->diagnose(inst->sourceLoc, Diagnostics::unsupportedUseOfLValueForAutoDiff);
                    return;
                case kIROp_GetElementPtr:
                case kIROp_FieldAddress:
                {
                    IRInst* args[] = { currentVal, inst->getOperand(1) };
                    currentVal = builder.emitIntrinsicInst(
                        cast<IRPtrTypeBase>(inst->getFullType())->getValueType(),
                        (inst->getOp() == kIROp_GetElementPtr ? kIROp_GetElement : kIROp_FieldExtract),
                        2,
                        args);
                    values[i] = currentVal;
                }
                break;
                }
            }
        }
        values[0] = val;
        for (Index i = 1; i < values.getCount(); i++)
        {
            auto inst = baseAddrs[i - 1];
            switch (inst->getOp())
            {
            case kIROp_GetElementPtr:
            case kIROp_FieldAddress:
            {
                IRInst* args[] = {values[i], inst->getOperand(1), values[i - 1]};
                values[i] = builder.emitIntrinsicInst(
                    values[i]->getFullType(),
                    (inst->getOp() == kIROp_GetElementPtr ? kIROp_UpdateElement : kIROp_UpdateField),
                    3,
                    args);
            }
            break;
            }
        }
        builder.emitStore(baseAddrs.getLast(), values.getLast());
    }

    void transformLoadAddr(IRUse* use)
    {
        auto addr = use->get();
        auto load = as<IRLoad>(use->getUser());

        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(use->getUser());
        auto value = getValue(builder, addr);
        load->replaceUsesWith(value);
        load->removeAndDeallocate();
    }

    void transformStoreAddr(IRUse* use)
    {
        auto addr = use->get();
        auto store = as<IRStore>(use->getUser());

        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(use->getUser());
        storeValue(builder, addr, store->getVal());
        store->removeAndDeallocate();
    }

    void transformCallAddr(IRUse* use)
    {
        auto addr = use->get();
        auto call = as<IRCall>(use->getUser());

        IRBuilder builder(sharedBuilder);
        builder.setInsertBefore(call);
        auto tempVar = builder.emitVar(cast<IRPtrTypeBase>(addr->getFullType())->getValueType());
        builder.emitStore(tempVar, getValue(builder, addr));
        builder.setInsertAfter(call);
        storeValue(builder, addr, builder.emitLoad(tempVar));
        use->set(tempVar);
    }

    SlangResult eliminateAddressInstsImpl(
        SharedIRBuilder* inSharedBuilder,
        AddressConversionPolicy* policy,
        IRFunc* func,
        DiagnosticSink* inSink)
    {
        sharedBuilder = inSharedBuilder;
        sink = inSink;

        IRBuilder builder(sharedBuilder);

        List<IRInst*> workList;
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                if (as<IRPtrTypeBase>(inst->getDataType()))
                {
                    workList.add(inst);
                }
            }
        }

        for (Index workListIndex = 0; workListIndex < workList.getCount(); workListIndex++)
        {
            auto addrInst = workList[workListIndex];

            if (!policy->shouldConvertAddrInst(addrInst))
                continue;

            for (auto use = addrInst->firstUse; use; )
            {
                if (as<IRDecoration>(use->getUser()))
                    continue;

                auto nextUse = use->nextUse;

                switch (use->getUser()->getOp())
                {
                case kIROp_Load:
                    transformLoadAddr(use);
                    break;
                case kIROp_Store:
                    transformStoreAddr(use);
                    break;
                case kIROp_Call:
                    transformCallAddr(use);
                    break;
                case kIROp_GetElementPtr:
                case kIROp_FieldAddress:
                    break;
                default:
                    sink->diagnose(use->getUser()->sourceLoc, Diagnostics::unsupportedUseOfLValueForAutoDiff);
                    break;
                }
                use = nextUse;
            }
        }

        return SLANG_OK;
    }
};

SlangResult eliminateAddressInsts(
    SharedIRBuilder* sharedBuilder,
    AddressConversionPolicy* policy,
    IRFunc* func,
    DiagnosticSink* sink)
{
    AddressInstEliminationContext ctx;
    return ctx.eliminateAddressInstsImpl(sharedBuilder, policy, func, sink);
}
} // namespace Slang
