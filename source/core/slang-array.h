#ifndef SLANG_CORE_ARRAY_H
#define SLANG_CORE_ARRAY_H

#include "slang-exception.h"
#include "slang-array-view.h"

namespace Slang
{
	template<typename T, int COUNT>
	class Array
	{
	private:
		T m_buffer[COUNT];
		int m_count = 0;
	public:
        T* begin() { return m_buffer; }
        const T* begin() const { return m_buffer; }

        const T* end() const { return m_buffer + m_count; }
        T* end() { return m_buffer + m_count; }

	public:
		inline int getCapacity() const { return COUNT; }
		inline int getCount() const { return m_count; }
		inline const T& getFirst() const
		{
            SLANG_ASSERT(m_count > 0);
			return m_buffer[0];
		}
        inline T& getFirst()
        {
            SLANG_ASSERT(m_count > 0);
            return m_buffer[0];
        }
		inline const T& getLast() const
		{
            SLANG_ASSERT(m_count > 0);
			return m_buffer[m_count - 1];
		}
        inline T& getLast()
        {
            SLANG_ASSERT(m_count > 0);
            return m_buffer[m_count - 1];
        }
		inline void setCount(int newCount)
		{
            SLANG_ASSERT(newCount >= 0 && newCount <= COUNT);
			m_count = newCount;
		}
		inline void add(const T & item)
		{
            SLANG_ASSERT(m_count < COUNT);
			m_buffer[m_count++] = item;
		}
		inline void add(T && item)
		{
            SLANG_ASSERT(m_count < COUNT);
			m_buffer[m_count++] = _Move(item);
		}

		inline const T& operator [](int idx) const
		{
            SLANG_ASSERT(idx >= 0 && idx < m_count);
			return m_buffer[idx];
		}
        inline T& operator [](int idx)
        {
            SLANG_ASSERT(idx >= 0 && idx < m_count);
            return m_buffer[idx];
        }

		inline const T* getBuffer() const { return m_buffer; }
        inline T* getBuffer() { return m_buffer; }

		inline void clear() { m_count = 0; }

		template<typename T2>
		int indexOf(const T2& val) const
		{
			for (int i = 0; i < m_count; i++)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return -1;
		}

		template<typename T2>
		int lastIndexOf(const T2& val) const
		{
			for (int i = m_count - 1; i >= 0; i--)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return -1;
		}

		inline ArrayView<T> getArrayView() const
		{
			return ArrayView<T>((T*)m_buffer, m_count);
		}
		inline ArrayView<T> getArrayView(int start, int count) const
		{
			return ArrayView<T>((T*)m_buffer + start, count);
		}
	};

	template<typename T, typename ...TArgs>
	struct FirstType
	{
		typedef T Type;
	};


	template<typename T, int SIZE>
	void insertArray(Array<T, SIZE>&) {}

	template<typename T, typename ...TArgs, int SIZE>
	void insertArray(Array<T, SIZE>& arr, const T& val, TArgs... args)
	{
		arr.add(val);
		insertArray(arr, args...);
	}

	template<typename ...TArgs>
	auto makeArray(TArgs ...args) -> Array<typename FirstType<TArgs...>::Type, sizeof...(args)>
	{
		Array<typename FirstType<TArgs...>::Type, sizeof...(args)> rs;
		insertArray(rs, args...);
		return rs;
	}
}

#endif
