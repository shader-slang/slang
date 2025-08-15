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

    // TODO: support nested pointers: Ptr<Ptr<int>> = &groupshared_mem[0]
    enum class AddressSpaceNodeType
    {
        // Points to the correct address-space
        Direct,

        // Stores an `Inst` the current `Inst` derives its mappings of inst<->addrspace from.
        // This is important for 2 reasons:
        // 1. Sometimes we have to figure out the address-space of an inst
        //    at a later point in the program (globals)
        // 2. calculating address-spaces if we change an
        //    address-space other inst's relied on
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

    // Map which stores the new mappings of inst<->addrspace
    Dictionary<IRInst*, AddressSpaceNode> mapInstToAddrSpace;
    InitialAddressSpaceAssigner* addrSpaceAssigner;
    DiagnosticSink* sink;
    HashSet<IRFunc*> functionsToConsiderRemoving;
    HashSet<IRDebugValue*> debugValuesToFixup;

    // TODO:
    // Ensure List of address-space chains which must be equal.
    // getAddrSpace(validateAddrSpaceIsTheSame[0][0])` must equal
    // `getAddrSpace(validateAddrSpaceIsTheSame[0][1])` and must equal
    // `getAddrSpace(validateAddrSpaceIsTheSame[0][2]). Primarily needed for: 1.`Phi` nodes
    // 2. If globalParam/globalVar gets specialized, we likely need to store all instances where
    // this happens to ensure consistent addr-space inference of globalParam/globalVar across all
    // sources
    //
    // /
    // List<List<IRInst*>> validateAddrSpaceIsTheSame;

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

    template<GetAddrSpaceOptions options = GetAddrSpaceOptions::None>
    AddressSpace getAddrSpace(IRInst* inst)
    {
        auto addrSpaceNode = mapInstToAddrSpace.tryGetValue(inst);
        if (addrSpaceNode)
        {
            if (addrSpaceNode->type == AddressSpaceNodeType::Direct)
                return addrSpaceNode->addressSpace;
            else if (addrSpaceNode->type == AddressSpaceNodeType::Indirect)
            {
                AddressSpace addrSpace = getAddrSpace<options>(addrSpaceNode->indirectAddressSpace);

                // Solve the Indirect references into a direct reference to save
                // the cost of re-resolving the indirect references.
                if constexpr (options == GetAddrSpaceOptions::CompressChain)
                {
                    mapInstToAddrSpace.set(inst, AddressSpaceNode(addrSpace));
                }

                return addrSpace;
            }
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
                auto newParamType =
                    builder.getPtrType(ptrType->getOp(), ptrType->getValueType(), paramAddrSpace);
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

    // some inst's always need to be updated, specifically,
    // insts that derive their addr-space from another inst.
    bool indirectAddressSpaceInst(IRInst* inst)
    {
        switch (inst->getOp())
        {
        case kIROp_DebugValue:
            {
                // In case address-spaces change we need to later-fixup the debugValues
                debugValuesToFixup.add(as<IRDebugValue>(inst));
                return true;
            }
        case kIROp_Add:
            if (!mapInstToAddrSpace.containsKey(inst))
            {
                // `Add` needs to preserve address space information
                // for the case: `BitCast<FunctionPtr_int>(BitCast<int>(PhysicalPtr_int)+1)`
                // which is code Slang produces during IR legalization passes.
                //
                // Note: this case is only to handle Slang's internal handling of byte-offsets
                // to a physical buffer. We derives addr-space from operand(0).
                //
                // To handle users using bit-cast and addition will require us to associate with
                // integers address-space info lost when bit-casting (and to propegate across
                // additions/math)
                mapInstToAddrSpace[inst] = AddressSpaceNode(inst->getOperand(0));
            }
            return true;
        case kIROp_GetElementPtr:
        case kIROp_FieldAddress:
        case kIROp_GetOffsetPtr:
            if (!mapInstToAddrSpace.containsKey(inst))
            {
                // Derives addr-space from base-inst.
                // We do not assume anything from the
                // inst since it may not have a concrete
                // address-space until this entire legalization
                // pass is over.
                mapInstToAddrSpace[inst] = AddressSpaceNode(inst->getOperand(0));
            }
            return true;
        default:
            return false;
        }
    }

    // Return true if the address space of the function return type is changed.
    bool processFunction(IRFunc* func)
    {
        bool retValAddrSpaceChanged = false;
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
                    // so that we can validate after legalization fully resolves addr-space
                    continue;
                }

                if (indirectAddressSpaceInst(inst))
                {
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
                        }
                        break;
                    }
                case kIROp_BitCast:
                    // Derives addr-space from base-inst.
                    // We do this to try and infer an address-space
                    // if a user (or Slang legalization pass) did not
                    // provide one.
                    mapInstToAddrSpace[inst] = AddressSpaceNode(inst->getOperand(0));
                    break;
                case kIROp_Load:
                    // The addr-space is the same as source.
                    mapInstToAddrSpace[inst] = AddressSpaceNode(inst->getOperand(0));
                    break;
                case kIROp_Store:
                    {
                        auto store = as<IRStore>(inst);
                        auto ptr = store->getPtr();
                        auto val = store->getVal();
                        // Store gets its addr-space from its ptr or val,
                        // which-ever has a address-space to share.
                        auto dstAddrSpace = getAddrSpace(ptr);
                        if (dstAddrSpace != AddressSpace::Generic)
                        {
                            // dst has an addr-space.
                            mapInstToAddrSpace[store] = AddressSpaceNode(ptr);
                        }
                        else
                        {
                            // dst did not have an addr-space. This means we infer from the val.
                            mapInstToAddrSpace[store] = AddressSpaceNode(val);
                            mapInstToAddrSpace[ptr] = AddressSpaceNode(store);
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

                        auto valAddrSpace = getAddrSpace(val);
                        if (valAddrSpace != AddressSpace::Generic &&
                            valAddrSpace != AddressSpace::UserPointer)
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
                                if (addrSpace != AddressSpace::Generic && addrSpace != argAddrSpace)
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
                                argAddrSpaces.add(argAddrSpace);
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
                            if (IRFunc** specializedFunc = functionSpecializations.tryGetValue(key))
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
                            auto callResultAddrSpace = getFuncResultAddrSpace(specializedCallee);
                            if (callResultAddrSpace != AddressSpace::Generic)
                            {
                                mapInstToAddrSpace[callInst] =
                                    AddressSpaceNode(callResultAddrSpace);
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

    // Iteratively compress the list of addr-space while we process each node.
    void applyAddressSpaceToInsts()
    {
        for (auto [inst, addrSpaceNode] : mapInstToAddrSpace)
        {
            auto ptrType = as<IRPtrTypeBase>(inst->getDataType());
            if (!ptrType)
                continue;

            auto addrSpace = getAddrSpace<GetAddrSpaceOptions::CompressChain>(inst);
            if (ptrType->getAddressSpace() == addrSpace)
                continue;

            IRBuilder builder(inst);
            auto newType = builder.getPtrType(ptrType->getOp(), ptrType->getValueType(), addrSpace);
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

        // Apply addr-space according to our addr-space resolution chain
        applyAddressSpaceToInsts();

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
