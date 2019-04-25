#ifndef FUNDAMENTAL_LIB_LIST_H
#define FUNDAMENTAL_LIB_LIST_H

#include "../../slang.h"

#include "allocator.h"
#include "slang-math.h"
#include "array-view.h"

#include <algorithm>
#include <new>
#include <type_traits>


namespace Slang
{
    const int MIN_QSORT_SIZE = 32;

	template<typename T, int isPOD>
	class Initializer
	{

	};

	template<typename T>
	class Initializer<T, 0>
	{
	public:
		static void Initialize(T* buffer, int size)
		{
			for (int i = 0; i<size; i++)
				new (buffer + i) T();
		}
	};
    template<typename T>
    class Initializer<T, 1>
    {
    public:
        static void Initialize(T* buffer, int size)
        {
            // It's pod so no initialization required
            //for (int i = 0; i < size; i++)
            //    new (buffer + i) T;
        }
    };

	template<typename T, typename TAllocator>
	class AllocateMethod
	{
	public:
		static inline T* Alloc(UInt size)
		{
			TAllocator allocator;
			T * rs = (T*)allocator.Alloc(size*sizeof(T));
			Initializer<T, std::is_pod<T>::value>::Initialize(rs, size);
			return rs;
		}
		static inline void Free(T* ptr, UInt bufferSize)
		{
			TAllocator allocator;
			if (!std::is_trivially_destructible<T>::value)
			{
				for (UInt i = 0; i<bufferSize; i++)
					ptr[i].~T();
			}
			allocator.Free(ptr);
		}
	};

	template<typename T>
	class AllocateMethod<T, StandardAllocator>
	{
	public:
		static inline T* Alloc(UInt size)
		{
			return new T[size];
		}
		static inline void Free(T* ptr, UInt /*bufferSize*/)
		{
			delete [] ptr;
		}
	};


	template<typename T, typename TAllocator = StandardAllocator>
	class List
	{
	private:
		inline T* Allocate(UInt size)
		{
			return AllocateMethod<T, TAllocator>::Alloc(size);				
		}
	private:
		static const Int kInitialSize = 16;
		TAllocator m_allocator;
	private:
		T*     m_buffer;
        UInt    m_bufferSize;
        UInt    m_count;
		void FreeBuffer()
		{
			AllocateMethod<T, TAllocator>::Free(m_buffer, m_bufferSize);
			m_buffer = 0;
		}
		void Free()
		{
			if (m_buffer)
			{
				FreeBuffer();
			}
			m_buffer = 0;
			m_count = m_bufferSize = 0;
		}
	public:
		T* begin() const
		{
			return m_buffer;
		}
		T* end() const
		{
			return m_buffer+m_count;
		}
	private:
		template<typename... Args>
		void Init(const T& val, Args... args)
		{
			Add(val);
			Init(args...);
		}
	public:
		List()
			: m_buffer(nullptr), m_count(0), m_bufferSize(0)
		{
		}
		template<typename... Args>
		List(const T& val, Args... args)
		{
			Init(val, args...);
		}
		List(const List<T> & list)
			: m_buffer(nullptr), m_count(0), m_bufferSize(0)
		{
			this->operator=(list);
		}
		List(List<T> && list)
			: m_buffer(nullptr), m_count(0), m_bufferSize(0)
		{
			this->operator=(static_cast<List<T>&&>(list));
		}
		static List<T> Create(const T& val, int count)
		{
			List<T> rs;
			rs.SetSize(count);
			for (int i = 0; i < count; i++)
				rs[i] = val;
			return rs;
		}
		~List()
		{
			Free();
		}
		List<T>& operator=(const List<T>& list)
		{
			Free();
			AddRange(list);

			return *this;
		}

		List<T>& operator=(List<T>&& list)
		{
			Free();
			m_count = list.m_count;
			m_bufferSize = list.m_bufferSize;
			m_buffer = list.m_buffer;

			list.m_buffer = nullptr;
			list.m_count = 0;
			list.m_bufferSize = 0;
			return *this;
		}

		T& First() const
		{
            SLANG_ASSERT(m_count > 0);
			return m_buffer[0];
		}

		T& Last() const
		{
            SLANG_ASSERT(m_count > 0);
			return m_buffer[m_count-1];
		}

        void RemoveLast()
        {
            SLANG_ASSERT(m_count > 0);
            m_count--;
        }

		inline void SwapWith(List<T, TAllocator>& other)
		{
			T* buffer = m_buffer;
			m_buffer = other.m_buffer;
			other.m_buffer = buffer;

			auto bufferSize = m_bufferSize;
			m_bufferSize = other.m_bufferSize;
			other.m_bufferSize = bufferSize;

			auto count = m_count;
			m_count = other.m_count;
			other.m_count = count;

			TAllocator tmpAlloc = _Move(m_allocator);
			m_allocator = _Move(other.m_allocator);
			other.m_allocator = _Move(tmpAlloc);
		}

		T* ReleaseBuffer()
		{
			T* rs = m_buffer;
			m_buffer = nullptr;
			m_count = 0;
			m_bufferSize = 0;
			return rs;
		}

		inline ArrayView<T> GetArrayView() const
		{
			return ArrayView<T>(m_buffer, m_count);
		}

		inline ArrayView<T> GetArrayView(int start, int count) const
		{
            SLANG_ASSERT(start >= 0 && count >= 0 && start + count <= m_count);
			return ArrayView<T>(m_buffer + start, count);
		}

		void Add(T&& obj)
		{
			if (m_bufferSize < m_count + 1)
			{
				UInt newBufferSize = kInitialSize;
				if (m_bufferSize)
					newBufferSize = (m_bufferSize << 1);

				Reserve(newBufferSize);
			}
			m_buffer[m_count++] = static_cast<T&&>(obj);
		}

		void Add(const T& obj)
		{
			if (m_bufferSize < m_count + 1)
			{
				UInt newBufferSize = kInitialSize;
				if (m_bufferSize)
					newBufferSize = (m_bufferSize << 1);

				Reserve(newBufferSize);
			}
			m_buffer[m_count++] = obj;

		}

		UInt Count() const
		{
			return m_count;
		}

		T* Buffer() const
		{
			return m_buffer;
		}

		UInt Capacity() const
		{
			return m_bufferSize;
		}

		void Insert(UInt id, const T& val)
		{
			InsertRange(id, &val, 1);
		}

		void InsertRange(UInt id, const T* vals, UInt n)
		{
			if (m_bufferSize < m_count + n)
			{
				UInt newBufferSize = kInitialSize;
				while (newBufferSize < m_count + n)
					newBufferSize = newBufferSize << 1;

				T * newBuffer = Allocate(newBufferSize);
				if (m_bufferSize)
				{
					/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
					{
						memcpy(newBuffer, buffer, sizeof(T) * id);
						memcpy(newBuffer + id + n, buffer + id, sizeof(T) * (_count - id));
					}
					else*/
					{
						for (UInt i = 0; i < id; i++)
							newBuffer[i] = m_buffer[i];
						for (UInt i = id; i < m_count; i++)
							newBuffer[i + n] = T(static_cast<T&&>(m_buffer[i]));
					}
					FreeBuffer();
				}
				m_buffer = newBuffer;
				m_bufferSize = newBufferSize;
			}
			else
			{
				/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
					memmove(buffer + id + n, buffer + id, sizeof(T) * (_count - id));
				else*/
				{
					for (UInt i = m_count; i > id; i--)
						m_buffer[i + n - 1] = static_cast<T&&>(m_buffer[i - 1]);
				}
			}
			/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
				memcpy(buffer + id, vals, sizeof(T) * n);
			else*/
				for (UInt i = 0; i < n; i++)
					m_buffer[id + i] = vals[i];

			m_count += n;
		}

		//slower than original edition
		//void Add(const T & val)
		//{
		//	InsertRange(_count, &val, 1);
		//}

		void InsertRange(int id, const List<T>& list)
		{
			InsertRange(id, list.m_buffer, list.m_count);
		}

		void AddRange(ArrayView<T> list)
		{
			InsertRange(m_count, list.Buffer(), list.Count());
		}

		void AddRange(const T* vals, UInt n)
		{
			InsertRange(m_count, vals, n);
		}

		void AddRange(const List<T>& list)
		{
			InsertRange(m_count, list.m_buffer, list.m_count);
		}

		void RemoveRange(UInt idx, UInt count)
		{
            SLANG_ASSERT(idx >= 0 && idx <= m_count);

			const UInt actualDeleteCount = ((idx + count) >= m_count)? (m_count - idx) : count;
			for (UInt i = idx + actualDeleteCount; i < m_count; i++)
				m_buffer[i - actualDeleteCount] = static_cast<T&&>(m_buffer[i]);
			m_count -= actualDeleteCount;
		}

		void RemoveAt(UInt id)
		{
			RemoveRange(id, 1);
		}

		void Remove(const T& val)
		{
			int idx = IndexOf(val);
			if (idx != -1)
				RemoveAt(idx);
		}

		void Reverse()
		{
			for (UInt i = 0; i < (m_count >> 1); i++)
			{
				Swap(m_buffer, i, m_count - i - 1);
			}
		}

		void FastRemove(const T& val)
		{
			int idx = IndexOf(val);
			FastRemoveAt(idx);
		}

		void FastRemoveAt(UInt idx)
		{
			if (idx != -1 && m_count - 1 != idx)
			{
				m_buffer[idx] = _Move(m_buffer[m_count - 1]);
			}
			m_count--;
		}

		void Clear()
		{
			m_count = 0;
		}

		void Reserve(UInt size)
		{
			if(size > m_bufferSize)
			{
				T * newBuffer = Allocate(size);
				if (m_bufferSize)
				{
					/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
						memcpy(newBuffer, buffer, _count * sizeof(T));
					else*/
					{
						for (UInt i = 0; i < m_count; i++)
							newBuffer[i] = static_cast<T&&>(m_buffer[i]);

                        // Default-initialize the remaining elements
                        for(UInt i = m_count; i < size; i++)
                        {
                            new(newBuffer + i) T();
                        }
					}
					FreeBuffer();
				}
				m_buffer = newBuffer;
				m_bufferSize = size;
			}
		}

		void GrowToSize(UInt size)
		{
			UInt newBufferSize = UInt(1) << Math::Log2Ceil(size);
			if (m_bufferSize < newBufferSize)
			{
				Reserve(newBufferSize);
			}
			m_count = size;
		}

		void SetSize(UInt size)
		{
			Reserve(size);
			m_count = size;
		}

		void UnsafeShrinkToSize(UInt size)
		{
			m_count = size;
		}

		void Compress()
		{
			if (m_bufferSize > m_count && m_count > 0)
			{
				T* newBuffer = Allocate(m_count);
				for (UInt i = 0; i < m_count; i++)
					newBuffer[i] = static_cast<T&&>(m_buffer[i]);
				FreeBuffer();
				m_buffer = newBuffer;
				m_bufferSize = m_count;
			}
		}

		SLANG_FORCE_INLINE T& operator [](UInt idx) const
		{
            SLANG_ASSERT(idx >= 0 && idx <= m_count);
			return m_buffer[idx];
		}

		template<typename Func>
		UInt FindFirst(const Func& predicate) const
		{
			for (UInt i = 0; i < m_count; i++)
			{
				if (predicate(m_buffer[i]))
					return i;
			}
			return (UInt)-1;
		}

		template<typename Func>
		UInt FindLast(const Func& predicate) const
		{
			for (UInt i = m_count; i > 0; i--)
			{
				if (predicate(m_buffer[i-1]))
					return i-1;
			}
			return (UInt)-1;
		}

		template<typename T2>
		UInt IndexOf(const T2& val) const
		{
			for (UInt i = 0; i < m_count; i++)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return (UInt)-1;
		}

		template<typename T2>
		UInt LastIndexOf(const T2& val) const
		{
			for (int i = m_count; i > 0; i--)
			{
				if(m_buffer[i-1] == val)
					return i-1;
			}
			return (UInt)-1;
		}

		void Sort()
		{
			Sort([](const T& t1, const T& t2){return t1 < t2;});
		}

		bool Contains(const T& val)
		{
			for (UInt i = 0; i< m_count; i++)
				if (m_buffer[i] == val)
					return true;
			return false;
		}

		template<typename Comparer>
		void Sort(Comparer compare)
		{
			//InsertionSort(buffer, 0, _count - 1);
			//QuickSort(buffer, 0, _count - 1, compare);
			std::sort(m_buffer, m_buffer + m_count, compare);
		}

		template <typename IterateFunc>
		void ForEach(IterateFunc f) const
		{
			for (int i = 0; i< m_count; i++)
				f(m_buffer[i]);
		}

		template<typename Comparer>
		void QuickSort(T* vals, int startIndex, int endIndex, Comparer comparer)
		{
			if(startIndex < endIndex)
			{
				if (endIndex - startIndex < MIN_QSORT_SIZE)
					InsertionSort(vals, startIndex, endIndex, comparer);
				else
				{
					int pivotIndex = (startIndex + endIndex) >> 1;
					int pivotNewIndex = Partition(vals, startIndex, endIndex, pivotIndex, comparer);
					QuickSort(vals, startIndex, pivotNewIndex - 1, comparer);
					QuickSort(vals, pivotNewIndex + 1, endIndex, comparer);
				}
			}

		}
		template<typename Comparer>
		int Partition(T* vals, int left, int right, int pivotIndex, Comparer comparer)
		{
			T pivotValue = vals[pivotIndex];
			Swap(vals, right, pivotIndex);
			int storeIndex = left;
			for (int i = left; i < right; i++)
			{
				if (comparer(vals[i], pivotValue))
				{
					Swap(vals, i, storeIndex);
					storeIndex++;
				}
			}
			Swap(vals, storeIndex, right);
			return storeIndex;
		}
		template<typename Comparer>
		void InsertionSort(T* vals, int startIndex, int endIndex, Comparer comparer)
		{
			for (int i = startIndex  + 1; i <= endIndex; i++)
			{
				T insertValue = static_cast<T&&>(vals[i]);
				int insertIndex = i - 1;
				while (insertIndex >= startIndex && comparer(insertValue, vals[insertIndex]))
				{
					vals[insertIndex + 1] = static_cast<T&&>(vals[insertIndex]);
					insertIndex--;
				}
				vals[insertIndex + 1] = static_cast<T&&>(insertValue);
			}
		}

		inline void Swap(T* vals, int index1, int index2)
		{
			if (index1 != index2)
			{
				T tmp = static_cast<T&&>(vals[index1]);
				vals[index1] = static_cast<T&&>(vals[index2]);
				vals[index2] = static_cast<T&&>(tmp);
			}
		}

		template<typename T2, typename Comparer>
		int BinarySearch(const T2& obj, Comparer comparer)
		{
			int imin = 0, imax = m_count - 1;
			while (imax >= imin)
			{
				int imid = (imin + imax) >> 1;
				int compareResult = comparer(m_buffer[imid], obj);
				if (compareResult == 0)
					return imid;
				else if (compareResult < 0)
					imin = imid + 1;
				else
					imax = imid - 1;
			}
			return -1;
		}

		template<typename T2>
		int BinarySearch(const T2& obj)
		{
			return BinarySearch(obj, 
				[](T & curObj, const T2 & thatObj)->int
				{
					if (curObj < thatObj)
						return -1;
					else if (curObj == thatObj)
						return 0;
					else
						return 1;
				});
		}
	};

	template<typename T>
	T Min(const List<T>& list)
	{
		T minVal = list.First();
		for (int i = 1; i < list.Count(); i++)
			if (list[i] < minVal)
				minVal = list[i];
		return minVal;
	}

	template<typename T>
	T Max(const List<T>& list)
	{
		T maxVal = list.First();
		for (int i = 1; i< list.Count(); i++)
			if (list[i] > maxVal)
				maxVal = list[i];
		return maxVal;
	}
}

#endif
