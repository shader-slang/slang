#include "slang-ir-use-uninitialized-out-param.h"
#include "slang-ir-util.h"
#include "slang-ir-reachability.h"

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;

    struct StoreSite
    {
        IRInst* storeInst;
        IRInst* address;
    };

    void checkForUsingUninitializedOutParams(IRFunc* func, DiagnosticSink* sink)
    {
        List<IRInst*> outParams;
        auto firstBlock = func->getFirstBlock();
        if (!firstBlock)
            return;

        ReachabilityContext reachability(func);

        for (auto param : firstBlock->getParams())
        {
            if (auto outType = as<IROutType>(param->getFullType()))
            {
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
                    continue;
                default:
                    break;
                }
            }
            else
            {
                continue;
            }
            List<IRInst*> addresses;
            addresses.add(param);
            List<StoreSite> stores;
            // Collect all sub-addresses from the param.
            for (Index i = 0; i < addresses.getCount(); i++)
            {
                auto addr = addresses[i];
                for (auto use = addr->firstUse; use; use = use->nextUse)
                {
                    switch (use->getUser()->getOp())
                    {
                    case kIROp_GetElementPtr:
                    case kIROp_FieldAddress:
                        addresses.add(use->getUser());
                        break;
                    case kIROp_Store:
                    case kIROp_SwizzledStore:
                        // If we see a store of this address, add it to stores set.
                        if (use == use->getUser()->getOperands())
                            stores.add(StoreSite{ use->getUser(), addr });
                        break;
                    case kIROp_Call:
                    case kIROp_SPIRVAsm:
                        // If we see a call using this address, treat it as a store.
                        stores.add(StoreSite{ use->getUser(), addr });
                        break;
                    case kIROp_SPIRVAsmOperandInst:
                        stores.add(StoreSite{ use->getUser()->getParent(), addr});
                        break;
                    }
                }
            }
            // Check all address loads.
            List<IRInst*> loadsAndReturns;
            for (auto addr : addresses)
            {
                for (auto use = addr->firstUse; use; use = use->nextUse)
                {
                    if (auto load = as<IRLoad>(use->getUser()))
                        loadsAndReturns.add(load);
                }
            }
            for(const auto& b : func->getBlocks())
            {
                auto t = as<IRReturn>(b->getTerminator());
                if (!t) continue;
                loadsAndReturns.add(t);
            }

            for (auto store : stores)
            {
                // Remove insts from `loads` that is reachable from the store.
                for (Index i = 0; i < loadsAndReturns.getCount();)
                {
                    auto load = as<IRLoad>(loadsAndReturns[i]);
                    if (load && !canAddressesPotentiallyAlias(func, store.address, load->getPtr()))
                        continue;
                    if (reachability.isInstReachable(store.storeInst, loadsAndReturns[i]))
                    {
                        loadsAndReturns.fastRemoveAt(i);
                    }
                    else
                    {
                        i++;
                    }
                }
            }
            // If there are any loads left, it means they are using uninitialized out params.
            for (auto load : loadsAndReturns)
            {
                sink->diagnose(
                    load,
                    load->m_op == kIROp_Return
                        ? Diagnostics::returningWithUninitializedOut
                        : Diagnostics::usingUninitializedValue,
                    param);
            }
        }
    }

    void checkForUsingUninitializedOutParams(
        IRModule* module,
        DiagnosticSink* sink)
    {
        for (auto inst : module->getGlobalInsts())
        {
            if (auto func = as<IRFunc>(inst))
            {
                checkForUsingUninitializedOutParams(func, sink);
            }
            else if (auto generic = as<IRGeneric>(inst))
            {
                auto retVal = findGenericReturnVal(generic);
                if (auto funcVal = as<IRFunc>(retVal))
                {
                    checkForUsingUninitializedOutParams(funcVal, sink);
                }
            }
        }
    }
}
