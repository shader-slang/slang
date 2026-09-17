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
    DiagnosticSink* sink = nullptr;

    Dictionary<IRInst*, AddressSpace> mapInstToAddrSpace;
    InitialAddressSpaceAssigner* addrSpaceAssigner;
    HashSet<IRFunc*> functionsToConsiderRemoving;

    // Functions that return pointers in more than one storage class, mapped to
    // the disagreeing return's location. Recorded as found (once each, across the
    // fixpoint's repeated passes) but reported only after dead-clone removal, so a
    // diagnostic never fires on an original that a specialized clone replaced and
    // this pass then deletes.
    OrderedDictionary<IRFunc*, SourceLoc> conflictingReturns;

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

    // Join the address spaces flowing into a phi (a non-entry block parameter) and return the
    // joined concrete address space, or `Generic` if no predecessor has been resolved yet.
    // Called both when the phi is first resolved and when it is revisited, so a predecessor
    // resolved on a later fixpoint iteration can still refine the mapping.
    //
    // This does not diagnose a phi that merges two *different* concrete address spaces. On
    // SPIR-V, phis are eliminated before this pass runs and
    // `SPIRVLegalizationContext::processParam` (slang-ir-spirv-legalize.cpp) already reports such a
    // conflict while phis still exist. The GLSL/Metal/WGSL callers now also pass a sink (for the
    // return-conflict diagnostic), but none of them infers a pointer's address space in this pass
    // (Metal/WGSL carry it in the type, GLSL uses the no-op assigner), so a conflicting phi never
    // needs diagnosing here. A conflicting phi therefore just joins to its last concrete arg here.
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
                        changed |=
                            reconcileHeldPointerAddressSpace(func, as<IRStore>(inst)->getPtr());
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
            // reconciliation via the join. As a consequence the pre-pass does not reason about a
            // parameter's address space at all: the cross-function parameter cases — a genuine
            // conflict through a physical parameter, and a specialized pointer parameter's `-g`
            // debug backing variable — are out of scope here and require reconciliation after call
            // specialization (tracked in #13039); until then they can emit ill-typed SPIR-V that
            // is only caught when validation is enabled.
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
        SourceLoc conflictLoc;
        auto consider = [&](IRInst* value, SourceLoc loc)
        {
            auto valueAddrSpace = getStoredValueAddrSpace(value);
            if (valueAddrSpace == AddressSpace::Generic)
                return;
            if (storedAddrSpace == AddressSpace::Generic)
                storedAddrSpace = valueAddrSpace;
            else if (storedAddrSpace != valueAddrSpace)
            {
                conflict = true;
                conflictLoc = loc;
            }
        };
        for (auto use = slot->firstUse; use; use = use->nextUse)
        {
            auto user = use->getUser();
            if (auto store = as<IRStore>(user))
            {
                if (store->getPtr() == slot)
                    consider(store->getVal(), store->sourceLoc);
            }
            else if (auto debugValue = as<IRDebugValue>(user))
            {
                if (debugValue->getDebugVar() == slot)
                    consider(debugValue->getValue(), debugValue->sourceLoc);
            }
        }
        if (conflict)
        {
            // A single slot cannot hold pointers in two concrete address spaces. Only the real
            // variable is diagnosed: a `DebugVar` mirrors it, and a debug slot must never be the
            // thing that rejects an otherwise valid program.
            //
            // When the slot's value reaches a return the conflict is a return conflict owned by
            // E58005 (`conflicting-return-pointer-storage-classes`), which names the disagreeing
            // return and is emitted only after dead-clone removal. We record it into
            // `conflictingReturns` here instead of raising the general inconsistent-slot
            // diagnostic, so exactly one diagnostic covers the slot. Establishing E58005 at the
            // same point we suppress the general one keeps the hand-off self-contained: it does
            // not rely on the later dataflow re-finding the conflict, which would miss a function
            // this pre-pass scans but the entry-point worklist never reaches. A non-returned
            // conflict keeps the general diagnostic. `diagnosedAddrSpaceConflicts` reports each
            // slot once across the fixpoint's repeated visits.
            if (sink && slot->getOp() == kIROp_Var && diagnosedAddrSpaceConflicts.add(slot))
            {
                if (anyLoadReachesReturn(slot))
                {
                    // A slot is enumerated from a function's blocks, so it always has a parent
                    // function; assert that invariant rather than silently dropping the diagnostic.
                    auto func = getParentFunc(slot);
                    SLANG_RELEASE_ASSERT(func);
                    conflictingReturns.addIfNotExists(func, conflictLoc);
                }
                else
                    sink->diagnose(Diagnostics::InconsistentPointerAddressSpace{
                        .inst = slot,
                        .location = slot->sourceLoc});
            }
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
        // This pre-pass — and its `InconsistentPointerAddressSpace` conflict diagnostic —
        // targets the logical-`StorageBuffer`-vs-physical-`Device` slot mismatch that only
        // SPIR-V's split of logical and physical pointers makes ill-typed, so it is gated on the
        // assigner opting in via `shouldReconcileLocalPointerSlots`, not on the presence of a
        // sink: the other targets do not infer a pointer's address space in this pass (Metal/WGSL
        // carry it in the type, GLSL uses the no-op assigner), so retyping slots there would be a
        // silent, undiagnosed change to their address-space handling with no correctness benefit —
        // even though they now supply a sink for the separate return-conflict diagnostic.
        if (addrSpaceAssigner->shouldReconcileLocalPointerSlots())
            reconcilePointerSlots();

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
