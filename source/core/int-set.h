#ifndef BIT_VECTOR_INT_SET_H
#define BIT_VECTOR_INT_SET_H

#include "list.h"
#include "slang-math.h"
#include "common.h"
#include "exception.h"

#include <memory.h>

namespace Slang
{
	class IntSet
	{
	private:
		List<int> buffer;
	public:
		IntSet()
		{}
		IntSet(const IntSet & other)
		{
			buffer = other.buffer;
		}
		IntSet(IntSet && other)
		{
			*this = (_Move(other));
		}
		IntSet & operator = (IntSet && other)
		{
			buffer = _Move(other.buffer);
			return *this;
		}
		IntSet & operator = (const IntSet & other)
		{
			buffer = other.buffer;
			return *this;
		}
		int GetHashCode()
		{
			int rs = 0;
			for (auto val : buffer)
				rs ^= val;
			return rs;
		}
		IntSet(int maxVal)
		{
			SetMax(maxVal);
		}
		UInt Size() const
		{
			return buffer.getCount()*32;
		}
		void SetMax(int val)
		{
			Resize(val);
			Clear();
		}
		void SetAll()
		{
			for (UInt i = 0; i<buffer.getCount(); i++)
				buffer[i] = 0xFFFFFFFF;
		}
		void Resize(UInt size)
		{
			UInt oldBufferSize = buffer.getCount();
			buffer.setCount((size+31)>>5);
			if (buffer.getCount() > oldBufferSize)
				memset(buffer.getBuffer()+oldBufferSize, 0, (buffer.getCount()-oldBufferSize) * sizeof(int));
		}
		void Clear()
		{
			for (UInt i = 0; i<buffer.getCount(); i++)
				buffer[i] = 0;
		}
		void Add(UInt val)
		{
			UInt id = val>>5;
			if (id < buffer.getCount())
				buffer[id] |= (1<<(val&31));
			else
			{
				UInt oldSize = buffer.getCount();
				buffer.setCount(id+1);
				memset(buffer.getBuffer() + oldSize, 0, (buffer.getCount()-oldSize)*sizeof(int));
				buffer[id] |= (1<<(val&31));
			}
		}
		void Remove(UInt val)
		{
			if ((val>>5) < buffer.getCount())
				buffer[(val>>5)] &= ~(1<<(val&31));
		}
		bool Contains(UInt val) const
		{
			if ((val>>5) >= buffer.getCount())
				return false;
			return (buffer[(val>>5)] & (1<<(val&31))) != 0;
		}
		void UnionWith(const IntSet & set)
		{
			for (UInt i = 0; i<Math::Min(set.buffer.getCount(), buffer.getCount()); i++)
			{
				buffer[i] |= set.buffer[i];
			}
			if (set.buffer.getCount() > buffer.getCount())
				buffer.addRange(set.buffer.getBuffer()+buffer.getCount(), set.buffer.getCount()-buffer.getCount());
		}
		bool operator == (const IntSet & set)
		{
			if (buffer.getCount() != set.buffer.getCount())
				return false;
			for (UInt i = 0; i<buffer.getCount(); i++)
				if (buffer[i] != set.buffer[i])
					return false;
			return true;
		}
		bool operator != (const IntSet & set)
		{
			return !(*this == set);
		}
		void IntersectWith(const IntSet & set)
		{
			if (set.buffer.getCount() < buffer.getCount())
				memset(buffer.getBuffer() + set.buffer.getCount(), 0, (buffer.getCount()-set.buffer.getCount())*sizeof(int));
			for (UInt i = 0; i<Math::Min(set.buffer.getCount(), buffer.getCount()); i++)
			{
				buffer[i] &= set.buffer[i];
			}
		}
		static void Union(IntSet & rs, const IntSet & set1, const IntSet & set2)
		{
			rs.buffer.setCount(Math::Max(set1.buffer.getCount(), set2.buffer.getCount()));
			rs.Clear();
			for (UInt i = 0; i<set1.buffer.getCount(); i++)
				rs.buffer[i] |= set1.buffer[i];
			for (UInt i = 0; i<set2.buffer.getCount(); i++)
				rs.buffer[i] |= set2.buffer[i];
		}
		static void Intersect(IntSet & rs, const IntSet & set1, const IntSet & set2)
		{
			rs.buffer.setCount(Math::Min(set1.buffer.getCount(), set2.buffer.getCount()));
			for (UInt i = 0; i<rs.buffer.getCount(); i++)
				rs.buffer[i] = set1.buffer[i] & set2.buffer[i];
		}
		static void Subtract(IntSet & rs, const IntSet & set1, const IntSet & set2)
		{
			rs.buffer.setCount(set1.buffer.getCount());
			for (UInt i = 0; i<Math::Min(set1.buffer.getCount(), set2.buffer.getCount()); i++)
				rs.buffer[i] = set1.buffer[i] & (~set2.buffer[i]);
		}
		static bool HasIntersection(const IntSet & set1, const IntSet & set2)
		{
			for (UInt i = 0; i<Math::Min(set1.buffer.getCount(), set2.buffer.getCount()); i++)
			{
				if (set1.buffer[i] & set2.buffer[i])
					return true;
			}
			return false;
		}
	};
}

#endif
