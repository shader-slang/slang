#include "slang-ir-specialize-address-space.h"

#include "slang-ir-clone.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir.h"

namespace Slang
{
struct AddressSpaceContext : public AddressSpaceSpecializationContext
{
    IRModule* module;

    Dictionary<IRInst*, AddressSpace> mapInstToAddrSpace;
    InitialAddressSpaceAssigner* addrSpaceAssigner;
    DiagnosticSink* sink;
    HashSet<IRFunc*> functionsToConsiderRemoving;
    HashSet<IRDebugValue*> debugValuesToFixup;

    AddressSpaceContext(
        IRModule* inModule,
        InitialAddressSpaceAssigner* inAddrSpaceAssigner,
        DiagnosticSink* sink)
        : module(inModule), addrSpaceAssigner(inAddrSpaceAssigner), sink(sink)
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
                auto newParamType =
                    builder.getPtrType(ptrType->getOp(), ptrType->getValueType(), paramAddrSpace);
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

    bool instHasUnreliablePtrAddressSpace(IRInst* inst)
    {
        // With our current approach, we set address-spaces based on
        // the `Ptr<T,AddressSpace` from `inst->getDataType()`.
        //
        // Due to this process, we have some special cases where
        // we cannot reliably infer the address-space of the `inst`
        // from the `inst->getDataType()` and need to ignore this
        // step of our specialization pass.
        //
        // Following cases should have address-space inferred from
        // operands instead.
        //
        switch (inst->getOp())
        {
        // These ops below are included in this check because of (one or more)
        // of the following cases:
        //
        // 1. These ops are linked to frontend-input. This means we (by-default)
        //     set these pointers to AddressSpace::UserPointer.
        //     This is incorrect since it means these types will have an `AddressSpace`
        //     already (which is possibly incorrect). We should infer AddressSpace based
        //     on source-operand. since these ops are getting address-space from these
        //     operands anyways.
        //
        // 2. These ops may be linked to a local-variable.
        //    If this is the case, we may rewrite the local-variable
        //    address-space. If we re-write the local-variable
        //    address-space we will need to reflect this change
        //    to these ops and fetch the address-space from the
        //    operands instead.
        //
        case kIROp_Add:
        case kIROp_BitCast:
        case kIROp_FieldAddress:
        case kIROp_GetElementPtr:
        case kIROp_GetOffsetPtr:
        case kIROp_Load:
            return true;

        default:
            return false;
        }
    }

    // Return true if the address space of the function return type is changed.
    bool processFunction(IRFunc* func)
    {
        bool retValAddrSpaceChanged = false;
        Dictionary<IRInst*, AddressSpace> mapVarValueToAddrSpace;
        bool changed = true;
        while (changed)
        {
            changed = false;
            for (auto block : func->getBlocks())
            {
                bool isFirstBlock = block == func->getFirstBlock();

                for (auto inst : block->getChildren())
                {
                    if (auto debugValue = as<IRDebugValue>(inst))
                        debugValuesToFixup.add(debugValue);

                    // If we have already assigned an address space to this instruction, then skip
                    // it.
                    if (mapInstToAddrSpace.containsKey(inst))
                    {
                        // TODO: if the inst is a phi node, we need to check if the address space of
                        // the phi arguments is consistent. If not, then we need to report an error.
                        // For now, we just skip the checks.
                        continue;
                    }

                    // If the inst already has a pointer type with explicit address space, then use
                    // it.
                    if (auto ptrType = as<IRPtrTypeBase>(inst->getDataType()))
                    {
                        if (ptrType->hasAddressSpace() && !instHasUnreliablePtrAddressSpace(inst))
                        {
                            mapInstToAddrSpace[inst] = ptrType->getAddressSpace();
                            continue;
                        }
                    }

                    // Otherwise, try to assign an address space based on the instruction type.
                    switch (inst->getOp())
                    {
                    case kIROp_Var:
                    case kIROp_RWStructuredBufferGetElementPtr:
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
                    case kIROp_Load:
                        if (!mapInstToAddrSpace.containsKey(inst))
                        {
                            auto load = as<IRLoad>(inst);
                            // This is a hack to handle `Ptr<Ptr<T>>`
                            auto ptr = load->getPtr();
                            auto pointerType = as<IRPtrTypeBase>(ptr->getDataType());
                            if (pointerType)
                            {
                                auto nestedPointerType =
                                    as<IRPtrTypeBase>(pointerType->getValueType());
                                if (nestedPointerType &&
                                    nestedPointerType->getAddressSpace() != AddressSpace::Generic)
                                {
                                    mapInstToAddrSpace[inst] = nestedPointerType->getAddressSpace();
                                    changed = true;
                                    break;
                                }
                            }

                            // Backup is to get address-space of `inst`.
                            // Technically this should be the only logic,
                            // but Ptr<Ptr<T>> is not fully functional.
                            auto addrSpace = getAddrSpace(inst->getOperand(0));
                            if (addrSpace != AddressSpace::Generic)
                            {
                                mapInstToAddrSpace[inst] = addrSpace;
                                changed = true;
                            }
                        }
                        break;
                    case kIROp_Add:
                    case kIROp_BitCast:
                    case kIROp_GetOffsetPtr:
                    case kIROp_GetElementPtr:
                    case kIROp_FieldAddress:
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
                        {
                            auto store = as<IRStore>(inst);
                            auto val = store->getVal();
                            auto ptr = store->getPtr();
                            auto addrSpaceVal = getAddrSpace(val);
                            if (addrSpaceVal != AddressSpace::Generic)
                            {
                                mapVarValueToAddrSpace[inst->getOperand(0)] = addrSpaceVal;
                                mapInstToAddrSpace[inst] = addrSpaceVal;
                                changed = true;
                            }

                            // If storing into a `Ptr<Ptr<...>>` a Non-UserPointer, warn since Slang
                            // is experiemental with this feature. We do not track nested pointers
                            // currently.
                            auto valPtrType = as<IRPtrTypeBase>(val->getFullType());
                            auto ptrPtrType = as<IRPtrTypeBase>(ptr->getFullType());
                            if (!valPtrType || !ptrPtrType)
                                break;
                            auto ptrPtrValueType = as<IRPtrTypeBase>(ptrPtrType->getValueType());
                            if (!ptrPtrValueType)
                                break;
                            if (addrSpaceVal != AddressSpace::Generic &&
                                addrSpaceVal != AddressSpace::UserPointer)
                            {
                                sink->diagnose(
                                    store,
                                    Diagnostics::assignmentOfNonUserPointerToNestedPointer);
                            }
                        }
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
                                        // TODO: this is an error in user code, because the
                                        // address spaces of the phi arguments don't match.
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
                                bool fullySpecialized = true;
                                for (UInt i = 0; i < callInst->getArgCount(); i++)
                                {
                                    auto arg = callInst->getArg(i);
                                    auto argAddrSpace = getAddrSpace(arg);
                                    argAddrSpaces.add(getAddrSpace(arg));
                                    if (argAddrSpace == AddressSpace::Generic &&
                                        as<IRPtrTypeBase>(arg->getDataType()))
                                    {
                                        fullySpecialized = false;
                                        break;
                                    }
                                }
                                if (!fullySpecialized)
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

                            // only specialize ptr returns
                            if (!as<IRPtrTypeBase>(retVal->getDataType()))
                                continue;

                            auto addrSpace = getAddrSpace(retVal);
                            if (addrSpace != AddressSpace::Generic)
                            {
                                auto funcType = as<IRFuncType>(func->getDataType());
                                AddressSpace resultAddrSpace = getFuncResultAddrSpace(func);
                                if (resultAddrSpace != addrSpace)
                                {
                                    auto ptrResultType =
                                        as<IRPtrTypeBase>(funcType->getResultType());
                                    IRBuilder builder(func);
                                    auto newResultType = builder.getPtrType(
                                        ptrResultType->getOp(),
                                        ptrResultType->getValueType(),
                                        addrSpace);
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
            if (!ptrType)
                continue;
            if (ptrType->getAddressSpace() != addrSpace)
            {
                IRBuilder builder(inst);
                auto newType =
                    builder.getPtrType(ptrType->getOp(), ptrType->getValueType(), addrSpace);
                setDataType(inst, newType);
            }
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

        // Correct DebugVar types
        IRBuilder builder(module);
        for (auto inst : debugValuesToFixup)
        {
            auto debugValue = as<IRDebugValue>(inst);
            auto debugVar = debugValue->getDebugVar();
            auto value = debugValue->getValue();
            auto valuePtrType = as<IRPtrTypeBase>(value->getDataType());
            if (!valuePtrType)
                continue;

            // Ensure the address-space of debug-ptr is correct.
            // Should be Ptr<Ptr<...>>
            builder.setInsertBefore(valuePtrType);
            auto newDebugPtrType = builder.getPtrType(valuePtrType);
            debugVar->setFullType(newDebugPtrType);
        }

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

} // namespace Slang
