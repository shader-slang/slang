#include "slang-ir-use-uninitialized-out-param.h"

#include "slang-ir-dataflow.h"
#include "slang-ir-util.h"
#include "slang-ir-reachability.h"
#include "../core/slang-bit-packed-short-list.h"

namespace Slang
{
    // The values which make up the components in our product join semilattice,
    // so valued as to enable use of bitwise or as a join operation.
    enum V
    {
        Bottom             = 0b00,
        Initialized        = 0b01,
        Uninitialized      = 0b10,
        MaybeUninitialized = 0b11,
    };

    // Our join semilattice to dataflow over, a product of several `V`s
    struct Initialization
    {
        using BitStorage = BitPackedShortList<V, 2>;

        bool join(const Initialization& other)
        {
            bool changed = false;
            const auto joined = v | other.v;
            if(joined != v)
            {
                v = joined;
                changed = true;
            }
            return changed;
        }
        static auto bottom(Index n) { return Initialization{BitStorage(n, Bottom)}; }
        static auto allUninitialized(Index n) { return Initialization{BitStorage(n, Uninitialized)}; }
        V getElem(Index i) const
        {
            return v.getElem(i);
        }
        void joinElem(Index i, V o)
        {
            v.orElem(i, o);
        }
        void setElem(Index i, V o)
        {
            v.setElem(i, o);
        }

        BitStorage v;
    };

    //
    // Now we define a small tree type, which we'll use to construct a mask of
    // `V` covering elements in a struct.
    //
    struct LeafRange
    {
        Index begin;
        Index end;
    };

    // A tree stored in preorder, represented as a linked list, where each
    // vertex contains the index of its next sibling. Anything between that
    // vertex and the sibling is a descendent of that vertex.
    template<typename T>
    struct Branch
    {
        // The index of the next sibling
        // leaf nodes are those which point directly to the next index
        Index next;
        T data;
    };

    // Each vertex stores the struct member which is represents in the nesting.
    struct NestingMember
    {
        IRStructKey* key;
        // If we imagine a totally flat struct, these indices represent the
        // range of leaves this member encompasses. The union of all children's
        // ranges, or if this is a leaf then the singleton index of this
        // position in the structure.
        LeafRange leafRange;
    };

    // A tree is a flat list of all the branches in the tree, stored in
    // preorder
    using MemberTree = List<Branch<NestingMember>>;

    //
    // Our dataflow setup
    //

    // An instruction which is writing to some portion of our out parameter, we
    // hold onto the storing instruction and the range of the structure it
    // writes to.
    struct StoreSite
    {
        IRInst* storeInst;
        LeafRange storeRange;
    };

    // For loads, we remember the range of values being loaded and also their
    // calculated initialization state.
    struct LoadSite
    {
        LeafRange loadRange;
        Initialization initializationState;
    };

    //
    // Our dataflow context
    //
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
                // If we encounter one of our stores, mark that location
                // as initialized.
                for(const auto& s : stores)
                {
                    if(i == s.storeInst)
                    {
                        for(Index m = s.storeRange.begin; m < s.storeRange.end; ++m)
                            next.setElem(m, Initialized);
                    }
                }
                // If we encounter one of our loads, remember the
                // initialization state at this point. This might be updated
                // several times as we revisit blocks in the dataflow run
                if(auto l = loadsAndReturns.tryGetValue(i))
                    l->initializationState = next;
            }
            return out.join(next);
        }
        const List<StoreSite>& stores;
        Dictionary<IRInst*, LoadSite>& loadsAndReturns;
    };

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

    static void checkForUsingUninitializedOutParam(
        IRFunc* func,
        DiagnosticSink* sink,
        ReachabilityContext& reachability,
        IRParam* param)
    {
        auto outType = as<IROutType>(param->getDataType());
        SLANG_ASSERT(outType);
        auto type = outType->getValueType();

        const auto memberTree = flattenStruct(type);
        const LeafRange everythingRange = memberTree[0].data.leafRange;
        const Index numFlatMembers = everythingRange.end;

        // Collect all sub-addresses from the param.
        List<StoreSite> stores;
        Dictionary<IRInst*, LoadSite> loadsAndReturns;
        List<IRBlock*> loadingBlocks;
        auto walkAddressUses = [&](
            auto& go,
            // The address under consideration, it will be the address of our
            // out parameter or some member nested underneath it
            IRInst* addr,
            // The index of this member in our flattened representation
            const Index addrMemberIndex,
            // If this is true then we'll keep following IRFieldAddress
            // instructions, if it's false then we'll never advance addrMemberIndex
            const bool allowDescent) -> void
        {
            for (auto use = addr->firstUse; use; use = use->nextUse)
            {
                instMatch_(use->getUser(),
                    [&](IRFieldAddress* fa)
                    {
                        // If we're not allowed to make the leaves more
                        // specific, then all we can do is recurse with our
                        // given addrMemberIndex
                        if(!allowDescent)
                            return go(go, fa, addrMemberIndex, false);

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
                        return go(go, fa, childMemberIndex, true);
                    },
                    [&](IRGetElementPtr* gep)
                    {
                        // For now, treat a write to any element as a write to
                        // all elements, as we don't track individual elements
                        // We do this by recursing with allowDescent = false;
                        return go(go, gep, addrMemberIndex, false);
                    },
                    [&](IRStore* store)
                    {
                        if(addr == store->getPtr())
                            stores.add({store, memberTree[addrMemberIndex].data.leafRange});
                    },
                    [&](IRSwizzledStore* store)
                    {
                        // TODO: track individual vector elements
                        if(addr == store->getDest())
                            stores.add({store, memberTree[addrMemberIndex].data.leafRange});
                    },
                    [&](IRCall* call)
                    {
                        // TODO: Take into account the polarity of this
                        // argument, only set as a store if we're passing as an
                        // out parameter
                        stores.add({call, memberTree[addrMemberIndex].data.leafRange});
                    },
                    [&](IRLoad* load)
                    {
                        IRBlock* bb = as<IRBlock>(load->getParent());
                        SLANG_ASSERT(bb);
                        if(!loadingBlocks.contains(bb))
                            loadingBlocks.add(bb);
                        loadsAndReturns.add(
                            load,
                            {memberTree[addrMemberIndex].data.leafRange,
                             Initialization::allUninitialized(numFlatMembers)
                            });
                    },
                    [&](IRInst* inst)
                    {
                        // Be conservative and assume anything unknown as a store
                        stores.add({inst, memberTree[addrMemberIndex].data.leafRange});
                    }
                );
            }
        };
        walkAddressUses(walkAddressUses, param, 0, true);

        // Also add a reference to the whole value on each return
        for(auto bb : func->getBlocks())
        {
            auto t = bb->getTerminator();
            if (t->m_op == kIROp_Return)
            {
                if(!loadingBlocks.contains(bb))
                    loadingBlocks.add(bb);
                loadsAndReturns.add(t, {everythingRange, Initialization::allUninitialized(numFlatMembers)});
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
            Initialization::bottom(numFlatMembers),
            Initialization::allUninitialized(numFlatMembers));

        //
        // Walk over all of our instructions under investigation,
        //
        // If their value was uninitialized when they loaded, produce a
        // diagnostic
        //
        for(const auto& [inst, site] : loadsAndReturns)
        {
            Initialization status = site.initializationState;
            auto range = site.loadRange;

            bool areAllLeavesInitialized = true;
            bool areNoLeavesInitialized = true;
            bool areAnyLeavesMaybeUninitialized = false;
            for(Index i = range.begin; i < range.end; ++i)
            {
                const auto leaf = status.getElem(i);
                SLANG_ASSERT(leaf != Bottom);
                const bool isLeafInitialized = leaf == Initialized;
                const bool isLeafMaybeUninitialized = leaf == MaybeUninitialized;
                areAllLeavesInitialized = areAllLeavesInitialized && isLeafInitialized;
                areNoLeavesInitialized = areNoLeavesInitialized && !isLeafInitialized;
                areAnyLeavesMaybeUninitialized = areAnyLeavesMaybeUninitialized || isLeafMaybeUninitialized;
            }

            // If this is guaranteed initialized, no need to do anything
            if(areAllLeavesInitialized)
                continue;

            const bool isReturn = as<IRReturn>(inst);
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
            sink->diagnose(inst, d, param);
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
