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
        List<IRInst*> accessChain;

        for (auto inst = addr; inst;)
        {
            switch (inst->getOp())
            {
            default:
                accessChain.add(inst);
                goto endLoop;
            case kIROp_GetElementPtr:
            case kIROp_FieldAddress:
                accessChain.add(inst->getOperand(1));
                inst = inst->getOperand(0);
                break;
            }
        }
    endLoop:;
        auto lastAddr = accessChain.getLast();
        accessChain.removeLast();
        accessChain.reverse();
        if (accessChain.getCount())
        {
            auto lastVal = builder.emitLoad(lastAddr);
            auto update = builder.emitUpdateElement(lastVal, accessChain, val);
            builder.emitStore(lastAddr, update);
        }
        else
        {
            builder.emitStore(lastAddr, val);
        }
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
        auto callee = getResolvedInstForDecorations(call->getCallee());
        auto funcType = as<IRFuncType>(callee->getFullType());
        SLANG_RELEASE_ASSERT(funcType);
        UInt paramIndex = (UInt)(use - call->getOperands() - 1);
        SLANG_RELEASE_ASSERT(call->getArg(paramIndex) == addr);
        if (!as<IROutType>(funcType->getParamType(paramIndex)))
        {
            builder.emitStore(tempVar, getValue(builder, addr));
        }
        else
        {
            builder.emitStore(
                tempVar,
                builder.emitDefaultConstruct(
                    as<IRPtrTypeBase>(tempVar->getDataType())->getValueType()));
        }
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
