#include "slang-uint-set.h"

namespace Slang
{

static bool _areAllZero(const UIntSet::Element* elems, Index count)
{
    for (Index i = 0; count; ++i)
    {
        if (elems[i])
        {
            return false;
        }
    }
    return true;
}

UIntSet& UIntSet::operator=(UIntSet&& other)
{
    m_buffer = _Move(other.m_buffer);
    return *this;
}

UIntSet& UIntSet::operator=(const UIntSet& other)
{
    m_buffer = other.m_buffer;
    return *this;
}

HashCode UIntSet::getHashCode() const
{
    int rs = 0;
    for (auto val : m_buffer)
       rs ^= val;
    return rs;
}

void UIntSet::resizeAndClear(UInt val)
{
    // TODO(JS): This could be faster in that if the resize is larger the additional area is cleared twice
    resize(val);
    clear();
}

void UIntSet::setAll()
{
    ::memset(m_buffer.getBuffer(), -1, m_buffer.getCount() * sizeof(Element));
}

void UIntSet::resize(UInt size)
{
    const Index oldCount = m_buffer.getCount();
    const Index newCount = Index((size + kElementMask) >> kElementShift);
    m_buffer.setCount(newCount);

    if (newCount > oldCount)
    {
        ::memset(m_buffer.getBuffer() + oldCount, 0, (newCount - oldCount) * sizeof(Element));
    }
}

void UIntSet::clear()
{
    ::memset(m_buffer.getBuffer(), 0, m_buffer.getCount() * sizeof(Element));
}

bool UIntSet::isEmpty() const
{
    const Element*const src = m_buffer.getBuffer();
    const Index count = m_buffer.getCount();

    for (Index i = 0; i < count; ++i)
    {
        if (src[i])
        {
            return false;
        }
    }
    return true;
}

void UIntSet::clearAndDeallocate()
{
    m_buffer.clearAndDeallocate();
}

void UIntSet::unionWith(const UIntSet& set)
{
    const Index minCount = Math::Min(set.m_buffer.getCount(), m_buffer.getCount());
    for (Index i = 0; i < minCount; i++)
    {
        m_buffer[i] |= set.m_buffer[i];
    }

    if (set.m_buffer.getCount() > m_buffer.getCount())
        m_buffer.addRange(set.m_buffer.getBuffer() + m_buffer.getCount(), set.m_buffer.getCount() - m_buffer.getCount());
}

bool UIntSet::operator==(const UIntSet& set) const
{
    const Index aCount = m_buffer.getCount();
    const auto aElems = m_buffer.getBuffer();

    const Index bCount = set.m_buffer.getCount();
    const auto bElems = set.m_buffer.getBuffer();

    const Index minCount = Math::Min(aCount, bCount);
    
    return ::memcmp(aElems, bElems, minCount) == 0 &&
        _areAllZero(aElems + minCount, aCount - minCount) &&
        _areAllZero(bElems + minCount, bCount - minCount);
}

void UIntSet::intersectWith(const UIntSet& set)
{
    if (set.m_buffer.getCount() < m_buffer.getCount())
        ::memset(m_buffer.getBuffer() + set.m_buffer.getCount(), 0, (m_buffer.getCount() - set.m_buffer.getCount()) * sizeof(Element));

    const Index minCount = Math::Min(set.m_buffer.getCount(), m_buffer.getCount());
    for (Index i = 0; i < minCount; i++)
    {
        m_buffer[i] &= set.m_buffer[i];
    }
}

/* static */void UIntSet::calcUnion(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2)
{
    outRs.m_buffer.setCount(Math::Max(set1.m_buffer.getCount(), set2.m_buffer.getCount()));
    outRs.clear();
    for (Index i = 0; i < set1.m_buffer.getCount(); i++)
        outRs.m_buffer[i] |= set1.m_buffer[i];
    for (Index i = 0; i < set2.m_buffer.getCount(); i++)
        outRs.m_buffer[i] |= set2.m_buffer[i];
}

/* static */void UIntSet::calcIntersection(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2)
{
    const Index minCount = Math::Min(set1.m_buffer.getCount(), set2.m_buffer.getCount());
    outRs.m_buffer.setCount(minCount);

    for (Index i = 0; i < minCount; i++)
        outRs.m_buffer[i] = set1.m_buffer[i] & set2.m_buffer[i];
}

/* static */void UIntSet::calcSubtract(UIntSet& outRs, const UIntSet& set1, const UIntSet& set2)
{
    outRs.m_buffer.setCount(set1.m_buffer.getCount());

    const Index minCount = Math::Min(set1.m_buffer.getCount(), set2.m_buffer.getCount());
    for (Index i = 0; i < minCount; i++)
        outRs.m_buffer[i] = set1.m_buffer[i] & (~set2.m_buffer[i]);
}

/* static */bool UIntSet::hasIntersection(const UIntSet& set1, const UIntSet& set2)
{
    const Index minCount = Math::Min(set1.m_buffer.getCount(), set2.m_buffer.getCount());
    for (Index i = 0; i < minCount; i++)
    {
        if (set1.m_buffer[i] & set2.m_buffer[i])
            return true;
    }
    return false;
}

}

