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

    // Casting to IRUndefined is currently vacuous
    // (e.g. any IRInst can be cast to IRUndefined)
    static bool isUndefinedValue(IRInst* inst)
    {
        return (inst->m_op == kIROp_undefined);
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

    static bool canIgnoreType(IRType* type)
    {
        if (as<IRVoidType>(type))
            return true;

        // For structs, ignore if its empty
        if (as<IRStructType>(type))
            return (type->getFirstChild() == nullptr);

        // Nothing to initialize for a pure interface
        if (as<IRInterfaceType>(type))
            return true;

        // For pointers, check the value type (primarily for globals)
        if (auto ptr = as<IRPtrType>(type))
            return canIgnoreType(ptr->getValueType());

        // In the case of specializations, check returned type
        if (auto spec = as<IRSpecialize>(type))
        {
            IRInst* base = spec->getBase();
            IRGeneric* generic = as<IRGeneric>(base);
            IRInst* inner = findInnerMostGenericReturnVal(generic);
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

    static void collectLoadStore(List<IRInst*>& stores, List<IRInst*>& loads, IRInst* user)
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

        // These instructions will store data...
        case kIROp_Store:
        case kIROp_SwizzledStore:
            // TODO: for calls, should make check that the
            // function is passing as an out param
        case kIROp_Call:
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
                collectLoadStore(stores, loads, user);
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
                collectLoadStore(stores, loads, user);
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
            if (!isUndefinedValue(inst))
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
                collectLoadStore(stores, loads, user);

                // Disregard if there is at least one store,
                // since we cannot tell what the control flow is
                if (stores.getCount())
                    return;
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
