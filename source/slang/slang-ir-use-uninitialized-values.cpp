#include "slang-ir-use-uninitialized-values.h"
#include "slang-ir-insts.h"
#include "slang-ir-reachability.h"
#include "slang-ir.h"

namespace Slang
{
    static bool isMetaOp(IRInst* inst)
    {
        switch (inst->getOp())
        {
        // These instructions only look at the parameter's type,
        // so passing an undefined value to them is permissible
        case kIROp_IsBool:
        case kIROp_IsInt:
        case kIROp_IsUnsignedInt:
        case kIROp_IsSignedInt:
        case kIROp_IsHalf:
        case kIROp_IsFloat:
        case kIROp_IsVector:
        case kIROp_GetNaturalStride:
        case kIROp_TypeEquals:
            return true;
        default:
            break;
        }

        return false;
    }

    static bool isUninitializedValue(IRInst* inst)
    {
        // Also consider var since it does not
        // automatically mean it will be initialized
        // (at least not as the user may have intended)
        return (inst->m_op == kIROp_undefined)
            || (inst->m_op == kIROp_Var);
    }

    static bool isUndefinedParam(IRParam* param)
    {
            auto outType = as<IROutType>(param->getFullType());
            if (!outType)
                return false;

            // Don't check `out Vertices<T>` or `out Indices<T>` parameters
            // in mesh shaders.
            // TODO: we should find a better way to represent these mesh shader
            // parameters so they conform to the initialize before use convention.
            // For example, we can use a `OutputVetices` and `OutputIndices` type
            // to represent an output, like `OutputPatch` in domain shader.
            // For now, we just skip the check for these parameters.
            switch (outType->getValueType()->getOp())
            {
            case kIROp_VerticesType:
            case kIROp_IndicesType:
            case kIROp_PrimitivesType:
                return false;
            default:
                break;
            }

            return true;
    }

    static bool isAliasable(IRInst* inst)
    {
        switch (inst->getOp())
        {
        // These instructions generate (implicit) references to inst
        case kIROp_FieldExtract:
        case kIROp_FieldAddress:
        case kIROp_GetElement:
        case kIROp_GetElementPtr:
            return true;
        default:
            break;
        }

        return false;
    }

    static bool isDifferentiableFunc(IRInst* func)
    {
        for (auto decor = func->getFirstDecoration(); decor; decor = decor->getNextDecoration())
        {
            switch (decor->getOp())
            {
            case kIROp_ForwardDerivativeDecoration:
            case kIROp_ForwardDifferentiableDecoration:
            case kIROp_BackwardDerivativeDecoration:
            case kIROp_BackwardDifferentiableDecoration:
            case kIROp_UserDefinedBackwardDerivativeDecoration:
                return true;
            default:
                break;
            }
        }

        return false;
    }

    static IRInst* resolveSpecialization(IRSpecialize* spec)
    {
        IRInst* base = spec->getBase();
        IRGeneric* generic = as<IRGeneric>(base);
        return findInnerMostGenericReturnVal(generic);
    }

    static bool canIgnoreType(IRType* type)
    {
        // In case specialization returns a function instead
        if (!type)
            return true;

        if (as<IRVoidType>(type))
            return true;

        // For structs, ignore if its empty
        if (auto str = as<IRStructType>(type))
        {
            int count = 0;
            for (auto field : str->getFields())
                count += !canIgnoreType(field->getFieldType());
            return (count == 0);
        }

        // Nothing to initialize for a pure interface
        if (as<IRInterfaceType>(type))
            return true;

        // For pointers, check the value type (primarily for globals)
        if (auto ptr = as<IRPtrType>(type))
            return canIgnoreType(ptr->getValueType());

        // In the case of specializations, check returned type
        if (auto spec = as<IRSpecialize>(type))
        {
            IRInst* inner = resolveSpecialization(spec);
            IRType* innerType = as<IRType>(inner);
            return canIgnoreType(innerType);
        }

        return false;
    }

    static List<IRInst*> getAliasableInstructions(IRInst* inst)
    {
        List<IRInst*> addresses;

        addresses.add(inst);
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            IRInst* user = use->getUser();

            // Meta instructions only use the argument type
            if (isMetaOp(user) || !isAliasable(user))
                continue;

            addresses.addRange(getAliasableInstructions(user));
        }

        return addresses;
    }
    
    static void checkCallUsage(List<IRInst*>& stores, List<IRInst*>& loads, IRCall* call, IRInst* inst)
    {
        IRInst* callee = call->getCallee();

        // Resolve the actual function
        IRFunc* ftn = nullptr;
        IRFuncType* ftype = nullptr;
        if (auto spec = as<IRSpecialize>(callee))
            ftn = as<IRFunc>(resolveSpecialization(spec));
        else if (auto fwd = as<IRForwardDifferentiate>(callee))
            ftn = as<IRFunc>(fwd->getBaseFn());
        else if (auto rev = as<IRBackwardDifferentiate>(callee))
            ftn = as<IRFunc>(rev->getBaseFn());
        else if (auto wit = as<IRLookupWitnessMethod>(callee))
            ftype = as<IRFuncType>(callee->getFullType());
        else
            ftn = as<IRFunc>(callee);

        // Find the argument index so we can fetch the type
        int index = 0;

        auto args = call->getArgsList();
        for (int i = 0; i < args.getCount(); i++)
        {
            if (args[i] == inst)
            {
                index = i;
                break;
            }
        }

        if (ftn)
            ftype = as<IRFuncType>(ftn->getFullType());

        if (!ftype)
            return;

        // Consider it as a store if its passed
        // as an out/inout/ref parameter
        IRType* type = ftype->getParamType(index);
        if (as<IROutType>(type) || as<IRInOutType>(type) || as<IRRefType>(type))
            stores.add(call);
        else
            loads.add(call);
    }

    static void collectLoadStore(List<IRInst*>& stores, List<IRInst*>& loads, IRInst* user, IRInst* inst)
    {
        // Meta intrinsics (which evaluate on type) do nothing
        if (isMetaOp(user))
            return;

        // Ignore instructions generating more aliases
        if (isAliasable(user))
            return;

        switch (user->getOp())
        {
        case kIROp_loop:
        case kIROp_unconditionalBranch:
            // TODO: Ignore branches for now
            return;
        
        case kIROp_Call:
            // Function calls can be either
            // stores or loads depending on
            // whether the callee takes it
            // in as a out parameter or not
            return checkCallUsage(stores, loads, as<IRCall>(user), inst);

        // These instructions will store data...
        case kIROp_Store:
        case kIROp_SwizzledStore:
        case kIROp_SPIRVAsm:
        case kIROp_GenericAsm:
            // For now assume that __intrinsic_asm blocks will do the right thing...
            stores.add(user);
            break;

        case kIROp_SPIRVAsmOperandInst:
            // For SPIRV asm instructions, need to check out the entire
            // block when doing reachability checks
            stores.add(user->getParent());
            break;

        case kIROp_MakeExistential:
        case kIROp_MakeExistentialWithRTTI:
            // For specializing generic structs
            stores.add(user);
            break;
        
        // Miscellaenous cases
        case kIROp_ManagedPtrAttach:
            stores.add(user);
            break;

        // ... and the rest will load/use them
        default:
            loads.add(user);
            break;
        }
    }

    static void cancelLoads(ReachabilityContext &reachability, const List<IRInst*>& stores, List<IRInst*>& loads)
    {
        // Remove all loads which are reachable from stores
        for (auto store : stores)
        {
            for (Index i = 0; i < loads.getCount(); )
            {
                if (reachability.isInstReachable(store, loads[i]))
                    loads.fastRemoveAt(i);
                else
                    i++;
            }
        }
    }

    static List<IRInst*> getUnresolvedParamLoads(ReachabilityContext &reachability, IRFunc* func, IRInst* inst)
    {
        // Collect all aliasable addresses
        auto addresses = getAliasableInstructions(inst);

        // Partition instructions
        List<IRInst*> stores;
        List<IRInst*> loads;

        for (auto alias : addresses)
        {
            // TODO: Mark specific parts assigned to for partial initialization checks
            for (auto use = alias->firstUse; use; use = use->nextUse)
            {
                IRInst* user = use->getUser();
                collectLoadStore(stores, loads, user, alias);
            }
        }

        // Only for out params we shall add all returns
        for (const auto& b : func->getBlocks())
        {
            auto t = as<IRReturn>(b->getTerminator());
            if (!t)
                continue;

            loads.add(t);
        }

        cancelLoads(reachability, stores, loads);

        return loads;
    }

    static List<IRInst*> getUnresolvedVariableLoads(ReachabilityContext &reachability, IRInst* inst)
    {
        auto addresses = getAliasableInstructions(inst);

        // Partition instructions
        List<IRInst*> stores;
        List<IRInst*> loads;

        for (auto alias : addresses)
        {
            for (auto use = alias->firstUse; use; use = use->nextUse)
            {
                IRInst* user = use->getUser();
                collectLoadStore(stores, loads, user, alias);
            }
        }

        cancelLoads(reachability, stores, loads);

        return loads;
    }

    static void checkUninitializedValues(IRFunc* func, DiagnosticSink* sink)
    {
        if (isDifferentiableFunc(func))
            return;

        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        ReachabilityContext reachability(func);

        // Check out parameters
        for (auto param : firstBlock->getParams())
        {
            if (!isUndefinedParam(param))
                continue;

            auto loads = getUnresolvedParamLoads(reachability, func, param);
            for (auto load : loads)
            {
                sink->diagnose(load,
                        as <IRReturn> (load)
                        ? Diagnostics::returningWithUninitializedOut
                        : Diagnostics::usingUninitializedOut,
                        param);
            }
        }

        // Check ordinary instructions
        for (auto inst = firstBlock->getFirstInst(); inst; inst = inst->getNextInst())
        {
            if (!isUninitializedValue(inst))
                continue;

            IRType* type = inst->getFullType();
            if (canIgnoreType(type))
               continue;

            auto loads = getUnresolvedVariableLoads(reachability, inst);
            for (auto load : loads)
            {
                sink->diagnose(load,
                        Diagnostics::usingUninitializedVariable,
                        inst);
            }
        }
    }

    static void checkUninitializedGlobals(IRGlobalVar* variable, DiagnosticSink* sink)
    {
        IRType* type = variable->getFullType();
        if (canIgnoreType(type))
            return;

        // Check for semantic decorations
        // (e.g. globals like gl_GlobalInvocationID)
        if (variable->findDecoration<IRSemanticDecoration>())
            return;

        // Check for initialization blocks
        for (auto inst : variable->getChildren())
        {
            if (as<IRBlock>(inst))
                return;
        }
        
        auto addresses = getAliasableInstructions(variable);
        
        List<IRInst*> stores;
        List<IRInst*> loads;

        for (auto alias : addresses)
        {
            for (auto use = alias->firstUse; use; use = use->nextUse)
            {
                IRInst* user = use->getUser();
                collectLoadStore(stores, loads, user, alias);

                // Disregard if there is at least one store,
                // since we cannot tell what the control flow is
                if (stores.getCount())
                    return;

                // TODO: see if we can do better here (another kind of reachability check?)
            }
        }

        for (auto load : loads)
        {
            sink->diagnose(load,
                Diagnostics::usingUninitializedGlobalVariable,
                variable);
        }
    }

    void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink)
    {
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                checkUninitializedValues(func, sink);
            }
            else if (auto generic = as<IRGeneric>(inst))
            {
                auto retVal = findGenericReturnVal(generic);
                if (auto funcVal = as<IRFunc>(retVal))
                    checkUninitializedValues(funcVal, sink);
            }
            else if (auto global = as<IRGlobalVar>(inst))
            {
                checkUninitializedGlobals(global, sink);
            }
        }
    }
}
