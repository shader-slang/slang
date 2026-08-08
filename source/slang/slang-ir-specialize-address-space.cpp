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

    // The exported (library-boundary) originals we seeded a default parameter address space into.
    // Only populated when the target supplies a concrete default (i.e. Metal); empty otherwise, so
    // no other target's DCE behavior is perturbed. These originals must survive dead-code removal
    // even when their only internal call is replaced by a specialized clone, because a library
    // boundary is emitted with its own signature. Specialization *clones* are deliberately
    // excluded: a clone that ends up unused is ordinary dead code and must still be removable.
    HashSet<IRFunc*> seededExportedFuncs;

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

    // Return true if `func` is a library-boundary function — one carrying `export`/`public`
    // linkage — that is emitted with its own signature (as opposed to being inlined or
    // specialized away at its call sites).
    static bool isExportedFunc(IRFunc* func)
    {
        return func->findDecoration<IRHLSLExportDecoration>() != nullptr ||
               func->findDecoration<IRPublicDecoration>() != nullptr;
    }

    // Assign `defaultAddrSpace` to every pointer parameter of an exported function that has not
    // otherwise been given an address space, and return whether any parameter was actually changed.
    // An exported function is a library boundary with no caller to specialize its mutable-reference
    // (`out`/`inout`) parameters from, so a target that needs a concrete address space at emission
    // (e.g. Metal, which renders `thread T*`) supplies one here. Only the parameter's own type is
    // rewritten now; the address space is propagated through the body by the normal worklist run
    // and baked into body instructions later by `applyAddressSpaceToInstType`, so a caller that
    // specializes this function from a concrete argument address space still clones a clean,
    // independent variant. The return value lets the caller seed only functions that genuinely
    // needed a default — a function whose pointer parameters already carry a concrete space (e.g.
    // an earlier pass already specialized it) is left completely untouched.
    bool assignDefaultAddressSpaceToExportedParams(IRFunc* func, AddressSpace defaultAddrSpace)
    {
        IRBuilder builder(module);
        bool changed = false;
        for (auto param : func->getParams())
        {
            if (mapInstToAddrSpace.containsKey(param))
                continue;
            auto ptrType = as<IRPtrTypeBase>(param->getFullType());
            if (!ptrType || ptrType->getAddressSpace() != AddressSpace::Generic)
                continue;
            auto newType = builder.getPtrType(
                ptrType->getOp(),
                ptrType->getValueType(),
                ptrType->getAccessQualifier(),
                defaultAddrSpace,
                ptrType->getDataLayout());
            param->setFullType(newType);
            mapInstToAddrSpace[param] = defaultAddrSpace;
            changed = true;
        }
        if (changed)
            fixUpFuncType(func);
        return changed;
    }

    // Register the seeded exported function as the specialization of itself for the address spaces
    // its own parameters now carry. An exported function is emitted with its own signature, so when
    // another function calls it with arguments whose address spaces already match those parameters
    // (e.g. one exported out-param helper calling another with `thread`-local locals), the
    // call-site specialization in `processFunction` would otherwise clone an identical variant and
    // leave the seeded original as dead, duplicated exported code. Seeding this entry makes such a
    // call reuse the original — keeping one canonical function per exported boundary. A caller
    // passing a *different* address space (e.g. a `device` pointer) still misses this key and
    // specializes a distinct variant, exactly as before.
    void registerExportedFuncAsOwnSpecialization(IRFunc* func)
    {
        List<AddressSpace> paramAddrSpaces;
        for (auto param : func->getParams())
            paramAddrSpaces.add(getAddrSpace(param));
        FuncSpecializationKey key(func, paramAddrSpaces);
        functionSpecializations.addIfNotExists(key, func);
    }

    void processModule()
    {
        auto exportedParamDefaultAddrSpace =
            addrSpaceAssigner->getDefaultAddressSpaceForExportedFunctionParam();

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

        // When the target supplies a concrete default for exported-function parameters, also seed
        // exported (library-boundary) functions. They are emitted with their own signature and are
        // never reached from the entry-point-only worklist above, so without this pass they reach
        // emission unspecialized. Targets whose default is `Generic` (the base behavior) skip this
        // block entirely, leaving their output byte-identical.
        //
        // Two independent things happen per exported function, and they must not be conflated:
        //   (A) Its body is added to the worklist so `processFunction` runs on it. This specializes
        //       the *internal calls* it makes, including calls to non-exported `out`/`inout`
        //       helpers. An exported root with no pointer parameters of its own still needs this:
        //       otherwise the helper it calls is never visited, keeps `AddressSpace::Generic`, and
        //       hits the same emitter crash one call level down.
        //   (B) Its own still-`Generic` pointer parameters are defaulted, and only then is it
        //       registered as its own specialization and protected from the end-of-run
        //       dead-function removal. This is the library-boundary signature itself.
        // (B) is gated on an *actual* assignment. An earlier pass (e.g.
        // `specializeFuncsForBufferLoadArgs`) can leave a fully-specialized clone that inherited
        // the copied `hlslExport` marker but whose pointer parameters already carry a concrete
        // space; registering or DCE-protecting that clone would spawn a redundant, dead duplicate.
        // (A) is safe for such a clone (re-running `processFunction` on an already-specialized body
        // is idempotent — every inst is already in `mapInstToAddrSpace`) and for a dead function
        // (it is removed below regardless), so it applies to every exported function.
        if (exportedParamDefaultAddrSpace != AddressSpace::Generic)
        {
            for (auto globalInst : module->getGlobalInsts())
            {
                auto func = as<IRFunc>(globalInst);
                if (!func || !func->getFirstBlock() || !isExportedFunc(func))
                    continue;
                if (assignDefaultAddressSpaceToExportedParams(func, exportedParamDefaultAddrSpace))
                {
                    registerExportedFuncAsOwnSpecialization(func);
                    seededExportedFuncs.add(func);
                }
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

        for (IRFunc* func : functionsToConsiderRemoving)
        {
            SLANG_ASSERT(!func->findDecoration<IREntryPointDecoration>());
            // Keep only the exported originals we seeded above: a library boundary must be emitted
            // with its own signature even when its only internal call was replaced by a specialized
            // clone. We deliberately do *not* key this off `isExportedFunc`: a dead specialization
            // clone must still be removable, and on non-Metal targets `seededExportedFuncs` is
            // empty so this loop behaves exactly as it did before this change.
            if (seededExportedFuncs.contains(func))
                continue;
            if (!func->hasUses())
                func->removeAndDeallocate();
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

AddressSpace InitialAddressSpaceAssigner::getDefaultAddressSpaceForExportedFunctionParam()
{
    return AddressSpace::Generic;
}

} // namespace Slang
