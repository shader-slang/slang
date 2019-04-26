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
			return buffer.getSize()*32;
		}
		void SetMax(int val)
		{
			Resize(val);
			Clear();
		}
		void SetAll()
		{
			for (UInt i = 0; i<buffer.getSize(); i++)
				buffer[i] = 0xFFFFFFFF;
		}
		void Resize(UInt size)
		{
			UInt oldBufferSize = buffer.getSize();
			buffer.SetSize((size+31)>>5);
			if (buffer.getSize() > oldBufferSize)
				memset(buffer.Buffer()+oldBufferSize, 0, (buffer.getSize()-oldBufferSize) * sizeof(int));
		}
		void Clear()
		{
			for (UInt i = 0; i<buffer.getSize(); i++)
				buffer[i] = 0;
		}
		void Add(UInt val)
		{
			UInt id = val>>5;
			if (id < buffer.getSize())
				buffer[id] |= (1<<(val&31));
			else
			{
				UInt oldSize = buffer.getSize();
				buffer.SetSize(id+1);
				memset(buffer.Buffer() + oldSize, 0, (buffer.getSize()-oldSize)*sizeof(int));
				buffer[id] |= (1<<(val&31));
			}
		}
		void Remove(UInt val)
		{
			if ((val>>5) < buffer.getSize())
				buffer[(val>>5)] &= ~(1<<(val&31));
		}
		bool Contains(UInt val) const
		{
			if ((val>>5) >= buffer.getSize())
				return false;
			return (buffer[(val>>5)] & (1<<(val&31))) != 0;
		}
		void UnionWith(const IntSet & set)
		{
			for (UInt i = 0; i<Math::Min(set.buffer.getSize(), buffer.getSize()); i++)
			{
				buffer[i] |= set.buffer[i];
			}
			if (set.buffer.getSize() > buffer.getSize())
				buffer.addRange(set.buffer.Buffer()+buffer.getSize(), set.buffer.getSize()-buffer.getSize());
		}
		bool operator == (const IntSet & set)
		{
			if (buffer.getSize() != set.buffer.getSize())
				return false;
			for (UInt i = 0; i<buffer.getSize(); i++)
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
			if (set.buffer.getSize() < buffer.getSize())
				memset(buffer.Buffer() + set.buffer.getSize(), 0, (buffer.getSize()-set.buffer.getSize())*sizeof(int));
			for (UInt i = 0; i<Math::Min(set.buffer.getSize(), buffer.getSize()); i++)
			{
				buffer[i] &= set.buffer[i];
			}
		}
		static void Union(IntSet & rs, const IntSet & set1, const IntSet & set2)
		{
			rs.buffer.SetSize(Math::Max(set1.buffer.getSize(), set2.buffer.getSize()));
			rs.Clear();
			for (UInt i = 0; i<set1.buffer.getSize(); i++)
				rs.buffer[i] |= set1.buffer[i];
			for (UInt i = 0; i<set2.buffer.getSize(); i++)
				rs.buffer[i] |= set2.buffer[i];
		}
		static void Intersect(IntSet & rs, const IntSet & set1, const IntSet & set2)
		{
			rs.buffer.SetSize(Math::Min(set1.buffer.getSize(), set2.buffer.getSize()));
			for (UInt i = 0; i<rs.buffer.getSize(); i++)
				rs.buffer[i] = set1.buffer[i] & set2.buffer[i];
		}
		static void Subtract(IntSet & rs, const IntSet & set1, const IntSet & set2)
		{
			rs.buffer.SetSize(set1.buffer.getSize());
			for (UInt i = 0; i<Math::Min(set1.buffer.getSize(), set2.buffer.getSize()); i++)
				rs.buffer[i] = set1.buffer[i] & (~set2.buffer[i]);
		}
		static bool HasIntersection(const IntSet & set1, const IntSet & set2)
		{
			for (UInt i = 0; i<Math::Min(set1.buffer.getSize(), set2.buffer.getSize()); i++)
			{
				if (set1.buffer[i] & set2.buffer[i])
					return true;
			}
			return false;
		}
	};
}

#endif
