#pragma once

#include "slang-basic.h"

namespace Slang
{
    //
    // BitPackedShortList<E, N> is a shortlist of some backing type, in which
    // elements of type `E` are densely packaed, taking up `N` bits each. Any
    // bits beyond the low `N` in each `E` are dropped.
    //
    template<typename ElemType, Index ElemBits>
    struct BitPackedShortList
    {
        using BackingType = uint64_t;

        BitPackedShortList() = default;
        explicit BitPackedShortList(Index n, ElemType init)
        {
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
}
