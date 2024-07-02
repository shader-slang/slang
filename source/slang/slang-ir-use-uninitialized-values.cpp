#include "slang-ir-use-uninitialized-values.h"
#include "slang-ir-insts.h"
#include "slang-ir-reachability.h"
#include "slang-ir.h"

namespace Slang
{
    bool metaOp(IRInst* inst) {
        switch (inst->getOp())
        {
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
    bool undefinedValue(IRInst* inst) {
        auto type = inst->getDataType();
        if (!type)
            return false;

        if (type->getOp() == kIROp_VoidType)
            return false;

        // TODO: is this alone sufficient?
        return (inst->m_op == kIROp_undefined);
    }

    bool undefinedOut(IRParam* param) {
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

    bool aliasable(IRInst* inst)
    {
        switch (inst->getOp())
        {
        // These instructions generate (implicit) references to inst
        case kIROp_GetElementPtr:
        case kIROp_FieldExtract:
        case kIROp_FieldAddress:
            return true;
        default:
            break;
        }

        return false;
    }

    bool synthesized(IRFunc* func)
    {
        const UnownedStringSlice slice = toSlice("$__syn_");

        auto decoratorList = func->getDecorations();
        for (auto head = decoratorList.first; head; head = head->next) {
            if (auto name = as<IRNameHintDecoration>(head)) {
                auto str = name->getName();
                auto index = str.indexOf(slice);
                if (index >= 0)
                    return true;
            }
        }

        return false;
    }

    List<IRInst*> concernableUsers(IRInst* inst)
    {
        List<IRInst*> users;
        for (auto use = inst->firstUse; use; use = use->nextUse) {
            IRInst* user = use->getUser();
            // Meta instructions only use the argument type
            if (!metaOp(user))
                users.add(user);
        }

        return users;
    }

    List<IRInst*> aliasableInstructions(IRInst* inst)
    {
        List<IRInst*> addresses;

        auto users = concernableUsers(inst);

        addresses.add(inst);
        for (auto inst : users) {
            if (aliasable(inst)) {
                auto instAddresses = aliasableInstructions(inst);
                addresses.addRange(instAddresses);
            }
        }

        return addresses;
    }

    void collectLoadStore(List<IRInst*>& stores, List<IRInst*>& loads, IRInst* user)
    {
        // Meta intrinsics (which evaluate on type) do nothing
        if (metaOp(user))
            return;

        // Ignore instructions generating more aliases
        if (aliasable(user))
            return;

        switch (user->getOp())
        {
        case kIROp_loop:
            // TODO: Ignore loops for now
            // printf("LOOP: (%p)\n", as<IRLoop>(user));
            // user->dump();
            return;
        // These instructions will store data...
        case kIROp_Store:
        case kIROp_SwizzledStore:
        // TODO: for calls, should make check that the function is passing as an out param
        case kIROp_Call:
        case kIROp_SPIRVAsm:
        // For now assume that __intrinsic_asm blocks will do the right thing...
        case kIROp_GenericAsm:
            stores.add(user);
            break;
        // For SPIRV asm instructions, need to check out the entire
        // block when doing reachability checks
        case kIROp_SPIRVAsmOperandInst:
            stores.add(user->getParent());
            break;
        // ... and the rest will load/use them
        default:
            loads.add(user);
            break;
        }
    }

    void cancelLoads(ReachabilityContext &reachability, const List<IRInst*>& stores, List<IRInst*>& loads)
    {
        // Remove all loads which are reachable from stores
        for (auto store : stores) {
            for (Index i = 0; i < loads.getCount(); ) {
                if (reachability.isInstReachable(store, loads[i]))
                    loads.fastRemoveAt(i);
                else
                    i++;
            }
        }
    }

    List<IRInst*> checkForUsingOutParam(ReachabilityContext &reachability, IRFunc* func, IRInst* inst)
    {
        // Collect all aliasable addresses
        auto addresses = aliasableInstructions(inst);

        // Partition instructions
        List<IRInst*> stores;
        List<IRInst*> loads;

        for (auto alias : addresses) {
            // TODO: Mark specific parts assigned to for partial initialization checks
            for (auto use = alias->firstUse; use; use = use->nextUse) {
                IRInst* user = use->getUser();
                collectLoadStore(stores, loads, user);
            }
        }

        // Only for out params we shall add all returns
        for (const auto& b : func->getBlocks()) {
            auto t = as<IRReturn>(b->getTerminator());
            if (!t)
                continue;

            loads.add(t);
        }

        cancelLoads(reachability, stores, loads);

        return loads;
    }

    List<IRInst*> checkForUsingUndefinedValue(ReachabilityContext &reachability, IRInst* inst)
    {
        auto addresses = aliasableInstructions(inst);

        // Partition instructions
        List<IRInst*> stores;
        List<IRInst*> loads;

        for (auto alias : addresses) {
            for (auto use = alias->firstUse; use; use = use->nextUse) {
                IRInst* user = use->getUser();
                collectLoadStore(stores, loads, user);
            }
        }

        cancelLoads(reachability, stores, loads);

        return loads;
    }

    void checkForUsingUninitializedValues(IRFunc* func, DiagnosticSink* sink)
    {
        // Skip synthesized functions; this includes those generated from autodiff.
        // We shall trust that synthesized functions are aware that they may be using
        // undefined values.
        if (synthesized(func))
            return;

        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        ReachabilityContext reachability(func);

        // Check out parameters
        for (auto param : firstBlock->getParams()) {
            if (!undefinedOut(param))
                continue;

            auto loads = checkForUsingOutParam(reachability, func, param);
            for (auto load : loads) {
                sink->diagnose(load,
                        as <IRReturn> (load)
                        ? Diagnostics::returningWithUninitializedOut
                        : Diagnostics::usingUninitializedValue,
                        param);
            }
        }

        // Check ordinary instructions
        for (auto inst = firstBlock->getFirstInst(); inst; inst = inst->getNextInst()) {
            if (!undefinedValue(inst))
                continue;

            auto loads = checkForUsingUndefinedValue(reachability, inst);
            for (auto load : loads) {
                sink->diagnose(load,
                        as <IRReturn> (load)
                        ? Diagnostics::returningWithUninitializedValue
                        : Diagnostics::usingUninitializedValue,
                        inst);
            }
        }
    }

    void checkForUsingUninitializedValues(IRModule* module, DiagnosticSink* sink)
    {
        for (auto inst : module->getGlobalInsts()) {
            if (auto func = as<IRFunc>(inst)) {
                checkForUsingUninitializedValues(func, sink);
            } else if (auto generic = as<IRGeneric>(inst)) {
                auto retVal = findGenericReturnVal(generic);
                if (auto funcVal = as<IRFunc>(retVal))
                    checkForUsingUninitializedValues(funcVal, sink);
            }
        }
    }
}
