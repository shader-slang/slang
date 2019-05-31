#ifndef SLANG_CORE_ARRAY_VIEW_H
#define SLANG_CORE_ARRAY_VIEW_H

#include "slang-common.h"

namespace Slang
{
	template<typename T>
	class ArrayView
	{
	private:
		T* m_buffer;
		int m_count;
	public:
        const T* begin() const { return m_buffer; }
        T* begin() { return m_buffer; }

        const T* end() const { return m_buffer + m_count; }
        T* end() { return m_buffer + m_count; }
        
	public:
		ArrayView():
			m_buffer(nullptr),
			m_count(0)
        {
		}
		ArrayView(T& singleObj):
            m_buffer(&singleObj),
            m_count(1)
        {
		}
		ArrayView(T* buffer, int size):
            m_buffer(buffer),
            m_count(size)
		{
		}
		
		inline int getCount() const { return m_count; }

		inline const T& operator [](int idx) const
		{
            SLANG_ASSERT(idx >= 0 && idx <= m_count);
            return m_buffer[idx];
		}
        inline T& operator [](int idx)
        {
            SLANG_ASSERT(idx >= 0 && idx <= m_count);
            return m_buffer[idx];
        }

        inline const T* getBuffer() const { return m_buffer; }
        inline T* getBuffer() { return m_buffer; }

		template<typename T2>
		int indexOf(const T2 & val) const
		{
			for (int i = 0; i < m_count; i++)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return -1;
		}

		template<typename T2>
		int lastIndexOf(const T2 & val) const
		{
			for (int i = m_count - 1; i >= 0; i--)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return -1;
		}

		template<typename Func>
		int findFirstIndex(const Func& predicate) const
		{
			for (int i = 0; i < m_count; i++)
			{
				if (predicate(m_buffer[i]))
					return i;
			}
			return -1;
		}

		template<typename Func>
		int findLastIndex(const Func& predicate) const
		{
			for (int i = m_count - 1; i >= 0; i--)
			{
				if (predicate(m_buffer[i]))
					return i;
			}
			return -1;
		}
	};

	template<typename T>
	ArrayView<T> makeArrayView(T& obj)
	{
		return ArrayView<T>(obj);
	}
		
	template<typename T>
	ArrayView<T> makeArrayView(T* buffer, int count)
	{
		return ArrayView<T>(buffer, count);
	}
}

#endif
