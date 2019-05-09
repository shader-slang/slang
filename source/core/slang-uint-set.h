#ifndef SLANG_UINT_SET_H
#define SLANG_UINT_SET_H

#include "list.h"
#include "slang-math.h"
#include "common.h"

#include <memory.h>

namespace Slang
{

/* Hold a set of UInt values. Implementation works by storing as a bit per value */
class UIntSet
{
public:
    typedef uint32_t Element;                           ///< Type that holds the bits
    enum
    {
        kElementShift = 5,                              ///< How many bits to shift to get Element index from an index
        kElementSize = sizeof(Element) * 8,             ///< The number of bits in an element
        kElementMask = kElementSize - 1,                ///< Mask to get shift from an index
    };

	UIntSet() {}
	UIntSet(const UIntSet& other) {	m_buffer = other.m_buffer; }
	UIntSet(UIntSet && other) { *this = (_Move(other)); }
    UIntSet(UInt maxVal) { resizeAndClear(maxVal); }

    UIntSet& operator=(UIntSet&& other);
    UIntSet& operator=(const UIntSet& other);

    int GetHashCode();

    Int getCount() const { return Int(m_buffer.getCount()) * kElementSize; }
			
    void resizeAndClear(UInt val);

    void setAll();

    void resize(UInt size);

    void clear();

    void clearAndDeallocate();

    inline void add(UInt val);
    inline void remove(UInt val);
    inline bool contains(UInt val) const;

    bool operator==(const UIntSet& set) const;
	bool operator!=(const UIntSet& set) const { return !(*this == set);	}

    void unionWith(const UIntSet& set);
    void intersectWith(const UIntSet& set);

    static void calcUnion(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2);
    static void calcIntersection(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2);
    static void calcSubtract(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2);

    static bool hasIntersection(const UIntSet& set1, const UIntSet& set2);

private:
    // Make sure they are correct for the Element type
    SLANG_COMPILE_TIME_ASSERT((1 << kElementShift) == kElementSize);

    List<Element> m_buffer;
};

// --------------------------------------------------------------------------
inline void UIntSet::remove(UInt val)
{
    const Index idx = Index(val >> kElementShift);
    if (idx < m_buffer.getCount())
    {
        m_buffer[idx] &= ~(Element(1) << (val & kElementMask));
    }
}

// --------------------------------------------------------------------------
inline bool UIntSet::contains(UInt val) const
{
    const Index idx = Index(val >> kElementShift);
    return idx <= m_buffer.getCount() &&
        ((m_buffer[idx] & (Element(1) << (val & kElementMask))) != 0);
}

// --------------------------------------------------------------------------
inline void UIntSet::add(UInt val)
{
    Index idx = Index(val >> kElementShift);
    if (idx >= m_buffer.getCount())
    {
        resize(val);
    }
    m_buffer[idx] |= Element(1) << (val & kElementMask);
}

}

#endif
