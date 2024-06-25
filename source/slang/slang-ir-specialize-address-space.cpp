#include "slang-ir-specialize-address-space.h"

#include "slang-ir.h"
#include "slang-ir-insts.h"
#include "slang-ir-util.h"
#include "slang-ir-clone.h"

namespace Slang
{
    enum class SpecializeAddressSpaceOptions : int
    {
        None = 0,
        AddEntryPoints = 1 << 0
    };

    struct AddressSpaceContext
    {
        IRModule* module;

        Dictionary<IRInst*, AddressSpace> mapInstToAddrSpace;

        AddressSpaceContext(IRModule* inModule)
            : module(inModule)
        {
        }

        AddressSpace getLeafInstAddressSpace(IRInst* inst)
        {
            if (as<IRGroupSharedRate>(inst->getRate()))
                return AddressSpace::GroupShared;
            switch (inst->getOp())
            {
            case kIROp_RWStructuredBufferGetElementPtr:
                return AddressSpace::Global;
            case kIROp_Var:
                if (as<IRBlock>(inst->getParent()))
                    return AddressSpace::ThreadLocal;
                break;
            default:
                break;
            }
            auto type = unwrapAttributedType(inst->getDataType());
            if (!type)
                return AddressSpace::Generic;
            if (as<IRUniformParameterGroupType>(type))
            {
                return AddressSpace::Uniform;
            }
            if (as<IRByteAddressBufferTypeBase>(type))
            {
                return AddressSpace::Global;
            }
            if (as<IRHLSLStructuredBufferTypeBase>(type))
            {
                return AddressSpace::Global;
            }
            if (as<IRGLSLShaderStorageBufferType>(type))
            {
                return AddressSpace::Global;
            }
            if (auto ptrType = as<IRPtrTypeBase>(type))
            {
                if (ptrType->hasAddressSpace())
                    return (AddressSpace)ptrType->getAddressSpace();
                return AddressSpace::Global;
            }
            return AddressSpace::Generic;
        }

        AddressSpace getAddrSpace(IRInst* inst)
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
                : func(func)
                , argAddrSpaces(argAddrSpaces)
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

            HashCode getHashCode() const
            {
                return hashCode;
            }
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
                    auto newParamType = builder.getPtrType(ptrType->getOp(), ptrType->getValueType(), paramAddrSpace);
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
            auto ptrResultType = as<IRPtrTypeBase>(funcType->getResultType());
            if (!ptrResultType)
                return AddressSpace::Generic;
            AddressSpace resultAddrSpace = AddressSpace::Generic;
            if (ptrResultType->hasAddressSpace())
                resultAddrSpace = (AddressSpace)ptrResultType->getAddressSpace();
            return resultAddrSpace;
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
                        // If we have already assigned an address space to this instruction, then skip it.
                        if (mapInstToAddrSpace.containsKey(inst))
                        {
                            // TODO: if the inst is a phi node, we need to check if the address space of the phi arguments
                            // is consistent. If not, then we need to report an error.
                            // For now, we just skip the checks.
                            continue;
                        }

                        switch (inst->getOp())
                        {
                        case kIROp_Var:
                        {
                            // All local variables should be in the thread-local address space.
                            mapInstToAddrSpace[inst] = AddressSpace::ThreadLocal;
                            changed = true;
                            break;
                        }
                        case kIROp_RWStructuredBufferGetElementPtr:
                        {
                            // The address space of the result of RWStructuredBufferGetElementPtr is always global.
                            mapInstToAddrSpace[inst] = AddressSpace::Global;
                            changed = true;
                            break;
                        }
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
                            auto addrSpace = getAddrSpace(inst->getOperand(1));
                            if (addrSpace != AddressSpace::Generic)
                            {
                                mapVarValueToAddrSpace[inst->getOperand(0)] = addrSpace;
                                changed = true;
                            }
                        }
                        break;
                        case kIROp_Load:
                        {
                            if (auto addrSpace = mapVarValueToAddrSpace.tryGetValue(inst->getOperand(0)))
                            {
                                mapInstToAddrSpace[inst] = *addrSpace;
                                changed = true;
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
                                            // TODO: this is an error in user code, because the address spaces of the
                                            // phi arguments don't match.
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
                                        callInst = as<IRCall>(builder.replaceOperand(callInst->getOperands(), specializedCallee));
                                    }
                                    auto callResultAddrSpace = getFuncResultAddrSpace(specializedCallee);
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
                                    auto ptrResultType = as<IRPtrTypeBase>(funcType->getResultType());
                                    SLANG_ASSERT(ptrResultType);
                                    AddressSpace resultAddrSpace = getFuncResultAddrSpace(func);
                                    if (resultAddrSpace != addrSpace)
                                    {
                                        IRBuilder builder(func);
                                        auto newResultType = builder.getPtrType(ptrResultType->getOp(), ptrResultType->getValueType(), addrSpace);
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
                inst->setFullType(dataType);
            
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
                    IRBuilder builder(inst);
                    auto newType = builder.getPtrType(ptrType->getOp(), ptrType->getValueType(), addrSpace);
                    setDataType(inst, newType);
                }
            }
        }

        template<int options>
        void processModule()
        {
            for (auto globalInst : module->getGlobalInsts())
            {
                auto addrSpace = getLeafInstAddressSpace(globalInst);
                if (addrSpace != AddressSpace::Generic)
                {
                    mapInstToAddrSpace[globalInst] = addrSpace;
                }

                if constexpr (options & (int)SpecializeAddressSpaceOptions::AddEntryPoints)
                {
                    if (auto func = as<IRFunc>(globalInst))
                    {
                        if (func->findDecoration<IREntryPointDecoration>())
                            workList.add(func);
                    }
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
        }
    };

    void specializeAddressSpace(IRModule* module)
    {
        AddressSpaceContext context(module);
        context.processModule<(int)SpecializeAddressSpaceOptions::AddEntryPoints>();
    }
    void specializeAddressSpace(IRModule* module, List<IRFunc*> functionsToSpecialize)
    {
        AddressSpaceContext context(module);
        
        // To specialize address space of a function we must specialize the call sites
        // then the 'functionToSpecialize'
        HashSet<IRFunc*> funcsThatNeedProcessing;
        for (auto& func : functionsToSpecialize)
        {
            for (auto use = func->firstUse; use; use = use->nextUse)
            {
                if (auto callInst = as<IRCall>(use->getUser()))
                {
                    funcsThatNeedProcessing.add(getParentFunc(callInst));
                }
            }
            funcsThatNeedProcessing.add(func);
        }

        context.workList.reserve(funcsThatNeedProcessing.getCount());
        for(auto i : funcsThatNeedProcessing)
            context.workList.add(i);
        
        context.processModule<(int)SpecializeAddressSpaceOptions::None>();
    }
}
