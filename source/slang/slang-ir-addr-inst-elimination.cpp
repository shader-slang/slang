#include "slang-ir-addr-inst-elimination.h"

#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{

// Rewrites address load/store into value extract/updates to allow SSA transform to apply to struct
// and array elements. For example,
//  load(elementPtr(arr, 1)) ==> elementExtract(load(arr), 1)
//  store(fieldAddr(s, field_key), val) ==> store(s, updateField(load(s), fieldKey, val))
// After this transform, all address operands of `load` and `store` insts will be either a var or a
// param.

struct AddressInstEliminationContext
{
    IRModule* module;
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
            auto update = builder.emitUpdateElement(lastVal, accessChain.getArrayView(), val);
            builder.emitStore(lastAddr, update);
        }
        else
        {
            builder.emitStore(lastAddr, val);
        }
    }

    void transformLoadAddr(IRBuilder& builder, IRUse* use)
    {
        auto addr = use->get();
        auto load = as<IRLoad>(use->getUser());

        builder.setInsertBefore(use->getUser());
        auto value = getValue(builder, addr);
        load->replaceUsesWith(value);
        load->removeAndDeallocate();
    }

    void transformStoreAddr(IRBuilder& builder, IRUse* use)
    {
        auto addr = use->get();
        auto store = as<IRStore>(use->getUser());

        builder.setInsertBefore(use->getUser());
        storeValue(builder, addr, store->getVal());
        store->removeAndDeallocate();
    }

    /// The copy of each local variable that a call receives a reference into, keyed by the call
    /// and the variable.
    Dictionary<KeyValuePair<IRInst*, IRInst*>, IRVar*> localCopiesForCall;

    /// Rewrite the reference argument `use` so that, if it names all or part of a local variable,
    /// it names the same part of a copy of that variable made for the call. A reference argument
    /// that names any other storage is left as is.
    void transformReferenceCallAddr(IRBuilder& builder, IRUse* use)
    {
        // We find the storage that the argument refers into by walking out through its element
        // and field addresses, and remember the path so that we can rebuild it on the copy.
        //
        List<IRInst*> accessChain;
        auto root = use->get();
        while (root->getOp() == kIROp_GetElementPtr || root->getOp() == kIROp_FieldAddress)
        {
            accessChain.add(root->getOperand(1));
            root = root->getOperand(0);
        }

        // Storage that other invocations or the caller's own caller may access, such as a
        // `groupshared` variable, is passed through in place, since a copy stored back after the
        // call would lose their accesses during it.
        //
        auto rootVar = as<IRVar>(root);
        if (!rootVar)
            return;
        accessChain.reverse();

        // A local variable is different: nothing outside the call can access it while the call
        // runs, so a single copy of the whole variable, shared by every reference argument of the
        // call that names part of it, behaves exactly like the variable itself. Consider this
        // example:
        //
        //     float sum2(__ref float a, __ref float b) { a += 1; return a + b; }
        //     [Differentiable] float f(float x) { float a = 3; return no_diff(sum2(a, a)) * x; }
        //
        // We copy `a` once for the call to `sum2` and store the copy back after it. Both arguments
        // point into that one copy, so `sum2` sees its write through `a` when it reads `b`.
        //
        // We make the copy because the backward derivative's checkpointing
        // (`findLatestUniqueWriteUse`) assumes that at most one call writes each local variable,
        // and a local passed by reference may be passed to several calls. Each copy is written
        // by only its own call.
        //
        auto call = use->getUser();
        auto key = KeyValuePair<IRInst*, IRInst*>(call, rootVar);
        IRVar* copy = nullptr;
        if (!localCopiesForCall.tryGetValue(key, copy))
        {
            builder.setInsertBefore(call);
            copy = builder.emitVar(rootVar->getDataType()->getValueType());
            builder.emitStore(copy, builder.emitLoad(rootVar));
            builder.setInsertAfter(call);
            builder.emitStore(rootVar, builder.emitLoad(copy));
            localCopiesForCall.add(key, copy);
        }

        builder.setInsertBefore(call);
        use->set(builder.emitElementAddress(copy, accessChain.getArrayView()));
    }

    void transformCallAddr(IRBuilder& builder, IRUse* use)
    {
        auto addr = use->get();
        auto call = as<IRCall>(use->getUser());

        // Don't change the use if addr is a non mutable address.
        if (as<IRBorrowInParamType>(getRootAddr(addr)->getDataType()))
        {
            return;
        }

        auto calleeType = as<IRFuncType>(call->getCallee()->getDataType());
        SLANG_RELEASE_ASSERT(calleeType);
        auto argIndex = UInt(use - call->getArgs());

        // The function type wraps a `groupshared` parameter's reference type in the group-shared
        // rate, so we look inside it.
        //
        auto paramType = unwrapAttributedType(calleeType->getParamType(argIndex));

        // A reference parameter names the argument's storage, so the callee accesses that storage,
        // or a copy that behaves exactly like it, and decides itself how to read, write, and
        // synchronize it. We never copy the argument alone: the callee may reach the same storage
        // through another parameter, and a separate copy for each argument would hide its writes
        // through one from its reads through the other.
        //
        if (as<IRRefParamType>(paramType))
        {
            transformReferenceCallAddr(builder, use);
            return;
        }

        builder.setInsertBefore(call);
        auto tempVar = builder.emitVar(cast<IRPtrTypeBase>(addr->getFullType())->getValueType());

        // Store the initial value of the mutable argument into temp var.
        // If this is an `out` var, the initial value will be undefined,
        // which will get cleaned up later into a `defaultConstruct`.
        builder.emitStore(tempVar, getValue(builder, addr));
        builder.setInsertAfter(call);
        storeValue(builder, addr, builder.emitLoad(tempVar));
        use->set(tempVar);
    }

    SlangResult eliminateAddressInstsImpl(IRFunc* func, DiagnosticSink* inSink)
    {
        sink = inSink;

        IRBuilder builder(func->getModule());

        List<IRInst*> workList;
        for (auto block : func->getBlocks())
        {
            for (auto inst : block->getChildren())
            {
                auto rootType = getRootAddr(inst)->getDataType();

                // A borrow is read-only, so an access through it is a load that needs no
                // write-back and can stay in memory.
                //
                if (as<IRBorrowInParamType>(rootType))
                    continue;

                // A reference names storage that the caller, other aliases, or other invocations
                // may access during the function. Rewriting a write to one element into a write of
                // the whole storage would overwrite their changes to the other elements, so each
                // access through a reference stays an access to just the element it names.
                //
                if (as<IRRefParamType>(rootType))
                    continue;

                if (auto ptrType = as<IRPtrTypeBase>(inst->getDataType()))
                {
                    auto valType = unwrapAttributedType(ptrType->getValueType());
                    if (getResolvedInstForDecorations(valType)
                            ->findDecoration<IRNonCopyableTypeDecoration>())
                        continue;

                    if (ptrType->getAddressSpace() == AddressSpace::UserPointer)
                        continue;

                    workList.add(inst);
                }
            }
        }

        for (Index workListIndex = 0; workListIndex < workList.getCount(); workListIndex++)
        {
            auto addrInst = workList[workListIndex];

            for (auto use = addrInst->firstUse; use;)
            {
                auto nextUse = use->nextUse;

                if (as<IRDecoration>(use->getUser()))
                {
                    use = nextUse;
                    continue;
                }

                IRBuilder transformBuilder(module);
                IRBuilderSourceLocRAII sourceLocationScope(
                    &transformBuilder,
                    use->getUser()->sourceLoc);

                switch (use->getUser()->getOp())
                {
                case kIROp_Load:
                    transformLoadAddr(transformBuilder, use);
                    break;
                case kIROp_Store:
                    transformStoreAddr(transformBuilder, use);
                    break;
                case kIROp_Call:
                    transformCallAddr(transformBuilder, use);
                    break;
                case kIROp_GetElementPtr:
                case kIROp_FieldAddress:
                case kIROp_Unmodified:
                case kIROp_DebugValue:
                case kIROp_GetOffsetPtr:
                    break;
                default:
                    // An atomic operation reads and modifies its memory in place, so its address
                    // operand has no value form to rewrite into, and we leave the use as is. In
                    // any program that compiles, that memory is a device buffer or `groupshared`:
                    // the atomic-destination validation that runs after autodiff
                    // (`validateAtomicOperations`, E41403) rejects every other address, and SSA
                    // promotion applies to neither. (The use is always the atomic's pointer
                    // operand, because `IAtomicable` admits only scalar value types.)
                    if (as<IRAtomicOperation>(use->getUser()))
                        break;
                    sink->diagnose(Diagnostics::UnsupportedUseOfLValueForAutoDiff{
                        .location = use->getUser()->sourceLoc,
                    });
                    break;
                }
                use = nextUse;
            }
        }

        return SLANG_OK;
    }
};

SlangResult eliminateAddressInsts(IRFunc* func, DiagnosticSink* sink)
{
    AddressInstEliminationContext ctx;
    ctx.module = func->getModule();
    ctx.sink = sink;
    return ctx.eliminateAddressInstsImpl(func, sink);
}

} // namespace Slang
