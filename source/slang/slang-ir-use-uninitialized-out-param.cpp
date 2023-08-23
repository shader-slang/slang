#include "slang-ir-use-uninitialized-out-param.h"

#include "slang-ir-dataflow.h"
#include "slang-ir-util.h"
#include "slang-ir-reachability.h"
#include <limits>

namespace Slang
{
    class DiagnosticSink;
    struct IRModule;

    namespace UninitializedDetail
    {
        template<typename ElemType, Index ElemBits>
        struct BitPackedShortList
        {
            using BackingType = uint64_t;

            BitPackedShortList() = default;
            explicit BitPackedShortList(Index n, ElemType init)
            {
                // BackingType rep = 0;
                // for(Index i = 0; i < kBackingTypeBits / ElemBits; ++i)
                //     rep |= static_cast<BackingType>(init) << (i * ElemBits);
                Index requiredBits = ElemBits * n;
                Index requiredBackingValues = (requiredBits + kBackingTypeBits - 1) / kBackingTypeBits;
                for(Index i = 0; i < requiredBackingValues; ++i)
                    m_backingBits.add(0);
                for(Index i = 0; i < n; ++i)
                    orElem(i, init);
            }
            BitPackedShortList(const BitPackedShortList& other) = default;
            BitPackedShortList& operator=(const BitPackedShortList& other) = default;
            BitPackedShortList(BitPackedShortList&& other) = default;
            BitPackedShortList& operator=(BitPackedShortList&& other) = default;

            ElemType getElem(const Index i) const
            {
                const auto u = (i * ElemBits) / kBackingTypeBits;
                const auto s = (i * ElemBits) % kBackingTypeBits;
                return static_cast<ElemType>((m_backingBits[u] >> s) & kElemMask);
            }
            void setElem(const Index i, ElemType e)
            {
                const auto u = (i * ElemBits) / kBackingTypeBits;
                const auto s = (i * ElemBits) % kBackingTypeBits;
                const BackingType clearMask = ~(kElemMask << s);
                const BackingType setMask = static_cast<BackingType>(e) << s;
                BackingType& b = m_backingBits[u];
                b &= clearMask;
                b |= setMask;
            }
            void orElem(const Index i, ElemType e)
            {
                const auto u = (i * ElemBits) / kBackingTypeBits;
                const auto s = (i * ElemBits) % kBackingTypeBits;
                const BackingType setMask = static_cast<BackingType>(e) << s;
                BackingType& b = m_backingBits[u];
                b |= setMask;
            }
            // TODO: This checks the extra bits beyond what was initialized at
            // the start, we should make sure to zero these out.
            bool operator!=(const BitPackedShortList& other) const
            {
                return m_backingBits != other.m_backingBits;
            }
            auto operator&=(const BitPackedShortList& other)
            {
                SLANG_ASSERT(m_backingBits.getCount() == other.m_backingBits.getCount());
                for(Index i = 0; i < m_backingBits.getCount(); ++i)
                    m_backingBits[i] &= other.m_backingBits[i];
                return *this;
            }
            friend auto operator&(BitPackedShortList a, const BitPackedShortList& b)
            {
                a &= b;
                return std::move(a);
            }
            auto operator|=(const BitPackedShortList& other)
            {
                SLANG_ASSERT(m_backingBits.getCount() == other.m_backingBits.getCount());
                for(Index i = 0; i < m_backingBits.getCount(); ++i)
                    m_backingBits[i] |= other.m_backingBits[i];
                return *this;
            }
            friend auto operator|(BitPackedShortList a, const BitPackedShortList& b)
            {
                a |= b;
                return a;
            }

        private:
            static constexpr Index kBackingTypeBits = std::numeric_limits<BackingType>::digits;
            static constexpr BackingType kElemMask = (1 << ElemBits) - 1;
            // At least one element needs to fit into each backing bit block
            static_assert(ElemBits < kBackingTypeBits);
            // Elements needs to neatly divide the blocks
            static_assert(kBackingTypeBits % ElemBits == 0);

            ShortList<BackingType, 4> m_backingBits;
        };

        enum V
        {
            Bottom             = 0b00,
            Initialized        = 0b01,
            Uninitialized      = 0b10,
            MaybeUninitialized = 0b11,
        };

        // Our join semilattice to dataflow over
        struct Initialization
        {
            using BitStorage = BitPackedShortList<V, 2>;

            bool join(const Initialization& other)
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
            Initialization initializationState;
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
                        l->initializationState = next;
                }
                return out.join(next);
            }
            const List<StoreSite>& stores;
            Dictionary<IRInst*, LoadSite>& loadsAndReturns;
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
        const Index numFlatMembers = everythingRange.end;

        // Collect all sub-addresses from the param.
        List<StoreSite> stores;
        Dictionary<IRInst*, LoadSite> loadsAndReturns;
        List<IRBlock*> loadingBlocks;
        auto walkAddressUses = [&](
            // walkAddressUses, the C++ compiler isn't smart enough to allow
            // the recursive call here
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
                    [&](IRInst*)
                    {
                        SLANG_UNREACHABLE("Non-exhaustive patterns in walkAddressUses");
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
