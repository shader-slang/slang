#include "slang-ir-use-uninitialized-values.h"
#include "slang-ir-insts.h"
#include "slang-ir-reachability.h"
#include "slang-ir.h"
#include "slang-ir-util.h"

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

    static bool isPotentiallyUnintended(IRParam* param)
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

    // The `upper` field contains the struct that the type is
    // is contained in. It is used to check for empty structs.
    static bool canIgnoreType(IRType* type, IRType* upper)
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
            {
                IRType* ftype = field->getFieldType();
                count += !canIgnoreType(ftype, type);
            }

            return (count == 0);
        }

        // Nothing to initialize for a pure interface
        if (as<IRInterfaceType>(type))
            return true;

        // For pointers, check the value type (primarily for globals)
        if (auto ptr = as<IRPtrType>(type))
        {
            // Avoid the recursive step if its a
            // recursive structure like a linked list
            IRType* ptype = ptr->getValueType();
            return (ptype != upper) && canIgnoreType(ptype, upper);
        }

        // In the case of specializations, check returned type
        if (auto spec = as<IRSpecialize>(type))
        {
            IRInst* inner = resolveSpecialization(spec);
            IRType* innerType = as<IRType>(inner);
            return canIgnoreType(innerType, upper);
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
            ftype = as<IRFuncType>(wit->getFullType());
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

    static void collectSpecialCaseInstructions(List<IRInst*>& stores, IRBlock* block)
    {
        for (auto inst = block->getFirstInst(); inst; inst = inst->next)
        {
            if (as<IRGenericAsm>(inst))
                stores.add(inst);
        }
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

    static bool canStoreReachLoad(ReachabilityContext& reachability, IRInst* store, IRInst* load)
    {
        if (reachability.isInstReachable(store, load))
            return true;

        // Special cases
        
        // Target switches; treat as reachable from any of its cases
        if (auto tswitch = as<IRTargetSwitch>(load))
        {
            IRBlock* upper = getBlock(store);
            for (Slang::UInt i = 0; i < tswitch->getCaseCount(); i++)
            {
                IRBlock* caseBlock = tswitch->getCaseBlock(i);
                if (caseBlock == upper || reachability.isBlockReachable(caseBlock, upper))
                    return true;
            }
        }

        return false;
    }

    static void cancelLoads(ReachabilityContext& reachability, const List<IRInst*>& stores, List<IRInst*>& loads)
    {
        // Remove all loads which are reachable from stores
        for (auto store : stores)
        {
            for (Index i = 0; i < loads.getCount(); )
            {
                if (canStoreReachLoad(reachability, store, loads[i]))
                    loads.fastRemoveAt(i);
                else
                    i++;
            }
        }
    }

    static void collectAliasableLoadStores(IRInst* inst, List<IRInst*>& stores, List<IRInst*>& loads)
    {
        auto addresses = getAliasableInstructions(inst);

        for (auto alias : addresses)
        {
            // TODO: Mark specific parts assigned to for partial initialization checks
            for (auto use = alias->firstUse; use; use = use->nextUse)
            {
                IRInst* user = use->getUser();
                collectLoadStore(stores, loads, user, alias);
            }
        }
    }

    static List<IRInst*> getUnresolvedParamLoads(ReachabilityContext &reachability, IRFunc* func, IRInst* inst)
    {
        // Partition instructions
        List<IRInst*> stores;
        List<IRInst*> loads;

        collectAliasableLoadStores(inst, stores, loads);

        // Special cases for parameters
        for (const auto& b : func->getBlocks())
        {
            collectSpecialCaseInstructions(stores, b);

            auto t = b->getTerminator();
            if (as<IRTargetSwitch>(t) || as<IRReturn>(t))
                loads.add(t);
        }

        cancelLoads(reachability, stores, loads);

        return loads;
    }

    static List<IRInst*> getUnresolvedVariableLoads(ReachabilityContext &reachability, IRInst* inst)
    {
        // Partition instructions
        List<IRInst*> stores;
        List<IRInst*> loads;

        collectAliasableLoadStores(inst, stores, loads);

        cancelLoads(reachability, stores, loads);

        return loads;
    }

    static bool isInstStoredInto(ReachabilityContext& reachability, IRInst* reference, IRInst* inst)
    {
        List<IRInst*> stores;
        List<IRInst*> loads;

        for (auto alias : getAliasableInstructions(inst))
        {
            for (auto use = alias->firstUse; use; use = use->nextUse)
            {
                IRInst* user = use->getUser();
                collectLoadStore(stores, loads, user, alias);
            }
        }

        for (auto store : stores)
        {
            if (reachability.isInstReachable(store, reference))
                return true;
        }

        return false;
    }

    static IRInst* traceInstOrigin(IRInst* inst)
    {
        if (auto load = as<IRLoad>(inst))
            return traceInstOrigin(load->getPtr());

        return inst;
    }

    static bool isReturnedValue(IRInst* inst)
    {
        for (auto use = inst->firstUse; use; use = use->nextUse)
        {
            IRInst* user = use->getUser();
            if (as<IRReturn>(user))
                return true;
        }
        return false;
    }

    static List<IRStructField*> checkFieldsFromExit(ReachabilityContext& reachability, IRReturn* ret, IRStructType* type)
    {
        IRInst* origin = traceInstOrigin(ret->getVal());

        // We don't want to warn on delegated construction
        if (!isUninitializedValue(origin))
            return {};

        // Now we can look for all references to fields
        HashSet<IRStructKey*> usedKeys;
        for (auto use = origin->firstUse; use; use = use->nextUse)
        {
            IRInst* user = use->getUser();
            
            auto fieldAddress = as<IRFieldAddress>(user);
            if (!fieldAddress || !isInstStoredInto(reachability, ret, user))
                continue;

            IRInst* field = fieldAddress->getField();
            usedKeys.add(as<IRStructKey>(field));
        }

        List<IRStructField*> uninitializedFields;

        auto fields = type->getFields();
        for (auto field : fields)
        {
            if (canIgnoreType(field->getFieldType(), nullptr))
                continue;

            if (!usedKeys.contains(field->getKey()))
                uninitializedFields.add(field);
        }
        
        return uninitializedFields;
    }

    static void checkConstructor(IRFunc* func, ReachabilityContext& reachability, DiagnosticSink* sink)
    {
        auto constructor = func->findDecoration<IRConstructorDecorartion>();
        if (!constructor)
            return;

        IRStructType* stype = as<IRStructType>(func->getResultType());
        if (!stype)
            return;

        // Don't bother giving warnings if its not being used
        bool synthesized = constructor->getSynthesizedStatus();
        if (synthesized && !func->firstUse)
            return;
        
        auto printWarnings = [&](const List<IRStructField*>& fields, IRReturn* ret)
        {
            for (auto field : fields)
            {
                if (synthesized)
                {
                    sink->diagnose(field->getKey(),
                        Diagnostics::fieldNotDefaultInitialized,
                        stype,
                        field->getKey());
                }
                else
                {
                    sink->diagnose(ret,
                        Diagnostics::constructorUninitializedField,
                        field->getKey());
                }
            }

        };

        // Work backwards, get exit points and find sources
        for (auto block : func->getBlocks())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->next)
            {
                auto ret = as<IRReturn>(inst);
                if (!ret)
                    continue;

                auto fields = checkFieldsFromExit(reachability, ret, stype);
                printWarnings(fields, ret);
            }
        }
    }

    static void checkUninitializedValues(IRFunc* func, DiagnosticSink* sink)
    {
        // Differentiable functions will generate undefined values
        // strictly so that they can be set in a differentiable way
        if (isDifferentiableFunc(func))
            return;

        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        ReachabilityContext reachability(func);

        // Used for a further analysis and to skip usual return checks
        auto constructor = func->findDecoration <IRConstructorDecorartion> ();

        // Check out parameters
        for (auto param : firstBlock->getParams())
        {
            if (!isPotentiallyUnintended(param))
                continue;

            auto loads = getUnresolvedParamLoads(reachability, func, param);
            for (auto load : loads)
            {
                sink->diagnose(load,
                        as<IRTerminatorInst>(load)
                        ? Diagnostics::returningWithUninitializedOut
                        : Diagnostics::usingUninitializedOut,
                        param);
            }
        }

        // Check ordinary instructions
        for (auto block : func->getBlocks())
        {
            for (auto inst = block->getFirstInst(); inst; inst = inst->getNextInst())
            {
                if (!isUninitializedValue(inst))
                    continue;

                // This will be looked into later
                if (constructor && isReturnedValue(inst))
                    continue;

                IRType* type = inst->getFullType();
                if (canIgnoreType(type, nullptr))
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

        // Separate analysis for constructors
        checkConstructor(func, reachability, sink);
    }

    static void checkUninitializedGlobals(IRGlobalVar* variable, DiagnosticSink* sink)
    {
        IRType* type = variable->getFullType();
        if (canIgnoreType(type, nullptr))
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
