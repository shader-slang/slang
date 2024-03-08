#ifndef SLANG_CORE_UINT_SET_H
#define SLANG_CORE_UINT_SET_H

#include "slang-list.h"
#include "slang-math.h"
#include "slang-common.h"
#include "slang-hash.h"

#include <memory.h>

namespace Slang
{

/* Hold a set of UInt values. Implementation works by storing as a bit per value */
class UIntSet
{
public:
    typedef UIntSet ThisType;
    typedef uint32_t Element;                                   ///< Type that holds the bits to say if value is present
    
    UIntSet() {}
    UIntSet(const UIntSet& other) { m_buffer = other.m_buffer; }
    UIntSet(UIntSet && other) { *this = (_Move(other)); }
    UIntSet(UInt maxVal) { resizeAndClear(maxVal); }

    UIntSet& operator=(UIntSet&& other);
    UIntSet& operator=(const UIntSet& other);

    HashCode getHashCode() const;

        /// Return the count of all bits directly represented
    Int getCount() const { return Int(m_buffer.getCount()) * kElementSize; }

        /// Resize such that val can be stored and clear contents
    void resizeAndClear(UInt val);
        /// Set all of the values up to count, as set
    void setAll();
        /// Resize (but maintain contents) up to bit size.
        /// NOTE! That since storage is in Element blocks, it may mean some values after size are set (up to the Element boundary)
    void resize(UInt size);

        /// Clear all of the contents (by clearing the bits)
    void clear();

        /// Clear all the contents and free memory
    void clearAndDeallocate();

        /// Add a value
    inline void add(UInt val);
        /// Remove a value
    inline void remove(UInt val);
        /// Returns true if the value is present
    inline bool contains(UInt val) const;

    inline bool contains(const UIntSet& set) const;

        /// ==
    bool operator==(const UIntSet& set) const;
        /// !=
    bool operator!=(const UIntSet& set) const { return !(*this == set); }

        /// Store the union between this and set in this
    void unionWith(const UIntSet& set);
        /// Store the intersection between this and set in this
    void intersectWith(const UIntSet& set);

        /// 
    bool isEmpty() const;

        /// Swap this with rhs
    void swapWith(ThisType& rhs) { m_buffer.swapWith(rhs.m_buffer); }

        /// Store the union of set1 and set2 in outRs
    static void calcUnion(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2);
        /// Store the intersection of set1 and set2 in outRs
    static void calcIntersection(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2);
        /// Store the subtraction of set2 from set1 in outRs
    static void calcSubtract(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2);

        /// Returns true if set1 and set2 have a same value set (ie there is an intersection)
    static bool hasIntersection(const UIntSet& set1, const UIntSet& set2);

private:
    enum
    {
        kElementShift = 5,                              ///< How many bits to shift to get Element index from an index
        kElementSize = sizeof(Element) * 8,             ///< The number of bits in an element
        kElementMask = kElementSize - 1,                ///< Mask to get shift from an index
    };

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
    return idx < m_buffer.getCount() &&
        ((m_buffer[idx] & (Element(1) << (val & kElementMask))) != 0);
}

// --------------------------------------------------------------------------
inline bool UIntSet::contains(const UIntSet& set) const
{
    for (Index i = 0; i < set.m_buffer.getCount(); i++)
    {
        if (i >= m_buffer.getCount())
        {
            if (set.m_buffer[i])
                return false;
        }
        else
        {
            if ((m_buffer[i] & set.m_buffer[i]) != set.m_buffer[i])
                return false;
        }
    }
    return true;
}

// --------------------------------------------------------------------------
inline void UIntSet::add(UInt val)
{
    const Index idx = Index(val >> kElementShift);
    if (idx >= m_buffer.getCount())
    {
        resize(val + 1);
    }
    m_buffer[idx] |= Element(1) << (val & kElementMask);
}

}

#endif
