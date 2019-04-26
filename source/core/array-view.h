#ifndef CORE_LIB_ARRAY_VIEW_H
#define CORE_LIB_ARRAY_VIEW_H

#include "common.h"

namespace Slang
{
	template<typename T>
	class ArrayView
	{
	private:
		T* m_buffer;
		int m_size;
	public:
        const T* begin() const { return m_buffer; }
        T* begin() { return m_buffer; }

        const T* end() const { return m_buffer + m_size; }
        T* end() { return m_buffer + m_size; }
        
	public:
		ArrayView():
			m_buffer(nullptr),
			m_size(0)
        {
		}
		ArrayView(T& singleObj):
            m_buffer(&singleObj),
            m_size(1)
        {
		}
		ArrayView(T* buffer, int size):
            m_buffer(buffer),
            m_size(size)
		{
		}
		
		inline int getSize() const
		{
			return m_size;
		}

		inline const T& operator [](int idx) const
		{
            SLANG_ASSERT(idx >= 0 && idx <= m_size);
            return m_buffer[idx];
		}
        inline T& operator [](int idx)
        {
            SLANG_ASSERT(idx >= 0 && idx <= m_size);
            return m_buffer[idx];
        }

        inline const T* Buffer() const {return m_buffer; }
        inline T* Buffer() { return m_buffer; }

		template<typename T2>
		int indexOf(const T2 & val) const
		{
			for (int i = 0; i < m_size; i++)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return -1;
		}

		template<typename T2>
		int lastIndexOf(const T2 & val) const
		{
			for (int i = m_size - 1; i >= 0; i--)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return -1;
		}

		template<typename Func>
		int findFirstIndex(const Func& predicate) const
		{
			for (int i = 0; i < m_size; i++)
			{
				if (predicate(m_buffer[i]))
					return i;
			}
			return -1;
		}

		template<typename Func>
		int findLastIndex(const Func& predicate) const
		{
			for (int i = m_size - 1; i >= 0; i--)
			{
				if (predicate(m_buffer[i]))
					return i;
			}
			return -1;
		}
	};

	template<typename T>
	ArrayView<T> MakeArrayView(T& obj)
	{
		return ArrayView<T>(obj);
	}
		
	template<typename T>
	ArrayView<T> MakeArrayView(T* buffer, int count)
	{
		return ArrayView<T>(buffer, count);
	}
}

#endif
