#include "slang-ir-use-uninitialized-out-param.h"

#include "slang-ir-dataflow.h"
#include "slang-ir-util.h"
#include "slang-ir-reachability.h"
#include "slang-product-lattice.h"
#include <limits>

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;

    namespace UninitializedDetail
    {
        enum V
        {
            Bottom             = 0b00,
            Initialized        = 0b01,
            Uninitialized      = 0b10,
            MaybeUninitialized = 0b11,
        };

        // Our join semilattice to dataflow over
        // struct Initialization
        // {
        //     bool join(Initialization other)
        //     {
        //         V joined = V(v | other.v);
        //         if(joined != v)
        //         {
        //             v = joined;
        //             return true;
        //         }
        //         return false;
        //     }
        //     static auto bottom() { return Initialization{Bottom}; }
        //
        //     V v;
        // };

        // Our join semilattice to dataflow over
        struct Initialization
        {
            bool join(Initialization other)
            {
                bool changed = false;
                auto joined = v | other.v;
                if(joined != v)
                {
                    v = joined;
                    changed = true;
                }
                return changed;
            }
            static auto bottom() { return Initialization{0}; }
            static auto allUninitialized() { return Initialization{0xAAAAAAAAAAAAAAAA}; }
            V getElem(int i) const
            {
                SLANG_ASSERT(i < maxElements);
                return V((v >> i*2) & 0b11);
            }
            void joinElem(int i, V o)
            {
                SLANG_ASSERT(i < maxElements);
                v |= (o << i*2);
            }
            void setElem(int i, V o)
            {
                SLANG_ASSERT(i < maxElements);
                v &= ~(0b11 << i*2);
                v |= o << i*2;
            }

            // Keep track of the 32 most significant elements
            // If there are more than 32 elements, track the first 31 and the
            // remainder.
            uint64_t v;
            static const int maxElements = std::numeric_limits<uint64_t>::digits / 2;
            static const int overflowElementIndex = maxElements - 1;

            // Hierarchical structs probably permit a more elaborate
            // representation...
        };

        struct LeafRange
        {
            Index begin;
            Index end;
        };

        struct StoreSite
        {
            IRInst* storeInst;
            LeafRange storeRange;
        };

        struct LoadSite
        {
            LeafRange loadRange;
            uint64_t initializationState;
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
                for(auto i = bb->getFirstInst(); i; i = i->getNextInst())
                {
                    for(const auto& s : stores)
                    {
                        if(i == s.storeInst)
                        {
                            for(Index m = s.storeRange.begin; m < s.storeRange.end; ++m)
                                next.setElem(m, Initialized);
                        }
                    }
                    if(auto l = loadsAndReturns.tryGetValue(i))
                        l->initializationState = next.v;
                }
                return out.join(next);
            }
            const List<StoreSite>& stores;
            const Dictionary<IRInst*, LoadSite>& loadsAndReturns;
        };

        // A tree stored in preorder
        template<typename T>
        struct Branch
        {
            // The index of the next sibling
            // leaf nodes are those which point directly to the next index
            Index next;
            T data;
        };

        struct NestingMember 
        {
            IRStructKey* key;
            // If we imagine a totally flat struct, these indices represent the
            // range of leaves this member encompasses.
            LeafRange leafRange;
        };

        using MemberTree = List<Branch<NestingMember>>;

        //
        // Perform a preorder traversal over the struct hierarchy
        // Maintain a tree in 'memberTree' which will allow us to construct
        // initialization state for portions of a struct
        //
        static MemberTree flattenStruct(IRType* type)
        {
            MemberTree tree;
            Index leafIndex = 0;
            auto walkStructType = [&](auto& go, IRType* t) -> Branch<NestingMember>&
            {
                Index i = tree.getCount();
                tree.add(Branch<NestingMember>{0, {nullptr, {leafIndex, leafIndex}}});
                if(auto structType = as<IRStructType>(t))
                {
                    for(auto field : structType->getFields())
                        go(go, field->getFieldType()).data.key = field->getKey();
                }
                else
                {
                    leafIndex++;
                }
                tree[i].next = tree.getCount();
                tree[i].data.leafRange.end = leafIndex;
                return tree[i];
            };
            walkStructType(walkStructType, type);
            return tree;
        }

    }

    static void checkForUsingUninitializedOutParam(
        IRFunc* func,
        DiagnosticSink* sink,
        ReachabilityContext& reachability,
        IRParam* param)
    {
        using namespace UninitializedDetail;

        auto outType = as<IROutType>(param->getDataType());
        SLANG_ASSERT(outType);
        auto type = outType->getValueType();

        const auto memberTree = flattenStruct(type);
        const LeafRange everythingRange = memberTree[0].data.leafRange;

        // Collect all sub-addresses from the param.
        List<StoreSite> stores;
        Dictionary<IRInst*, LoadSite> loadsAndReturns;
        List<IRBlock*> loadingBlocks;
        auto walkAddressUses = [&](auto& go, IRInst* addr, const Index addrMemberIndex) -> void
        {
            for (auto use = addr->firstUse; use; use = use->nextUse)
            {
                instMatch_(use->getUser(),
                    [&](IRFieldAddress* fa)
                    {
                        auto key = as<IRStructKey>(fa->getField());
                        // TODO: Is this assert correct?
                        SLANG_ASSERT(key);

                        // Move to the children
                        Index childMemberIndex = addrMemberIndex + 1;
                        SLANG_ASSERT(childMemberIndex < memberTree.getCount());
                        while(memberTree[childMemberIndex].data.key != key)
                        {
                            childMemberIndex = memberTree[childMemberIndex].next;
                            SLANG_ASSERT(childMemberIndex < memberTree.getCount());
                        }
                        int i = Initialization::overflowElementIndex;
                        return go(go, use->getUser(), childMemberIndex);
                    },
                    [&](IRGetElementPtr*)
                    {
                        // SLANG_ASSERT(!"TODO");
                    },
                    [&](IRStore* store)
                    {
                        if(addr == store->getPtr())
                            stores.add({store, memberTree[addrMemberIndex].data.leafRange});
                    },
                    [&](IRSwizzledStore* store)
                    {
                        if(addr == store->getDest())
                            stores.add({store, memberTree[addrMemberIndex].data.leafRange});
                    },
                    [&](IRCall* call)
                    {
                        // TODO: Take into account the polarity of this argument
                        stores.add({call, memberTree[addrMemberIndex].data.leafRange});
                    },
                    [&](IRLoad* load)
                    {
                        IRBlock* bb = as<IRBlock>(load->getParent());
                        SLANG_ASSERT(bb);
                        if(!loadingBlocks.contains(bb))
                            loadingBlocks.add(bb);
                        loadsAndReturns.add(load, {memberTree[addrMemberIndex].data.leafRange, Uninitialized});
                    },
                    [&](IRInst*)
                    {
                        SLANG_UNREACHABLE("Non-exhaustive patterns in walkAddressUses");
                    }
                );
            }
        };
        walkAddressUses(walkAddressUses, param, 0);

        // Also add a reference to the whole value on each return
        for(auto bb : func->getBlocks())
        {
            auto t = bb->getTerminator();
            if (t->m_op == kIROp_Return)
            {
                if(!loadingBlocks.contains(bb))
                    loadingBlocks.add(bb);
                loadsAndReturns.add(t, {everythingRange, Uninitialized});
            }
        }

        //
        // Run the dataflow analysis
        //
        UninitializedDataFlow context{{}, stores, loadsAndReturns};
        basicBlockForwardDataFlow(
            context,
            loadingBlocks,
            reachability,
            Initialization::allUninitialized());

        //
        // Walk over all of our instructions under investigation,
        //
        // If their value was uninitialized when they loaded, produce a
        // diagnostic
        //
        for(auto l : loadsAndReturns)
        {
            uint64_t status = l.value.initializationState;
            auto range = l.value.loadRange;

            bool areAllLeavesInitialized = true;
            bool areNoLeavesInitialized = true;
            bool areAnyLeavesMaybeUninitialized = false;
            for(Index i = range.begin; i < range.end; ++i)
            {
                const auto leaf = (status >> (i * 2)) & 0b11;
                const bool isLeafInitialized = leaf == Initialized;
                const bool isLeafMaybeUninitialized = leaf == MaybeUninitialized;
                areAllLeavesInitialized = areAllLeavesInitialized && isLeafInitialized;
                areNoLeavesInitialized = areNoLeavesInitialized && !isLeafInitialized;
                areAnyLeavesMaybeUninitialized = areAnyLeavesMaybeUninitialized || isLeafMaybeUninitialized;
            }

            // If this is guaranteed initialized, no need to do anything
            if(areAllLeavesInitialized)
                continue;

            const bool isReturn = as<IRReturn>(l.key);
            const bool isMaybe = areAnyLeavesMaybeUninitialized;
            const bool isPartially = !areAllLeavesInitialized && !areNoLeavesInitialized;
            const auto d =
                  !isReturn && !isMaybe && !isPartially ? Diagnostics::usingUninitializedValue
                : !isReturn && !isMaybe &&  isPartially ? Diagnostics::usingPartiallyUninitializedValue
                : !isReturn &&  isMaybe && !isPartially ? Diagnostics::usingMaybeUninitializedValue
                : !isReturn &&  isMaybe &&  isPartially ? Diagnostics::usingMaybePartiallyUninitializedValue
                :  isReturn && !isMaybe && !isPartially ? Diagnostics::returningWithUninitializedOut
                :  isReturn && !isMaybe &&  isPartially ? Diagnostics::returningWithPartiallyUninitializedOut
                :  isReturn &&  isMaybe && !isPartially ? Diagnostics::returningWithMaybeUninitializedOut
                :  isReturn &&  isMaybe &&  isPartially ? Diagnostics::returningWithMaybePartiallyUninitializedOut
                : (SLANG_UNREACHABLE("impossible"), Diagnostics::internalCompilerError);
            sink->diagnose(l.key, d, param);
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
