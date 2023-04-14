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

        ReachabilityContext reachability;

        for (auto param : firstBlock->getParams())
        {
            if (!as<IROutType>(param->getFullType()))
                continue;
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
                        // If we see a store of this address, add it to stores set.
                        if (use == use->getUser()->getOperands())
                            stores.add(StoreSite{ use->getUser(), addr });
                        break;
                    case kIROp_Call:
                        // If we see a call using this address, treat it as a store.
                        stores.add(StoreSite{ use->getUser(), addr });
                        break;
                    }
                }
            }
            // Check all address loads.
            List<IRLoad*> loads;
            for (auto addr : addresses)
            {
                for (auto use = addr->firstUse; use; use = use->nextUse)
                {
                    if (auto load = as<IRLoad>(use->getUser()))
                        loads.add(load);
                }
            }

            for (auto store : stores)
            {
                // Remove insts from `loads` that is reachable from the store.
                for (Index i = 0; i < loads.getCount();)
                {
                    auto load = loads[i];
                    if (!canAddressesPotentiallyAlias(func, store.address, loads[i]->getPtr()))
                        continue;
                    if (reachability.isInstReachable(store.storeInst, load))
                    {
                        loads.fastRemoveAt(i);
                    }
                    else
                    {
                        i++;
                    }
                }
            }
            // If there are any loads left, it means they are using uninitialized out params.
            for (auto load : loads)
            {
                sink->diagnose(load, Diagnostics::usingUninitializedValue);
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
