#include "slang-ir-specialize-address-space.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"
#include "slang-rich-diagnostics.h"

namespace Slang
{
struct AddressSpaceContext : public AddressSpaceSpecializationContext
{
    IRModule* module;

    Dictionary<IRInst*, AddressSpace> mapInstToAddrSpace;
    InitialAddressSpaceAssigner* addrSpaceAssigner;
    HashSet<IRFunc*> functionsToConsiderRemoving;
    DiagnosticSink* sink = nullptr;

    // Reconciled contained address space of each local pointer slot (`Var`/`DebugVar`). A
    // load of one slot stored into another resolves to the source slot's *reconciled* space
    // (see `getStoredValueAddrSpace`), so the slot-reconciliation fixpoint converges
    // regardless of the order slots are visited rather than depending on declaration order.
    Dictionary<IRInst*, AddressSpace> reconciledSlotAddrSpace;

    // Local pointer slots (`Var`) already reported for holding two different concrete address
    // spaces, so the fixpoint diagnoses each exactly once.
    HashSet<IRInst*> diagnosedAddrSpaceConflicts;

    AddressSpaceContext(
        IRModule* inModule,
        InitialAddressSpaceAssigner* inAddrSpaceAssigner,
        DiagnosticSink* inSink)
        : module(inModule), addrSpaceAssigner(inAddrSpaceAssigner), sink(inSink)
    {
    }

    AddressSpace getAddressSpaceFromVarType(IRInst* type)
    {
        return addrSpaceAssigner->getAddressSpaceFromVarType(type);
    }

    AddressSpace getLeafInstAddressSpace(IRInst* inst)
    {
        return addrSpaceAssigner->getLeafInstAddressSpace(inst);
    }

    AddressSpace getAddrSpace(IRInst* inst) override
    {
        auto addrSpace = mapInstToAddrSpace.tryGetValue(inst);
        if (addrSpace)
            return *addrSpace;
        return AddressSpace::Generic;
    }

    List<IRFunc*> workList;

    struct FuncSpecializationKey
    {
    private:
        IRFunc* func;
        List<AddressSpace> argAddrSpaces;
        HashCode hashCode;

    public:
        IRFunc* getFunc() const { return func; }
        ArrayView<AddressSpace> getArgAddrSpaces() const { return argAddrSpaces.getArrayView(); }

        FuncSpecializationKey() = default;

        FuncSpecializationKey(IRFunc* func, List<AddressSpace> argAddrSpaces)
            : func(func), argAddrSpaces(argAddrSpaces)
        {
            Hasher hasher;
            hasher.addHash(Slang::getHashCode(func));
            for (auto addrSpace : argAddrSpaces)
            {
                hasher.addHash((HashCode)addrSpace);
            }
            hashCode = hasher.getResult();
        }

        bool operator==(const FuncSpecializationKey& key) const
        {
            if (func != key.func)
                return false;
            if (argAddrSpaces.getCount() != key.argAddrSpaces.getCount())
                return false;
            for (Index i = 0; i < argAddrSpaces.getCount(); i++)
            {
                if (argAddrSpaces[i] != key.argAddrSpaces[i])
                    return false;
            }
            return true;
        }

        HashCode getHashCode() const { return hashCode; }
    };

    Dictionary<FuncSpecializationKey, IRFunc*> functionSpecializations;

    IRFunc* specializeFunc(const FuncSpecializationKey& key)
    {
        auto func = key.getFunc();
        IRCloneEnv cloneEnv;
        IRBuilder builder(module);

        // First, clone the function body.
        builder.setInsertBefore(func);
        auto specializedFunc = as<IRFunc>(cloneInst(&cloneEnv, &builder, func));

        // Update the parameter types with new address spaces in the specialized function.
        Index paramIndex = 0;
        for (auto param : specializedFunc->getParams())
        {
            auto paramType = param->getFullType();
            auto ptrType = as<IRPtrTypeBase>(paramType);
            if (ptrType)
            {
                auto paramAddrSpace = key.getArgAddrSpaces()[paramIndex];
                auto newParamType = builder.getPtrType(
                    ptrType->getOp(),
                    ptrType->getValueType(),
                    ptrType->getAccessQualifier(),
                    paramAddrSpace,
                    ptrType->getDataLayout());
                param->setFullType(newParamType);
                mapInstToAddrSpace[param] = paramAddrSpace;
            }
            paramIndex++;
        }

        // Update the function type.
        fixUpFuncType(specializedFunc);

        functionSpecializations[key] = specializedFunc;
        return specializedFunc;
    }

    AddressSpace getFuncResultAddrSpace(IRFunc* callee)
    {
        auto funcType = as<IRFuncType>(callee->getDataType());
        return getAddressSpaceFromVarType(funcType->getResultType());
    }

    // Join the address spaces flowing into a phi (a non-entry block parameter) and return the
    // joined concrete address space, or `Generic` if no predecessor has been resolved yet.
    // Called both when the phi is first resolved and when it is revisited, so a predecessor
    // resolved on a later fixpoint iteration can still refine the mapping.
    //
    // This does not diagnose a phi that merges two *different* concrete address spaces:
    // `SPIRVLegalizationContext::processParam` (slang-ir-spirv-legalize.cpp) already reports
    // that, and it runs while phis still exist — by the time this pass runs on the only
    // sink-carrying path (SPIR-V) phis have been eliminated, so a phi conflict never reaches
    // here with a sink. Diagnosing it here would be dead on every path (no other caller passes a
    // sink) and would double-report with `processParam`. A conflicting phi therefore just joins
    // to its last concrete arg here; compilation has already failed via `processParam`.
    AddressSpace resolvePhiAddrSpace(IRInst* param)
    {
        AddressSpace joined = AddressSpace::Generic;
        for (auto arg : getPhiArgs(param))
        {
            auto argAddrSpace = getAddrSpace(arg);
            if (argAddrSpace != AddressSpace::Generic)
                joined = argAddrSpace;
        }
        return joined;
    }

    // Return true if the address space of the function return type is changed.
    bool processFunction(IRFunc* func)
    {
        bool retValAddrSpaceChanged = false;
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (auto block : func->getBlocks())
            {
                bool isFirstBlock = block == func->getFirstBlock();

                for (auto inst : block->getChildren())
                {
                    // If we have already assigned an address space to this instruction, then skip
                    // it -- except for a phi, whose predecessors may resolve on different fixpoint
                    // iterations: re-join its arguments so a later-resolved predecessor can refine
                    // the mapping.
                    if (mapInstToAddrSpace.containsKey(inst))
                    {
                        // A phi may have been mapped from its (possibly stale) declared type
                        // before its predecessors resolved. Re-join its arguments; if they now
                        // agree on a different concrete address space, adopt it and keep the
                        // fixpoint going, so a consistent set of incoming values corrects the
                        // mapping.
                        if (inst->getOp() == kIROp_Param && !isFirstBlock)
                        {
                            auto joined = resolvePhiAddrSpace(inst);
                            if (joined != AddressSpace::Generic &&
                                mapInstToAddrSpace[inst] != joined)
                            {
                                mapInstToAddrSpace[inst] = joined;
                                changed = true;
                            }
                        }
                        continue;
                    }

                    // If the inst already has a pointer/pointer-like type with explicit address
                    // space, then use it.
                    auto addrSpaceFromType =
                        addrSpaceAssigner->getAddressSpaceFromVarType(inst->getDataType());
                    if (addrSpaceFromType != AddressSpace::Generic)
                    {
                        mapInstToAddrSpace[inst] = addrSpaceFromType;
                        changed = true;

                        // Don't return early if the inst itself is a call, as we may still need to
                        // specialize it down below.
                        if (inst->getOp() != kIROp_Call)
                            continue;
                    }

                    // Try to assign an address space based on the instruction type, and specialize
                    // calls.
                    switch (inst->getOp())
                    {
                    case kIROp_Var:
                    case kIROp_RWStructuredBufferGetElementPtr:
                    case kIROp_Load:
                        {
                            // The address space of these insts should be assigned by the initial
                            // address space assigner.
                            AddressSpace addrSpace = AddressSpace::Generic;
                            if (addrSpaceAssigner->tryAssignAddressSpace(inst, addrSpace))
                            {
                                mapInstToAddrSpace[inst] = addrSpace;
                                changed = true;
                            }
                            break;
                        }
                    case kIROp_GetElementPtr:
                    case kIROp_FieldAddress:
                    case kIROp_GetOffsetPtr:
                    case kIROp_BitCast:
                        if (!mapInstToAddrSpace.containsKey(inst))
                        {
                            auto addrSpace = getAddrSpace(inst->getOperand(0));
                            if (addrSpace != AddressSpace::Generic)
                            {
                                mapInstToAddrSpace[inst] = addrSpace;
                                changed = true;
                            }
                        }
                        break;
                    case kIROp_Store:
                        break;
                    case kIROp_Param:
                        if (!isFirstBlock)
                        {
                            auto addrSpace = resolvePhiAddrSpace(inst);
                            if (addrSpace != AddressSpace::Generic)
                            {
                                mapInstToAddrSpace[inst] = addrSpace;
                                changed = true;
                            }
                        }
                        break;
                    case kIROp_Call:
                        {
                            auto callInst = as<IRCall>(inst);
                            auto callee = as<IRFunc>(inst->getOperand(0));
                            if (callee)
                            {
                                List<AddressSpace> argAddrSpaces;
                                bool hasSpecializableArg = false;
                                for (UInt i = 0; i < callInst->getArgCount(); i++)
                                {
                                    auto arg = callInst->getArg(i);
                                    auto addrSpace = getAddrSpace(arg);
                                    argAddrSpaces.add(addrSpace);
                                    if (addrSpace != AddressSpace::Generic)
                                    {
                                        hasSpecializableArg = true;
                                    }
                                }
                                if (!hasSpecializableArg)
                                {
                                    workList.add(callee);
                                    break;
                                }
                                // If callee doesn't have a body, don't specialize.
                                if (!callee->getFirstBlock())
                                    break;
                                FuncSpecializationKey key(callee, argAddrSpaces);
                                IRFunc* specializedCallee = nullptr;
                                if (IRFunc** specializedFunc =
                                        functionSpecializations.tryGetValue(key))
                                {
                                    specializedCallee = *specializedFunc;
                                }
                                else
                                {
                                    specializedCallee = specializeFunc(key);
                                    workList.add(specializedCallee);
                                }
                                IRBuilder builder(callInst);
                                builder.setInsertBefore(callInst);
                                if (specializedCallee != callInst->getCallee())
                                {
                                    callInst = as<IRCall>(builder.replaceOperand(
                                        callInst->getOperands(),
                                        specializedCallee));
                                    // At this point, the original callee may be left without uses.
                                    functionsToConsiderRemoving.add(callee);
                                }
                                auto callResultAddrSpace =
                                    getFuncResultAddrSpace(specializedCallee);
                                if (callResultAddrSpace != AddressSpace::Generic)
                                {
                                    mapInstToAddrSpace[callInst] = callResultAddrSpace;
                                    changed = true;
                                }
                            }
                        }
                        break;
                    case kIROp_Return:
                        {
                            auto retVal = inst->getOperand(0);
                            auto addrSpace = getAddrSpace(retVal);
                            if (addrSpace != AddressSpace::Generic)
                            {
                                auto funcType = as<IRFuncType>(func->getDataType());
                                AddressSpace resultAddrSpace = getFuncResultAddrSpace(func);
                                if (resultAddrSpace != addrSpace)
                                {
                                    auto ptrResultType =
                                        as<IRPtrTypeBase>(funcType->getResultType());
                                    SLANG_ASSERT(ptrResultType);
                                    IRBuilder builder(func);
                                    auto newResultType = builder.getPtrType(
                                        ptrResultType->getOp(),
                                        ptrResultType->getValueType(),
                                        ptrResultType->getAccessQualifier(),
                                        addrSpace,
                                        ptrResultType->getDataLayout());
                                    fixUpFuncType(func, newResultType);
                                    retValAddrSpaceChanged = true;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
        return retValAddrSpaceChanged;
    }

    static void setDataType(IRInst* inst, IRType* dataType)
    {
        auto rate = inst->getRate();
        if (!rate)
        {
            inst->setFullType(dataType);
            return;
        }

        IRBuilder builder(inst);
        builder.setInsertBefore(inst);
        auto newType = builder.getRateQualifiedType(rate, dataType);
        inst->setFullType(newType);
    }

    void applyAddressSpaceToInstType()
    {
        for (auto [inst, addrSpace] : mapInstToAddrSpace)
        {
            auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
            if (ptrType)
            {
                if (ptrType->getAddressSpace() != addrSpace)
                {
                    IRBuilder builder(inst);
                    auto newType = builder.getPtrType(
                        ptrType->getOp(),
                        ptrType->getValueType(),
                        ptrType->getAccessQualifier(),
                        addrSpace,
                        ptrType->getDataLayout());
                    setDataType(inst, newType);
                }
            }
        }
    }

    // Returns the concrete address space of a pointer-typed value once specialization has
    // resolved it, or `Generic` if the value is not a pointer or its address space is still
    // unknown.
    AddressSpace getPointerValueAddrSpace(IRInst* value)
    {
        if (auto mapped = mapInstToAddrSpace.tryGetValue(value))
            if (*mapped != AddressSpace::Generic)
                return *mapped;
        if (auto ptrType = as<IRPtrTypeBase>(value->getDataType()))
            if (ptrType->hasAddressSpace())
                return ptrType->getAddressSpace();
        return AddressSpace::Generic;
    }

    // Return the address space a *stored* pointer value contributes to the slot it is written
    // into. A load of another local pointer slot contributes that source slot's reconciled
    // address space (`Generic` until the source slot has itself been reconciled), rather than
    // the load's stale surface-default pointee -- so `reconcilePointerSlots` reaches the same
    // fixpoint whatever order it visits slots, and never mistakes an unresolved `Ptr<T>`
    // default for a deliberate physical (`Device`) annotation. For any other value (a
    // descriptor-backed element pointer, a physical pointer built by a cast, etc.) the value's
    // own resolved address space is authoritative.
    AddressSpace getStoredValueAddrSpace(IRInst* value)
    {
        if (auto load = as<IRLoad>(value))
        {
            auto ptr = load->getPtr();
            if (ptr->getOp() == kIROp_Var || ptr->getOp() == kIROp_DebugVar)
            {
                if (auto slotAddrSpace = reconciledSlotAddrSpace.tryGetValue(ptr))
                    return *slotAddrSpace;
                return AddressSpace::Generic;
            }
        }
        // An address-computation op (`p + 1`, `&p->field`, `&p[i]`) inherits its base pointer's
        // address space, so resolve through to the base rather than reading the derived inst's
        // own type. The derived inst is not retyped until `propagateAddressSpaceFromInsts` runs
        // *after* this pre-pass, so its own type is still the stale surface default here;
        // recursing lets a slot fed e.g. `s + 1` (where `s` is another slot whose load is not
        // yet retyped) resolve to `s`'s reconciled space once the fixpoint reaches it. A pointer
        // bitcast is deliberately *not* resolved through: it reinterprets its operand, and its
        // declared result type is authoritative (an `(int*)0x1000` physical pointer must stay
        // physical; reinterpreting a logical pointer is unsupported by SPIR-V regardless).
        switch (value->getOp())
        {
        case kIROp_GetOffsetPtr:
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
            return getStoredValueAddrSpace(value->getOperand(0));
        case kIROp_Param:
            // A parameter's contained address space is not yet known at this pre-pass. A *phi*
            // (a non-entry block parameter) is resolved later by the propagation fixpoint; a
            // *function* parameter (an entry-block parameter) is rewritten by `specializeFunc`
            // from the actual argument's address space when its callers are specialized, which
            // runs *after* this pre-pass. Its declared surface pointee (`int*` → `Device`) is
            // therefore only provisional — reading it here would both miss real cases and, worse,
            // wrongly reject valid ones (a helper whose `int*` argument is a descriptor-backed
            // `StorageBuffer` element pointer would be specialized to `StorageBuffer`, so merging
            // it with another such pointer is consistent, not a conflict). So treat any
            // unresolved parameter as unknown; a slot's other, concrete writes still drive its
            // reconciliation via the join. A genuine conflict against a physical parameter is left
            // to surface downstream (see the parameter-reconciliation limitation in #13039).
            if (!mapInstToAddrSpace.containsKey(value))
                return AddressSpace::Generic;
            break;
        }
        return getPointerValueAddrSpace(value);
    }

    // Reconcile a local pointer *slot* — an ordinary `Var` or a debug-only `DebugVar` — with
    // the pointer values written into it.
    //
    // The slot's pointee comes from the surface type of the declaration: `int* p = ...`
    // lowers to a `Ptr<Ptr<T, Device>, Function>` slot, because `Ptr<T>` defaults to
    // `AddressSpace.Device` (== `UserPointer` == SPIR-V `PhysicalStorageBuffer`). But the
    // value written may be a descriptor-backed pointer: e.g. `&buf[i]` / `__getAddress(buf[i])`
    // on an `RWStructuredBuffer` lowers (via `RWStructuredBufferGetElementPtr`) to a logical
    // `StorageBuffer` pointer. The declared `Device` pointee then disagrees with the value's
    // real address space, and there is no valid logical<->physical pointer conversion. When
    // the slot is optimized to SSA the disagreement is invisible (only the value survives),
    // but a slot that must materialize — most commonly a `DebugVar` under `-g` — produces a
    // store whose pointer/value storage classes do not match.
    //
    // We resolve this by specializing the slot to the address space of the pointer written
    // into it, rewriting only the slot's *contained* pointer value-type; the slot's own
    // `Function` storage class is unchanged. Loads then yield the specialized value-type, and
    // the SPIR-V debug backing variable (which is omitted for a `StorageBuffer` pointee) no
    // longer emits a mismatched store. The address spaces of all writes are joined; a genuine
    // conflict between two concrete address spaces is diagnosed rather than silently coerced.
    //
    // Returns true if this call made progress (recorded the slot's address space for the
    // first time, or specialized the slot's contained type), so `reconcilePointerSlots` can
    // iterate to a fixpoint: a slot fed from another slot's load only becomes known once that
    // source slot is reconciled, and slots may be visited in any order.
    bool reconcilePointerSlotWithStoredValue(IRInst* slot, List<IRInst*>& reconciledLoads)
    {
        auto slotPtrType = as<IRPtrTypeBase>(slot->getFullType());
        if (!slotPtrType)
            return false;
        auto contained = as<IRPtrTypeBase>(slotPtrType->getValueType());
        if (!contained)
            return false;

        AddressSpace storedAddrSpace = AddressSpace::Generic;
        bool conflict = false;
        auto consider = [&](IRInst* value)
        {
            auto valueAddrSpace = getStoredValueAddrSpace(value);
            if (valueAddrSpace == AddressSpace::Generic)
                return;
            if (storedAddrSpace == AddressSpace::Generic)
                storedAddrSpace = valueAddrSpace;
            else if (storedAddrSpace != valueAddrSpace)
                conflict = true;
        };
        for (auto use = slot->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (auto store = as<IRStore>(user))
            {
                if (store->getPtr() == slot)
                    consider(store->getVal());
            }
            else if (auto debugValue = as<IRDebugValue>(user))
            {
                if (debugValue->getDebugVar() == slot)
                    consider(debugValue->getValue());
            }
        }
        if (conflict)
        {
            // Two writes give the slot pointer values in different concrete address spaces;
            // a single slot cannot hold both, so diagnose rather than silently picking one.
            // Only the real variable is diagnosed: a `DebugVar` mirrors that same variable, so
            // diagnosing it too would double-report, and a debug slot must never be the thing
            // that rejects an otherwise valid program. The fixpoint may revisit the slot, so
            // report each conflicting slot exactly once.
            if (sink && slot->getOp() == kIROp_Var && diagnosedAddrSpaceConflicts.add(slot))
                sink->diagnose(Diagnostics::InconsistentPointerAddressSpace{
                    .inst = slot,
                    .location = slot->sourceLoc});
            return false;
        }
        if (storedAddrSpace == AddressSpace::Generic)
            return false;

        // Record the slot's resolved address space so a load of it stored into another slot
        // resolves correctly regardless of visitation order; recording or refining it is
        // progress that can unblock a slot fed from this one.
        bool madeProgress = false;
        auto recorded = reconciledSlotAddrSpace.tryGetValue(slot);
        if (!recorded || *recorded != storedAddrSpace)
        {
            reconciledSlotAddrSpace[slot] = storedAddrSpace;
            madeProgress = true;
        }

        auto containedAddrSpace =
            contained->hasAddressSpace() ? contained->getAddressSpace() : AddressSpace::Generic;
        if (containedAddrSpace == storedAddrSpace)
            return madeProgress;

        IRBuilder builder(slot);
        auto newContained = builder.getPtrType(
            contained->getOp(),
            contained->getValueType(),
            contained->getAccessQualifier(),
            storedAddrSpace,
            contained->getDataLayout());
        setDataType(slot, builder.getPtrTypeWithAddressSpace(newContained, slotPtrType));

        // Loads of the slot now yield the specialized pointer value-type. Record them so the
        // new address space is propagated on to their derived pointer users (`GetOffsetPtr`,
        // `GetElementPtr`, `FieldAddress`, phi/block parameters), which were lowered with the
        // stale `Device` type and are otherwise pinned to it. (Re-collecting a load after a
        // later refinement is harmless: propagation just re-applies the now-correct type.)
        for (auto use = slot->firstUse; use; use = use->nextUse)
        {
            if (auto load = as<IRLoad>(use->getUser()))
                if (load->getPtr() == slot)
                {
                    setDataType(load, newContained);
                    reconciledLoads.add(load);
                }
        }
        return true;
    }

    void reconcilePointerSlots()
    {
        // Outer fixpoint: reconciling slots retypes their loads; propagating those loads can
        // retype derived ops / block parameters that feed *other* slots, which are then
        // reconciled on the next round (e.g. a pointer flowing through a block parameter and
        // stored back into a slot). Terminates because a slot's address space only ever moves
        // from `Generic` toward a single concrete value and never back, so each round can only
        // add reconciliations; once a round reconciles nothing, no new address space can flow and
        // the fixpoint has converged.
        for (;;)
        {
            List<IRInst*> reconciledLoads;
            // Inner fixpoint over the slots: a slot fed from another slot's load resolves only
            // after that source slot has, and slots are visited in IR order, so a single pass
            // would be order-dependent.
            bool changed = true;
            while (changed)
            {
                changed = false;
                for (auto globalInst : module->getGlobalInsts())
                {
                    auto func = as<IRFunc>(globalInst);
                    if (!func)
                        continue;
                    for (auto block : func->getBlocks())
                    {
                        for (auto inst : block->getChildren())
                        {
                            if (inst->getOp() == kIROp_Var || inst->getOp() == kIROp_DebugVar)
                                if (reconcilePointerSlotWithStoredValue(inst, reconciledLoads))
                                    changed = true;
                        }
                    }
                }
            }
            // No slot was (re)specialized this round, so no new address spaces can flow to
            // other slots; the fixpoint has converged.
            if (!reconciledLoads.getCount())
                break;
            propagateAddressSpaceFromInsts(_Move(reconciledLoads));
        }
    }

    void processModule()
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            auto addrSpace = getLeafInstAddressSpace(globalInst);
            if (addrSpace != AddressSpace::Generic)
            {
                mapInstToAddrSpace[globalInst] = addrSpace;
            }
            if (auto func = as<IRFunc>(globalInst))
            {
                if (func->findDecoration<IREntryPointDecoration>())
                    workList.add(func);
            }
        }

        // Before running the dataflow, reconcile every local pointer slot's *contained*
        // pointer value-type with the pointer values written into it. The declared pointee
        // comes from the surface type (`int*` defaults to `AddressSpace.Device`), but the
        // value may be a descriptor-backed `StorageBuffer` pointer. Fixing the slot (and its
        // loads) up front lets the dataflow below propagate the specialized address space
        // through derived pointer ops, phi/block parameters, calls, and returns via the
        // existing machinery, rather than leaving stale `Device` types on the users.
        //
        // Gated to the SPIR-V path (the only caller that passes a `sink`). This pre-pass — and
        // its `InconsistentPointerAddressSpace` conflict diagnostic — targets the logical-
        // `StorageBuffer`-vs-physical-`Device` slot mismatch that only SPIR-V's split of logical
        // and physical pointers makes ill-typed. The other callers (Metal/WGSL/GLSL legalization)
        // pass no sink; their targets do not model that split, so retyping slots there would be a
        // silent, undiagnosed change to their address-space handling with no correctness benefit.
        // A caller that wants slot reconciliation (and the conflict diagnostic) must opt in with a
        // sink.
        if (sink)
            reconcilePointerSlots();

        HashSet<IRFunc*> newWorkList;
        while (workList.getCount())
        {
            for (Index i = 0; i < workList.getCount(); i++)
            {
                auto func = workList[i];
                bool resultTypeChanged = processFunction(func);
                if (resultTypeChanged)
                {
                    for (auto use = func->firstUse; use; use = use->nextUse)
                    {
                        if (auto callInst = as<IRCall>(use->getUser()))
                        {
                            newWorkList.add(getParentFunc(callInst));
                        }
                    }
                }
            }
            workList.clear();
            for (auto f : newWorkList)
                workList.add(f);
        }

        applyAddressSpaceToInstType();

        for (IRFunc* func : functionsToConsiderRemoving)
        {
            SLANG_ASSERT(!func->findDecoration<IREntryPointDecoration>());
            if (!func->hasUses())
                func->removeAndDeallocate();
        }
    }
};

void specializeAddressSpace(
    IRModule* module,
    InitialAddressSpaceAssigner* addrSpaceAssigner,
    DiagnosticSink* sink)
{
    AddressSpaceContext context(module, addrSpaceAssigner, sink);
    context.processModule();
}

void propagateAddressSpaceFromInsts(List<IRInst*>&& workList)
{
    HashSet<IRInst*> visited;
    auto addUserToWorkList = [&](IRInst* inst)
    {
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (visited.add(user))
                workList.add(user);
        }
    };
    for (auto item : workList)
    {
        visited.add(item);
    }
    for (Index i = 0; i < workList.getCount(); i++)
    {
        auto inst = workList[i];
        IRBuilder builder(inst);
        auto instPtrType = as<IRPtrTypeBase>(inst->getDataType());
        if (!instPtrType)
            continue;
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            builder.setInsertBefore(user);
            switch (user->getOp())
            {
            case kIROp_Loop:
            case kIROp_UnconditionalBranch:
                {
                    auto branch = as<IRUnconditionalBranch>(user);
                    UIndex phiIndex = (UIndex)(use - branch->getArgs());
                    auto param = getParamAt(branch->getTargetBlock(), phiIndex);
                    if (!param)
                        continue;
                    user = param;
                    break;
                }
            }
            switch (user->getOp())
            {
            case kIROp_FieldAddress:
            case kIROp_GetElementPtr:
            case kIROp_GetOffsetPtr:
            case kIROp_Param:
                {
                    auto valueType = tryGetPointedToType(&builder, user->getDataType());
                    if (!valueType)
                        continue;
                    auto newType = builder.getPtrTypeWithAddressSpace(valueType, instPtrType);
                    if (newType != user->getDataType())
                    {
                        user->setFullType(newType);
                        addUserToWorkList(user);
                    }
                    break;
                }
            }
        }
    }
}

AddressSpace NoOpInitialAddressSpaceAssigner::getAddressSpaceFromVarType(IRInst* type)
{
    if (auto ptrType = as<IRPtrTypeBase>(type))
    {
        if (ptrType->hasAddressSpace())
            return ptrType->getAddressSpace();
    }
    return AddressSpace::Generic;
}

AddressSpace NoOpInitialAddressSpaceAssigner::getLeafInstAddressSpace(IRInst*)
{
    return AddressSpace::Generic;
}

} // namespace Slang
