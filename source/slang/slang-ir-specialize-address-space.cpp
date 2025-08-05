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

    enum class AddressSpaceNodeType
    {
        // Points to the correct address-space
        Direct,

        // Stores an `Inst` the current `Inst` derives its mappings of inst<->addrspace from.
        // This is important for 2 reasons:
        // 1. Sometimes we have to figure out the address-space of an inst 
        //    at a later point in the program (globals)
        // 2. e-calculating address-spaces if we change an
        // address-space other inst's relied on
        // to determine their addrspace.
        // Note: when we fill this struct, we do not store a chain of inst1->inst2->inst3. We
        // compress this list immediatly into inst1->inst3.
        Indirect
    };
    
    struct AddressSpaceNode
    {
        AddressSpaceNodeType type = AddressSpaceNodeType::Direct; 
        AddressSpace addressSpace = AddressSpace::Generic;
        IRInst* indirectAddressSpace = nullptr;
        
        AddressSpaceNode() {}

        AddressSpaceNode(AddressSpace addressSpace)
        { 
            type = AddressSpaceNodeType::Direct;
            this->addressSpace = addressSpace;
        }
        AddressSpaceNode(IRInst* instWithAddressSpace)
        {
            type = AddressSpaceNodeType::Indirect;
            indirectAddressSpace = instWithAddressSpace;
        }
    };

    // map which stores the new mappings of inst<->addrspace
    Dictionary<IRInst*, AddressSpaceNode> mapInstToAddrSpace;
    Dictionary<IRInst*, AddressSpace> mapInstPtrValueTypeToAddress;
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
        auto addrSpaceNode = mapInstToAddrSpace.tryGetValue(inst);
        if (addrSpaceNode)
        {
            if (addrSpaceNode->type == AddressSpaceNodeType::Direct)
                return addrSpaceNode->addressSpace;
            else if (addrSpaceNode->type == AddressSpaceNodeType::Indirect)
                return getAddrSpace(addrSpaceNode->indirectAddressSpace);
        }
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
                    AccessQualifier::ReadWrite,
                    paramAddrSpace);
                param->setFullType(newParamType);
                mapInstToAddrSpace[param] = AddressSpaceNode(paramAddrSpace);
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

    //Dictionary<IRInst*, IRInst*> remapSrcPtrToPtrInsidePtrAddressSpace;
    //// Ptr of ptr require us to check use-site to ensure that the inner pointer type has
    //// a correct address-space assigned given a store-load relationship
    //void maybeCachePtrOfPtrInstToFixup(IRInst* inst)
    //{
    //    switch (inst->getOp())
    //    {
    //    case kIROp_Store:
    //    {
    //        auto store = as<IRStore>(inst);
    //        auto srcType = as<IRPtrTypeBase>(store->getVal()->getDataType());
    //        if (!srcType)
    //            break;
    //        if (!srcType->hasAddressSpace())
    //            break;

    //        // TODO: do we need to re-check all functions referenced by a globalVar?
    //        // TODO: handle ptr<ptr<ptr<...>
    //        // legalize ptr<ptr<T>>. Covers the `data; store(GlobalVar, &data)` case
    //        auto dstType = as<IRPtrTypeBase>(store->getPtr()->getDataType());
    //        if (!dstType)
    //            break;
    //        auto dstPtrTypeInner = as<IRPtrType>(dstType->getValueType());
    //        if (!dstPtrTypeInner)
    //            break;
    //        
    //        // src has same type as dst.
    //        if (srcType->getAddressSpace() == dstPtrTypeInner->getAddressSpace())
    //            break;

    //        // Remap this types address-space
    //        // 
    //        // signal
    //        // & signal that we need to map this inst to a new-addr-space.
    //        // Given that we just changed what we 
    //        remapSrcPtrToPtrInsidePtrAddressSpace;
    //    }
    //    case kIROp_Load:
    //    {
    //        auto load = as<IRLoad>(inst);
    //        if (!mapInstToAddrSpace.containsKey(inst))
    //            break;

    //        // we need to fix up the "what we thought" the correct addr-space was
    //        
    //    }
    //    }
    //}

    // some inst's always need to be updated, specifically,
    // insts that derive their addr-space from another inst.
    bool indirectAddressSpaceInst(IRInst* inst)
    {
        // Ex of cases being handled:
        /*
            cbuffer
            {
                // `data` is a `uniform`
                // The `Load(data)` is a `PhysicalStorageBuffer`
                uint* data;
            }
        */

        switch (inst->getOp())
        {
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
        case kIROp_GetOffsetPtr:
        case kIROp_BitCast:
            if (!mapInstToAddrSpace.containsKey(inst))
            {
                // derives addr-space from base-inst
                mapInstToAddrSpace[inst] = AddressSpaceNode(inst->getOperand(0));
                return true;
            }
            break;
        default:
            break;
        }
        return false;
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
                        //
                        // Do this by adding a `List<> validateInstAddrSpaceAreTheSame`
                        // so that we can validae after legalization fully resolves addr-space
                        continue;
                    }

                    if (indirectAddressSpaceInst(inst))
                    {
                        changed = true;
                        continue;
                    }

                    // If the inst already has a pointer type with explicit address space, then use
                    // it.
                    if (auto ptrType = as<IRPtrTypeBase>(inst->getDataType()))
                    {
                        if (ptrType->hasAddressSpace())
                        {
                             mapInstToAddrSpace[inst] = AddressSpaceNode(ptrType->getAddressSpace());
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
                                mapInstToAddrSpace[inst] = AddressSpaceNode(addrSpace);
                                changed = true;
                            }
                            break;
                        }
                    case kIROp_Load:
                        {
                            // If we are not loading from a ptr, the addr-space is the same as
                            // source. Ex: GlobalParam<vec2> ==> addr-space of
                            // load(GlobalParam<vec2>) is same as parent
                            mapInstToAddrSpace[inst] = AddressSpaceNode(inst->getOperand(0));
                            changed = true;
                        }
                        break;
                    case kIROp_Store:
                        {
                            auto store = as<IRStore>(inst);
                            auto ptr = store->getPtr();

                            // Store gets its addr-space from its ptr or val,
                            // which-ever has a address-space to share.
                            auto dstAddrSpace = getAddrSpace(ptr);
                            if (dstAddrSpace != AddressSpace::Generic)
                            {
                                // ptr has an addr-space.
                                // This has priority since we can store a groupshared into a local.
                                mapInstToAddrSpace[store] = AddressSpaceNode(dstAddrSpace);
                                changed = true;
                            }
                            else
                            {
                                // dst did not have an addr-space. This means we infer from the val.
                                mapInstToAddrSpace[store] = AddressSpaceNode(store->getVal());
                                mapInstToAddrSpace[ptr] = AddressSpaceNode(store);
                                changed = true;
                            }

                            // We now know the correct address-space of the inner pointer
                            // This means we must ensure all Loads have correct address-space.
                            // We need to always run this logic given an `IRStore` since
                            // address-space-specialization may introduce new `IRLoad`s.
                            // 
                            // TODO: do I still need this?
                            //traverseUsers<IRLoad>(
                            //    store->getPtr(),
                            //    [&](IRLoad* load)
                            //    { 
                            //        if (!mapInstToAddrSpace.containsKey(load))
                            //        {
                            //            mapInstToAddrSpace[load] = AddressSpaceNode(store);
                            //            changed = true;
                            //        }
                            //    });
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
                                mapInstToAddrSpace[inst] = AddressSpaceNode(addrSpace);
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
                                    mapInstToAddrSpace[callInst] = AddressSpaceNode(callResultAddrSpace);
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
                                auto ptrResultType = as<IRPtrTypeBase>(funcType->getResultType());
                                if (resultAddrSpace != addrSpace && ptrResultType)
                                {
                                    IRBuilder builder(func);
                                    auto newResultType = builder.getPtrType(
                                        ptrResultType->getOp(),
                                        ptrResultType->getValueType(),
                                        AccessQualifier::ReadWrite,
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
        for (auto [inst, addrSpaceNode] : mapInstToAddrSpace)
        {
            auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
            if (ptrType)
            {
                auto addrSpace = getAddrSpace(inst);
                if (ptrType->getAddressSpace() != addrSpace)
                {
                    IRBuilder builder(inst);
                    auto newType = builder.getPtrType(
                        ptrType->getOp(),
                        ptrType->getValueType(),
                        ptrType->getAccessQualifier(),
                        addrSpace);
                    setDataType(inst, newType);
                }
            }
        }

        for (auto [inst, addrSpace] : mapInstPtrValueTypeToAddress)
        {
            auto instType = as<IRPtrTypeBase>(inst->getDataType());
            auto valueOfPtrType = as<IRPtrTypeBase>(instType->getValueType());
            if (addrSpace == valueOfPtrType->getAddressSpace())
                continue;
            IRBuilder builder(valueOfPtrType);
            auto newValueType = builder.getPtrType(
                valueOfPtrType->getOp(),
                valueOfPtrType->getValueType(),
                valueOfPtrType->getAccessQualifier(),
                addrSpace);
            auto newType = builder.getPtrType(
                instType->getOp(),
                newValueType,
                instType->getAccessQualifier(),
                instType->getAddressSpace());
            setDataType(inst, newType);
        }
    }

    void processModule()
    {
        for (auto globalInst : module->getGlobalInsts())
        {
            auto addrSpace = getLeafInstAddressSpace(globalInst);
            if (addrSpace != AddressSpace::Generic)
            {
                mapInstToAddrSpace[globalInst] = AddressSpaceNode(addrSpace);
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

        for (IRFunc* func : functionsToConsiderRemoving)
        {
            SLANG_ASSERT(!func->findDecoration<IREntryPointDecoration>());
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

} // namespace Slang
