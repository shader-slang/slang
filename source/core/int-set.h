#ifndef BIT_VECTOR_INT_SET_H
#define BIT_VECTOR_INT_SET_H

#include "list.h"
#include "slang-math.h"
#include "common.h"
#include "exception.h"

#include <memory.h>

namespace Slang
{
    /* The set works by storing as a bit per integer */
	class IntSet
	{
	private:
        typedef uint32_t Element;                           ///< Type that holds the bits
        enum
        {
            ELEMENT_SHIFT = 5,                              ///< How many bits to shift to get Element index from an index
            ELEMENT_SIZE = sizeof(Element) * 8,             ///< The number of bits in an element
            ELEMENT_MASK = ELEMENT_SIZE - 1,                ///< Mask to get shift from an index
        };

        // Make sure they are correct for the Element type
        SLANG_COMPILE_TIME_ASSERT( (1 << ELEMENT_SHIFT) == ELEMENT_SIZE);

		List<Element> m_buffer;

	public:
		IntSet()
		{}
		IntSet(const IntSet& other)
		{
			m_buffer = other.m_buffer;
		}
		IntSet(IntSet && other)
		{
			*this = (_Move(other));
		}
		IntSet& operator=(IntSet&& other)
		{
			m_buffer = _Move(other.m_buffer);
			return *this;
		}
		IntSet& operator=(const IntSet& other)
		{
			m_buffer = other.m_buffer;
			return *this;
		}
		int GetHashCode()
		{
			int rs = 0;
			for (auto val : m_buffer)
				rs ^= val;
			return rs;
		}
		IntSet(Int maxVal)
		{
			setMax(maxVal);
		}
		Int getCount() const
		{
			return Int(m_buffer.getCount()) * ELEMENT_SIZE;
		}
		void setMax(Int val)
		{
			resize(val);
			clear();
		}
		void setAll()
		{
			for (Index i = 0; i < m_buffer.getCount(); i++)
				m_buffer[i] = ~Element(0); 
		}
		void resize(Int size)
		{
            const Index oldBufferSize = m_buffer.getCount();
            const Index newCount = Index((size + ELEMENT_MASK) >> ELEMENT_SHIFT);
			m_buffer.setCount(newCount);

			if (newCount > oldBufferSize)
				memset(m_buffer.getBuffer() + oldBufferSize, 0, (newCount - oldBufferSize) * sizeof(Element));
		}
		void clear()
		{
			for (Index i = 0; i < m_buffer.getCount(); i++)
				m_buffer[i] = 0;
		}
        void clearAndDeallocate()
        {
            m_buffer.clearAndDeallocate();
        }
		void add(Int val)
		{
			Index id = Index(val >> 5);
			if (id < m_buffer.getCount())
				m_buffer[id] |= Element(1) << (val & ELEMENT_MASK);
			else
			{
				Index oldCount = m_buffer.getCount();
				m_buffer.setCount(id + 1);
				memset(m_buffer.getBuffer() + oldCount, 0, (m_buffer.getCount() - oldCount) * sizeof(Element));
				m_buffer[id] |= Element(1) << (val & ELEMENT_MASK);
			}
		}
		void remove(Int val)
		{
            const Index idx = Index(val >> ELEMENT_SHIFT);
			if (idx < m_buffer.getCount())
				m_buffer[idx] &= ~(Element(1) << (val & ELEMENT_MASK));
		}
		bool contains(Int val) const
		{
            const Index idx = Index(val >> ELEMENT_SHIFT);
			if (idx >= m_buffer.getCount())
				return false;
			return (m_buffer[idx] & (Element(1) << (val & ELEMENT_MASK))) != 0;
		}
		void unionWith(const IntSet& set)
		{
            const Index minCount = Math::Min(set.m_buffer.getCount(), m_buffer.getCount());
			for (Index i = 0; i < minCount; i++)
			{
				m_buffer[i] |= set.m_buffer[i];
			}
			if (set.m_buffer.getCount() > m_buffer.getCount())
				m_buffer.addRange(set.m_buffer.getBuffer()+m_buffer.getCount(), set.m_buffer.getCount()-m_buffer.getCount());
		}
		bool operator==(const IntSet& set) const
		{
            Index minCount = Math::Min(set.m_buffer.getCount(), m_buffer.getCount());
            if (::memcmp(m_buffer.getBuffer(), set.m_buffer.getBuffer(), minCount * sizeof(Element)) != 0)
            {
                return false;
            }
            return m_buffer.getCount() == set.m_buffer.getCount() || (_areRemainingZeros(m_buffer, minCount) && _areRemainingZeros(set.m_buffer, minCount));
		}
		bool operator!=(const IntSet& set) const
		{
			return !(*this == set);
		}
		void intersectWith(const IntSet& set)
		{
			if (set.m_buffer.getCount() < m_buffer.getCount())
				memset(m_buffer.getBuffer() + set.m_buffer.getCount(), 0, (m_buffer.getCount() - set.m_buffer.getCount()) * sizeof(Element));

            const Index minCount = Math::Min(set.m_buffer.getCount(), m_buffer.getCount());
			for (Index i = 0; i < minCount; i++)
			{
				m_buffer[i] &= set.m_buffer[i];
			}
		}
		static void calcUnion(IntSet& outRs, const IntSet& set1, const IntSet& set2)
		{
			outRs.m_buffer.setCount(Math::Max(set1.m_buffer.getCount(), set2.m_buffer.getCount()));
			outRs.clear();
			for (Index i = 0; i < set1.m_buffer.getCount(); i++)
				outRs.m_buffer[i] |= set1.m_buffer[i];
			for (Index i = 0; i < set2.m_buffer.getCount(); i++)
				outRs.m_buffer[i] |= set2.m_buffer[i];
		}
		static void calcIntersection(IntSet& outRs, const IntSet& set1, const IntSet& set2)
		{
            const Index minCount = Math::Min(set1.m_buffer.getCount(), set2.m_buffer.getCount());
			outRs.m_buffer.setCount(minCount);

			for (Index i = 0; i < minCount; i++)
				outRs.m_buffer[i] = set1.m_buffer[i] & set2.m_buffer[i];
		}
		static void calcSubtract(IntSet& outRs, const IntSet& set1, const IntSet& set2)
		{
			outRs.m_buffer.setCount(set1.m_buffer.getCount());

            const Index minCount = Math::Min(set1.m_buffer.getCount(), set2.m_buffer.getCount());
			for (Index i = 0; i < minCount; i++)
				outRs.m_buffer[i] = set1.m_buffer[i] & (~set2.m_buffer[i]);
		}
		static bool hasIntersection(const IntSet& set1, const IntSet& set2)
		{
            const Index minCount = Math::Min(set1.m_buffer.getCount(), set2.m_buffer.getCount());
			for (Index i = 0; i < minCount; i++)
			{
				if (set1.m_buffer[i] & set2.m_buffer[i])
					return true;
			}
			return false;
		}

    private:
        static bool _areRemainingZeros(const List<Element>& elems, Index minCount)
        {
            const Index count = elems.getCount();
            const Element* base = elems.getBuffer();

            for (Index i = minCount; i < count; ++i)
            {
                if (base[i])
                {
                    return false;
                }
            }
            return true;
        }

	};
}

#endif
