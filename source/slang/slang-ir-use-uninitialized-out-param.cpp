#include "slang-ir-use-uninitialized-out-param.h"

#include "slang-ir-dataflow.h"
#include "slang-ir-util.h"
#include "slang-ir-reachability.h"

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;


    static void checkForUsingUninitializedOutParam(
        IRFunc* func,
        DiagnosticSink* sink,
        ReachabilityContext& reachability,
        IRParam* param)
    {
        // Our join semilattice to dataflow over
        struct Initialization
        {
            enum V
            {
                Bottom             = 0b00,
                Initialized        = 0b01,
                Uninitialized      = 0b10,
                MaybeUninitialized = 0b11,
            };
            bool join(Initialization other)
            {
                V joined = V(v | other.v);
                if(joined != v)
                {
                    v = joined;
                    return true;
                }
                return false;
            }
            static auto bottom() { return Initialization{Bottom}; }

            V v;
        };

        struct StoreSite
        {
            IRInst* storeInst;
            IRInst* address;
        };

        // Our dataflow context
        struct UninitializedDataFlow : BasicBlockForwardDataFlow
        {
            using Domain = Initialization;
            //
            // The transfer function changes nothing, unless there is a write to
            // the variable, in which case it becomes Initialized
            //
            bool transfer(IRBlock* bb, Initialization& out, const Initialization& in)
            {
                Initialization next = in;
                // No need to check anything if it's already initialized, it can't
                // become uninitialized
                if(in.v != Initialization::Initialized)
                {
                    for( auto i = bb->getFirstInst(); i; i = i->getNextInst() )
                    {
                        for(const auto& s : stores)
                        {
                            if(i == s.storeInst)
                            {
                                next.v = Initialization::Initialized;
                            }
                        }
                    }
                }
                return out.join(next);
            }
            const List<StoreSite>& stores;
        };

        // Collect all sub-addresses from the param.
        // TODO: Currently a write to any of these makes us assume that the
        // whole value is written to, we should be more precise here.
        List<IRInst*> addresses;
        addresses.add(param);
        List<StoreSite> stores;
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
                    // If we see a call using this address, treat it as a store.
                    stores.add(StoreSite{ use->getUser(), addr });
                    break;
                }
            }
        }

        //
        // Get the list of blocks where this value is used, i.e. any loads or
        // returns from its addresses.
        //
        List<IRInst*> loadsAndReturns;
        List<IRBlock*> loadingBlocks;
        for (auto addr : addresses)
        {
            for (auto use = addr->firstUse; use; use = use->nextUse)
            {
                if (auto load = as<IRLoad>(use->getUser()))
                {
                    loadsAndReturns.add(load);
                    IRBlock* bb = as<IRBlock>(load->getParent());
                    if(bb && !loadingBlocks.contains(bb))
                        loadingBlocks.add(bb);
                }
            }
        }
        for(auto bb : func->getBlocks())
        {
            auto t = bb->getTerminator();
            if (t->m_op == kIROp_Return)
            {
                loadsAndReturns.add(t);
                if(!loadingBlocks.contains(bb))
                    loadingBlocks.add(bb);
            }
        }

        //
        // Run the dataflow analysis
        //
        UninitializedDataFlow context{{}, stores};
        auto ios = basicBlockForwardDataFlow(
            context,
            loadingBlocks,
            reachability,
            Initialization{Initialization::Uninitialized});

        // Sort the instructions under investigation so we don't have to walk
        // the whole basic block list for each one
        loadsAndReturns.sort([&](IRInst* x, IRInst* y){
            IRInst* bx = x->getParent();
            IRInst* by = y->getParent();
            return loadingBlocks.indexOf(bx) < loadingBlocks.indexOf(by);
        });
        int bbIndex = 0;

        //
        // Walk over all of our instructions under investigation,
        //
        // If their value was uninitialized when they loaded, produce a
        // diagnostic
        //
        for(auto l : loadsAndReturns)
        {
            IRBlock* bb = as<IRBlock>(l->getParent());
            if(loadingBlocks[bbIndex] != bb)
                bbIndex++;
            SLANG_ASSERT(loadingBlocks[bbIndex] == bb);

            const auto& io = ios[bbIndex];
            SLANG_ASSERT(io.in.v != Initialization::Bottom);
            if(io.in.v == Initialization::Initialized)
                continue;

            auto diag = [&](){
                sink->diagnose(
                    l,
                    l->m_op == kIROp_Return
                        ? (io.in.v == Initialization::Uninitialized
                            ? Diagnostics::returningWithUninitializedOut
                            : Diagnostics::returningWithMaybeUninitializedOut)
                        : (io.in.v == Initialization::Uninitialized
                            ? Diagnostics::usingUninitializedValue
                            : Diagnostics::maybeUsingUninitializedValue),
                    param);
            };

            // The easy case, the initialization state didn't change during
            // this BB
            if(io.in.v == io.out.v)
            {
                diag();
            }
            // Otherwise, walk and find the place where it changed (only care
            // about loads here, as the return is necessarily after any
            // possible store)
            else if(!as<IRTerminatorInst>(l))
            {
                bool stored = false;
                for(IRInst* i = bb->getFirstInst(); i != l && !stored; i = i->getNextInst())
                {
                    if(i == l)
                        diag();
                    for(const auto& s : stores)
                    {
                        if(i == s.storeInst)
                        {
                            stored = true;
                            break;
                        }
                    }
                }
            }
        }
    }

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
                    checkForUsingUninitializedOutParam(func, sink, reachability, param);
                    break;
                }
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
