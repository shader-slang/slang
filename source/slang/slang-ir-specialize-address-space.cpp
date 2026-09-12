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
                    // it.
                    if (mapInstToAddrSpace.containsKey(inst))
                    {
                        // TODO: if the inst is a phi node, we need to check if the address space of
                        // the phi arguments is consistent. If not, then we need to report an error.
                        // For now, we just skip the checks.
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
                            auto phiArgs = getPhiArgs(inst);
                            AddressSpace addrSpace = AddressSpace::Generic;
                            for (auto arg : phiArgs)
                            {
                                auto argAddrSpace = getAddrSpace(arg);
                                if (argAddrSpace != AddressSpace::Generic)
                                {
                                    if (addrSpace != AddressSpace::Generic &&
                                        addrSpace != argAddrSpace)
                                    {
                                        // The block parameter merges pointer values from
                                        // different concrete address spaces (e.g. a phi of a
                                        // `StorageBuffer` and a `Device` pointer). A single
                                        // pointer value cannot inhabit two storage classes, so
                                        // diagnose rather than silently taking one arm.
                                        if (sink)
                                            sink->diagnose(
                                                Diagnostics::InconsistentPointerAddressSpace{
                                                    .inst = inst,
                                                    .location = inst->sourceLoc});
                                    }
                                    addrSpace = argAddrSpace;
                                }
                            }
                            if (addrSpace != AddressSpace::Generic)
                            {
                                mapInstToAddrSpace[inst] = addrSpace;
                                changed = true;
                            }
                            break;
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
    // conflict between two concrete address spaces is left unchanged (it is unrepresentable
    // in a single slot and surfaces downstream rather than being silently coerced here).
    void reconcilePointerSlotWithStoredValue(IRInst* slot, List<IRInst*>& reconciledLoads)
    {
        auto slotPtrType = as<IRPtrTypeBase>(slot->getFullType());
        if (!slotPtrType)
            return;
        auto contained = as<IRPtrTypeBase>(slotPtrType->getValueType());
        if (!contained)
            return;

        AddressSpace storedAddrSpace = AddressSpace::Generic;
        bool conflict = false;
        auto consider = [&](IRInst* value)
        {
            auto valueAddrSpace = getPointerValueAddrSpace(value);
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
            // that rejects an otherwise valid program.
            if (sink && slot->getOp() == kIROp_Var)
                sink->diagnose(Diagnostics::InconsistentPointerAddressSpace{
                    .inst = slot,
                    .location = slot->sourceLoc});
            return;
        }
        if (storedAddrSpace == AddressSpace::Generic)
            return;

        auto containedAddrSpace =
            contained->hasAddressSpace() ? contained->getAddressSpace() : AddressSpace::Generic;
        if (containedAddrSpace == storedAddrSpace)
            return;

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
        // stale `Device` type and are otherwise pinned to it.
        for (auto use = slot->firstUse; use; use = use->nextUse)
        {
            if (auto load = as<IRLoad>(use->getUser()))
                if (load->getPtr() == slot)
                {
                    setDataType(load, newContained);
                    reconciledLoads.add(load);
                }
        }
    }

    void reconcilePointerSlots()
    {
        List<IRInst*> reconciledLoads;
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
                        reconcilePointerSlotWithStoredValue(inst, reconciledLoads);
                }
            }
        }
        if (reconciledLoads.getCount())
            propagateAddressSpaceFromInsts(_Move(reconciledLoads));
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
