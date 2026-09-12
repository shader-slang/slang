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
    HashSet<IRFunc*> functionsToConsiderRemoving;

    AddressSpaceContext(IRModule* inModule, InitialAddressSpaceAssigner* inAddrSpaceAssigner)
        : module(inModule), addrSpaceAssigner(inAddrSpaceAssigner)
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

    // Maps each specialized clone back to the ultimate original function it was
    // (transitively) specialized from. Used to detect cyclic specialization
    // regardless of how many intermediate clones a recursive call passes
    // through: the clone identities all differ, but their root does not.
    Dictionary<IRFunc*, IRFunc*> specializationRootOf;

    // The specialization roots whose specialization is currently in progress on
    // the processFunction call stack. Guards against unbounded recursion when a
    // recursive function reaches this pass (only possible with
    // -disable-non-essential-validations, which skips the E55201 recursion
    // check).
    HashSet<IRFunc*> rootsBeingSpecialized;

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

        // Record the specialization root so a later cyclic specialization
        // (recursion) can be detected even though every clone has a distinct
        // identity: the clone's root is the original func's root, or the
        // original func itself when it is not a clone.
        IRFunc* root = func;
        if (IRFunc** existingRoot = specializationRootOf.tryGetValue(func))
            root = *existingRoot;
        specializationRootOf[specializedFunc] = root;

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
            // Tracks whether this traversal has already derived the function's result
            // address space from a return, so only the first concrete return (in
            // iteration order) is used — see the Return case.
            bool resultAddrSpaceSetThisPass = false;
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
                                    // Detect cyclic specialization: if specializing `callee` would
                                    // re-enter a specialization root already in progress on this
                                    // stack, the call graph is recursive. That is normally rejected
                                    // by E55201 before this pass, but the check is skipped under
                                    // -disable-non-essential-validations. Cloning would not
                                    // terminate (each clone is a fresh identity, and the recursive
                                    // self-call, direct or with permuted argument address spaces,
                                    // keeps producing new keys), so reuse `callee` to break the
                                    // cycle instead of overflowing the stack.
                                    IRFunc* root = callee;
                                    if (IRFunc** existingRoot =
                                            specializationRootOf.tryGetValue(callee))
                                        root = *existingRoot;
                                    if (rootsBeingSpecialized.contains(root))
                                    {
                                        // Reuse callee, and cache this decision under the current
                                        // key so the worklist's later revisit of the clone resolves
                                        // the same recursive call from the cache. Without caching,
                                        // the revisit would miss the key (the root is no longer on
                                        // the stack) and clone again, forming an unbounded clone
                                        // chain iteratively rather than through stack recursion.
                                        specializedCallee = callee;
                                        functionSpecializations[key] = callee;
                                    }
                                    else
                                    {
                                        specializedCallee = specializeFunc(key);
                                        workList.add(specializedCallee);

                                        // Settle the callee's result address space before it is
                                        // read below: specializeFunc concretizes only parameters,
                                        // and the result is concretized lazily in Return handling,
                                        // so an unsettled callee would record a stale result
                                        // address space that the mapInstToAddrSpace cache then
                                        // makes permanent. The workList.add above still stands: the
                                        // later visit is idempotent because processFunction skips
                                        // insts already in mapInstToAddrSpace. Bracketing the
                                        // recursive descent with rootsBeingSpecialized lets the
                                        // check above catch a cyclic callee instead of recursing
                                        // forever.
                                        rootsBeingSpecialized.add(root);
                                        processFunction(specializedCallee);
                                        rootsBeingSpecialized.remove(root);
                                    }
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
                            // Derive the function's result address space from the first return
                            // (in iteration order) that has a concrete address space, ignoring
                            // later returns this pass. A well-typed pointer-returning function has
                            // one result type, so all of its returns agree and the choice is
                            // unambiguous. Committing to a single deterministic return also stops
                            // the result from oscillating when returns disagree on the address
                            // space: without it, the last return processed would win, so two
                            // conflicting returns would flip the result type on every drain and
                            // requeue the function forever. Conflicting returns are target-invalid
                            // (one result type per function); they arise from code that returns
                            // pointers of different address spaces, and from a recursive function
                            // under -disable-non-essential-validations where E55201 no longer
                            // rejects the recursion that produced them.
                            if (resultAddrSpaceSetThisPass)
                                break;
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
                                resultAddrSpaceSetThisPass = true;
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

        while (workList.getCount())
        {
            // Requeue only the callers discovered this round; the set must reset
            // each iteration or the worklist refills from the whole accumulated
            // set forever and the fixpoint never terminates (#12498).
            HashSet<IRFunc*> newWorkList;
            // Process each function at most once per drain. processFunction is
            // idempotent (it skips insts already in mapInstToAddrSpace), and a
            // caller that must re-observe a callee's settled result is requeued
            // through newWorkList across drains, so this changes nothing for an
            // acyclic call graph. It is what bounds the drain when the graph is
            // cyclic: a recursive call re-adds its callee to workList on every
            // visit (see the Call case), which would otherwise grow the list
            // without bound under -disable-non-essential-validations (E55201
            // normally rejects recursion first).
            HashSet<IRFunc*> processedThisDrain;
            for (Index i = 0; i < workList.getCount(); i++)
            {
                auto func = workList[i];
                if (!processedThisDrain.add(func))
                    continue;
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

        // Remove the original functions that were replaced by specialized
        // clones. Removal must not depend on iteration order: an original
        // callee can still be used by an original caller that is itself pending
        // removal (e.g. `doSomething` calls `foo`, and both are specialized
        // away). A single pass over the unordered set may visit the callee
        // first, see it still used, skip it, then remove the caller — orphaning
        // the callee as a dead, unspecialized function whose parameter keeps a
        // Generic address space that a later emit pass (Metal, WGSL) cannot
        // lower. Iterate to a fixpoint so that removing a caller lets its
        // now-unused callees be reclaimed on a subsequent pass.
        List<IRFunc*> deadCandidates;
        for (auto func : functionsToConsiderRemoving)
            deadCandidates.add(func);
        bool removedAny = true;
        while (removedAny)
        {
            removedAny = false;
            for (Index i = 0; i < deadCandidates.getCount(); i++)
            {
                auto func = deadCandidates[i];
                if (!func)
                    continue;
                SLANG_ASSERT(!func->findDecoration<IREntryPointDecoration>());
                if (!func->hasUses())
                {
                    func->removeAndDeallocate();
                    deadCandidates[i] = nullptr;
                    removedAny = true;
                }
            }
        }
    }
};

void specializeAddressSpace(IRModule* module, InitialAddressSpaceAssigner* addrSpaceAssigner)
{
    AddressSpaceContext context(module, addrSpaceAssigner);
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
