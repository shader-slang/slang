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
    DiagnosticSink* sink;

    Dictionary<IRInst*, AddressSpace> mapInstToAddrSpace;
    InitialAddressSpaceAssigner* addrSpaceAssigner;
    HashSet<IRFunc*> functionsToConsiderRemoving;

    // Functions that return pointers in more than one storage class, mapped to
    // the disagreeing return's location. Recorded as found (once each, across the
    // fixpoint's repeated passes) but reported only after dead-clone removal, so a
    // diagnostic never fires on an original that a specialized clone replaced and
    // this pass then deletes.
    OrderedDictionary<IRFunc*, SourceLoc> conflictingReturns;

    AddressSpaceContext(
        IRModule* inModule,
        InitialAddressSpaceAssigner* inAddrSpaceAssigner,
        DiagnosticSink* inSink)
        : module(inModule), sink(inSink), addrSpaceAssigner(inAddrSpaceAssigner)
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

    // Maps each specialized clone to its ultimate original. Detects cyclic
    // specialization: clone identities differ across recursion levels, the root
    // does not.
    Dictionary<IRFunc*, IRFunc*> specializationRootOf;

    // Specialization roots in progress on the processFunction stack. Guards
    // unbounded recursion when a recursive function reaches this pass (only under
    // -disable-non-essential-validations, which skips the E55201 check).
    HashSet<IRFunc*> rootsBeingSpecialized;

    // Specialization roots ever observed recursing (a call re-entered a root still
    // on the stack). A recursive call's result never settles to a concrete pointer,
    // so held-pointer reconciliation ignores stores fed by such calls rather than
    // treat a provisional default as a conflicting storage class; the recursion
    // itself is invalid SPIR-V and diagnosed elsewhere.
    HashSet<IRFunc*> recursiveRoots;

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

        // Record this clone's root (the original's root, else the original
        // itself) so a later cyclic specialization is detected despite each
        // clone's distinct identity.
        specializationRootOf[specializedFunc] = getSpecializationRoot(func);

        return specializedFunc;
    }

    AddressSpace getFuncResultAddrSpace(IRFunc* callee)
    {
        auto funcType = as<IRFuncType>(callee->getDataType());
        return getAddressSpaceFromVarType(funcType->getResultType());
    }

    // Return true if a value loaded from `var` is directly returned by its
    // function. Used to scope the held-pointer conflict diagnostic (E58005) to a
    // merged pointer that actually reaches a return — the shape #12563 is about and
    // what the E58005 message describes.
    bool anyLoadReachesReturn(IRInst* var)
    {
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            auto load = as<IRLoad>(use->getUser());
            if (!load || load->getPtr() != var)
                continue;
            for (auto loadUse = load->firstUse; loadUse; loadUse = loadUse->nextUse)
                if (loadUse->getUser()->getOp() == kIROp_Return)
                    return true;
        }
        return false;
    }

    // Reconcile the address space of the pointer *held by* a local variable that
    // stores a pointer (a `Ptr(Ptr(T))` such as an `int*` local) from the pointers
    // written into it. The pass otherwise tracks a single address space per inst,
    // which is the variable's own storage class (Function); the *pointee* pointer's
    // class comes only from what is stored. Consider:
    //
    //     int* result;
    //     if (c) result = &gShared;   // Workgroup
    //     else   result = &buf[i];    // StorageBuffer
    //     return result;
    //
    // `result` is a `Ptr(Ptr(int, UserPointer), Function)` whose inner pointer keeps
    // the pre-specialization default (UserPointer) while the stored pointers are
    // Workgroup/StorageBuffer, so the OpStore/OpLoad types disagree. When every
    // stored pointer shares one concrete class, rewrite the variable's inner pointer
    // (and its loads) to that class so the stores and the loaded/returned pointer
    // agree. When two stored pointers disagree, the held pointer would need two
    // classes at once — the same conflict as a function returning two classes — so
    // record it for the E58005 report. Returns whether anything changed.
    bool reconcileHeldPointerAddressSpace(IRFunc* func, IRInst* storePtr)
    {
        auto var = as<IRVar>(storePtr);
        if (!var)
            return false;
        auto outerPtr = as<IRPtrTypeBase>(var->getDataType());
        if (!outerPtr)
            return false;
        auto innerPtr = as<IRPtrTypeBase>(outerPtr->getValueType());
        if (!innerPtr)
            return false;

        AddressSpace held = AddressSpace::Generic;
        bool conflict = false;
        SourceLoc conflictLoc;
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            auto store = as<IRStore>(use->getUser());
            if (!store || store->getPtr() != var)
                continue;
            // Skip a value still settling on the recursion stack: an in-progress
            // recursive call's result is provisional (a pre-settlement default) and
            // would otherwise look like a second, conflicting storage class. Once the
            // callee settles, a later drain rescans this store with its concrete
            // result.
            if (auto call = as<IRCall>(store->getVal()))
                if (auto calleeFunc = as<IRFunc>(call->getCallee()))
                {
                    auto calleeRoot = getSpecializationRoot(calleeFunc);
                    if (rootsBeingSpecialized.contains(calleeRoot) ||
                        recursiveRoots.contains(calleeRoot))
                        continue;
                }
            auto valAddrSpace = getAddrSpace(store->getVal());
            if (valAddrSpace == AddressSpace::Generic)
                continue;
            if (held == AddressSpace::Generic)
            {
                held = valAddrSpace;
            }
            else if (held != valAddrSpace)
            {
                conflict = true;
                conflictLoc = store->sourceLoc;
                break;
            }
        }
        if (conflict)
        {
            // The held pointer would need two storage classes at once. Diagnose it
            // only when the merged pointer is actually returned — a conflicting local
            // that is not returned (e.g. only dereferenced) is a separate,
            // pre-existing gap that would need its own, non-return-worded diagnostic.
            if (sink && anyLoadReachesReturn(var))
                conflictingReturns.addIfNotExists(func, conflictLoc);
            return false;
        }
        if (held == AddressSpace::Generic)
            return false;

        bool changed = false;
        if (innerPtr->getAddressSpace() != held)
        {
            IRBuilder builder(var);
            auto newInner = builder.getPtrType(
                innerPtr->getOp(),
                innerPtr->getValueType(),
                innerPtr->getAccessQualifier(),
                held,
                innerPtr->getDataLayout());
            auto newOuter = builder.getPtrType(
                outerPtr->getOp(),
                newInner,
                outerPtr->getAccessQualifier(),
                outerPtr->getAddressSpace(),
                outerPtr->getDataLayout());
            setDataType(var, newOuter);
            changed = true;
        }
        for (auto use = var->firstUse; use; use = use->nextUse)
        {
            auto load = as<IRLoad>(use->getUser());
            if (!load || load->getPtr() != var)
                continue;
            if (getAddrSpace(load) != held)
            {
                mapInstToAddrSpace[load] = held;
                changed = true;
            }
        }
        return changed;
    }

    // Return the ultimate original `func` was (transitively) specialized from, or
    // `func` itself when it is not a clone. Every specialized copy of one source
    // function shares this root, so it is a stable per-source-function key.
    IRFunc* getSpecializationRoot(IRFunc* func)
    {
        if (IRFunc** root = specializationRootOf.tryGetValue(func))
            return *root;
        return func;
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
                        changed |=
                            reconcileHeldPointerAddressSpace(func, as<IRStore>(inst)->getPtr());
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
                            if (!callee)
                                break;

                            List<AddressSpace> argAddrSpaces;
                            bool hasSpecializableArg = false;
                            for (UInt i = 0; i < callInst->getArgCount(); i++)
                            {
                                auto arg = callInst->getArg(i);
                                auto argAddrSpace = getAddrSpace(arg);
                                argAddrSpaces.add(argAddrSpace);
                                if (argAddrSpace != AddressSpace::Generic)
                                    hasSpecializableArg = true;
                            }

                            // A callee with no pointer arguments (or no body) is not
                            // cloned, but it can still RETURN a pointer, so its result
                            // address space must be reconciled onto the call below either
                            // way. Argument specialization and result reconciliation are
                            // independent concerns; only the former is gated on
                            // hasSpecializableArg.
                            IRFunc* specializedCallee = callee;
                            if (hasSpecializableArg && callee->getFirstBlock())
                            {
                                FuncSpecializationKey key(callee, argAddrSpaces);
                                if (IRFunc** cached = functionSpecializations.tryGetValue(key))
                                {
                                    specializedCallee = *cached;
                                }
                                else
                                {
                                    // Cyclic specialization: if specializing `callee` re-enters a
                                    // root already on this stack, the call graph is recursive
                                    // (normally rejected by E55201, skipped under
                                    // -disable-non-essential-validations). Cloning would not
                                    // terminate (each clone is a fresh identity), so reuse `callee`
                                    // to break the cycle.
                                    IRFunc* root = getSpecializationRoot(callee);
                                    if (rootsBeingSpecialized.contains(root))
                                    {
                                        // Cache the reuse under this key so the worklist's later
                                        // revisit resolves the same call from the cache instead of
                                        // re-cloning (the root has left the stack by then).
                                        specializedCallee = callee;
                                        functionSpecializations[key] = callee;
                                        recursiveRoots.add(root);
                                    }
                                    else
                                    {
                                        specializedCallee = specializeFunc(key);
                                        workList.add(specializedCallee);

                                        // Settle the callee's result address space before reading
                                        // it below: specializeFunc concretizes only parameters, the
                                        // result lazily in Return handling. The workList.add stays
                                        // idempotent (processFunction skips already-mapped insts).
                                        // Bracketing with rootsBeingSpecialized lets the check
                                        // above catch a cyclic callee.
                                        rootsBeingSpecialized.add(root);
                                        processFunction(specializedCallee);
                                        rootsBeingSpecialized.remove(root);
                                    }
                                }
                            }
                            else if (callee->getFirstBlock())
                            {
                                // No argument to specialize on, but the callee may still
                                // return a pointer. Add it to the worklist, and if it
                                // returns a pointer settle its result now (guarded against
                                // recursion via the specialization root) — exactly as the
                                // specialized branch settles a fresh clone. The tail below
                                // records this call's result once and it is then skipped on
                                // later drains (a mapped inst is not revisited), so reading a
                                // pre-settlement default here would never be corrected.
                                workList.add(callee);
                                IRFunc* root = getSpecializationRoot(callee);
                                if (rootsBeingSpecialized.contains(root))
                                {
                                    recursiveRoots.add(root);
                                }
                                else if (getFuncResultAddrSpace(callee) != AddressSpace::Generic)
                                {
                                    rootsBeingSpecialized.add(root);
                                    processFunction(callee);
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
                            // Reconcile the call's result address space to the callee's.
                            // The callee has been settled above (eagerly for a fresh
                            // specialization or a pointer-returning unspecialized callee;
                            // a recursive back-edge reuses an already-cached callee whose
                            // result the base case settles before the recursive return is
                            // read), so this reads a concrete result rather than a default.
                            auto callResultAddrSpace = getFuncResultAddrSpace(specializedCallee);
                            if (callResultAddrSpace != AddressSpace::Generic)
                            {
                                mapInstToAddrSpace[callInst] = callResultAddrSpace;
                                changed = true;
                            }
                        }
                        break;
                    case kIROp_Return:
                        {
                            // Use the first concrete return (in iteration order) as the result
                            // address space and skip the rest. A well-typed function's returns
                            // agree, so the choice is unambiguous; committing to one also keeps
                            // conflicting returns from flipping the result type every drain and
                            // requeuing the function forever.
                            auto retVal = inst->getOperand(0);
                            auto addrSpace = getAddrSpace(retVal);
                            if (resultAddrSpaceSetThisPass)
                            {
                                // A later return in a different concrete storage class means the
                                // function returns pointers in more than one class, which is
                                // invalid (a function has one result type). Record it (once per
                                // function); it is reported from this shared pass after dead-clone
                                // removal, so every target is covered and no diagnostic fires on an
                                // original that a clone replaced and this pass deletes.
                                if (sink && addrSpace != AddressSpace::Generic &&
                                    addrSpace != getFuncResultAddrSpace(func))
                                {
                                    conflictingReturns.addIfNotExists(func, inst->sourceLoc);
                                }
                                break;
                            }
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
            // idempotent, and a caller needing a callee's settled result is
            // requeued via newWorkList, so this is a no-op for an acyclic graph.
            // It bounds the drain for a cyclic one: a recursive call re-adds its
            // callee every visit (see the Call case), which would otherwise grow
            // workList without bound under -disable-non-essential-validations.
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

        // Remove originals replaced by specialized clones. Removal must be
        // order-independent: an original callee may still be used by an original
        // caller that is itself pending removal, so a single pass could skip the
        // callee, remove the caller, and orphan it (a dead unspecialized function
        // whose Generic address space a later Metal/WGSL emit cannot lower).
        // Iterate to a fixpoint.
        List<IRFunc*> deadCandidates;
        for (auto func : functionsToConsiderRemoving)
            deadCandidates.add(func);
        HashSet<IRFunc*> removedFuncs;
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
                    removedFuncs.add(func);
                    func->removeAndDeallocate();
                    deadCandidates[i] = nullptr;
                    removedAny = true;
                }
            }
        }

        // Report conflicting-return-storage-class functions now that dead clones
        // are gone. Skip any removed function (removedFuncs holds freed pointers,
        // compared by identity only, never read). One source function can survive
        // as several specialized copies, so key on the specialization root to
        // report each source function at most once.
        if (sink)
        {
            HashSet<IRFunc*> reportedRoots;
            for (auto& [func, loc] : conflictingReturns)
            {
                if (removedFuncs.contains(func))
                    continue;
                if (reportedRoots.add(getSpecializationRoot(func)))
                    sink->diagnose(
                        Diagnostics::ConflictingReturnPointerStorageClasses{.location = loc});
            }
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
