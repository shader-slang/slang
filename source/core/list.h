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
		static void initialize(T* buffer, int size)
		{
			for (int i = 0; i<size; i++)
				new (buffer + i) T();
		}
	};
    template<typename T>
    class Initializer<T, 1>
    {
    public:
        static void initialize(T* buffer, int size)
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
			Initializer<T, std::is_pod<T>::value>::initialize(rs, size);
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
	public:
		List()
			: m_buffer(nullptr), m_size(0), m_capacity(0)
		{
		}
		template<typename... Args>
		List(const T& val, Args... args)
		{
			_init(val, args...);
		}
		List(const List<T>& list)
			: m_buffer(nullptr), m_size(0), m_capacity(0)
		{
			this->operator=(list);
		}
		List(List<T>&& list)
			: m_buffer(nullptr), m_size(0), m_capacity(0)
		{
			this->operator=(static_cast<List<T>&&>(list));
		}
		static List<T> makeRepeated(const T& val, int count)
		{
			List<T> rs;
			rs.setSize(count);
			for (int i = 0; i < count; i++)
				rs[i] = val;
			return rs;
		}
		~List()
		{
            _deallocateBuffer();
		}
		List<T>& operator=(const List<T>& list)
		{
			clearAndDeallocate();
			addRange(list);
			return *this;
		}

		List<T>& operator=(List<T>&& list)
		{
            // Could just do a swap here, and memory would be freed on rhs dtor

            _deallocateBuffer();
			m_size = list.m_size;
			m_capacity = list.m_capacity;
			m_buffer = list.m_buffer;

			list.m_buffer = nullptr;
			list.m_size = 0;
			list.m_capacity = 0;
			return *this;
		}

        T* begin() const
        {
            return m_buffer;
        }
        T* end() const
        {
            return m_buffer + m_size;
        }

		const T& getFirst() const
		{
            SLANG_ASSERT(m_size > 0);
			return m_buffer[0];
		}

		const T& getLast() const
		{
            SLANG_ASSERT(m_size > 0);
			return m_buffer[m_size-1];
		}

        T& getFirst()
        {
            SLANG_ASSERT(m_size > 0);
            return m_buffer[0];
        }

        T& getLast() 
        {
            SLANG_ASSERT(m_size > 0);
            return m_buffer[m_size - 1];
        }

        void removeLast()
        {
            SLANG_ASSERT(m_size > 0);
            m_size--;
        }

		inline void swapWith(List<T, TAllocator>& other)
		{
			T* buffer = m_buffer;
			m_buffer = other.m_buffer;
			other.m_buffer = buffer;

			auto bufferSize = m_capacity;
			m_capacity = other.m_capacity;
			other.m_capacity = bufferSize;

			auto count = m_size;
			m_size = other.m_size;
			other.m_size = count;
		}

		T* detachBuffer()
		{
			T* rs = m_buffer;
			m_buffer = nullptr;
			m_size = 0;
			m_capacity = 0;
			return rs;
		}

		inline ArrayView<T> getArrayView() const
		{
			return ArrayView<T>(m_buffer, m_size);
		}

		inline ArrayView<T> getArrayView(int start, int count) const
		{
            SLANG_ASSERT(start >= 0 && count >= 0 && start + count <= m_size);
			return ArrayView<T>(m_buffer + start, count);
		}

		void add(T&& obj)
		{
			if (m_capacity < m_size + 1)
			{
				UInt newBufferSize = kInitialSize;
				if (m_capacity)
					newBufferSize = (m_capacity << 1);

				reserve(newBufferSize);
			}
			m_buffer[m_size++] = static_cast<T&&>(obj);
		}

		void add(const T& obj)
		{
			if (m_capacity < m_size + 1)
			{
				UInt newBufferSize = kInitialSize;
				if (m_capacity)
					newBufferSize = (m_capacity << 1);

				reserve(newBufferSize);
			}
			m_buffer[m_size++] = obj;

		}

		UInt getSize() const
		{
			return m_size;
		}

		T* Buffer() const
		{
			return m_buffer;
		}

		UInt getCapacity() const
		{
			return m_capacity;
		}

		void insert(UInt id, const T& val)
		{
			insertRange(id, &val, 1);
		}

		void insertRange(UInt id, const T* vals, UInt n)
		{
			if (m_capacity < m_size + n)
			{
				UInt newBufferSize = kInitialSize;
				while (newBufferSize < m_size + n)
					newBufferSize = newBufferSize << 1;

				T * newBuffer = _allocate(newBufferSize);
				if (m_capacity)
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
						for (UInt i = id; i < m_size; i++)
							newBuffer[i + n] = T(static_cast<T&&>(m_buffer[i]));
					}
					_deallocateBuffer();
				}
				m_buffer = newBuffer;
				m_capacity = newBufferSize;
			}
			else
			{
				/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
					memmove(buffer + id + n, buffer + id, sizeof(T) * (_count - id));
				else*/
				{
					for (UInt i = m_size; i > id; i--)
						m_buffer[i + n - 1] = static_cast<T&&>(m_buffer[i - 1]);
				}
			}
			/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
				memcpy(buffer + id, vals, sizeof(T) * n);
			else*/
				for (UInt i = 0; i < n; i++)
					m_buffer[id + i] = vals[i];

			m_size += n;
		}

		//slower than original edition
		//void Add(const T & val)
		//{
		//	InsertRange(_count, &val, 1);
		//}

		void insertRange(int id, const List<T>& list)
		{
			insertRange(id, list.m_buffer, list.m_size);
		}

		void addRange(ArrayView<T> list)
		{
			insertRange(m_size, list.Buffer(), list.Count());
		}

		void addRange(const T* vals, UInt n)
		{
			insertRange(m_size, vals, n);
		}

		void addRange(const List<T>& list)
		{
			insertRange(m_size, list.m_buffer, list.m_size);
		}

		void removeRange(UInt idx, UInt count)
		{
            SLANG_ASSERT(idx >= 0 && idx <= m_size);

			const UInt actualDeleteCount = ((idx + count) >= m_size)? (m_size - idx) : count;
			for (UInt i = idx + actualDeleteCount; i < m_size; i++)
				m_buffer[i - actualDeleteCount] = static_cast<T&&>(m_buffer[i]);
			m_size -= actualDeleteCount;
		}

		void removeAt(UInt id)
		{
			removeRange(id, 1);
		}

		void remove(const T& val)
		{
			int idx = indexOf(val);
			if (idx != -1)
				removeAt(idx);
		}

		void reverse()
		{
			for (UInt i = 0; i < (m_size >> 1); i++)
			{
				swapElements(m_buffer, i, m_size - i - 1);
			}
		}

		void fastRemove(const T& val)
		{
			int idx = indexOf(val);
			fastRemoveAt(idx);
		}

		void fastRemoveAt(UInt idx)
		{
			if (idx != -1 && m_size - 1 != idx)
			{
				m_buffer[idx] = _Move(m_buffer[m_size - 1]);
			}
			m_size--;
		}

		void clear()
		{
			m_size = 0;
		}

        void clearAndDeallocate()
        {
            _deallocateBuffer();
            m_size = m_capacity = 0;
        }

		void reserve(UInt size)
		{
			if(size > m_capacity)
			{
				T* newBuffer = _allocate(size);
				if (m_capacity)
				{
					/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
						memcpy(newBuffer, buffer, _count * sizeof(T));
					else*/
					{
						for (UInt i = 0; i < m_size; i++)
							newBuffer[i] = static_cast<T&&>(m_buffer[i]);

                        // Default-initialize the remaining elements
                        for(UInt i = m_size; i < size; i++)
                        {
                            new(newBuffer + i) T();
                        }
					}
					_deallocateBuffer();
				}
				m_buffer = newBuffer;
				m_capacity = size;
			}
		}

		void growToSize(UInt size)
		{
			UInt newBufferSize = UInt(1) << Math::Log2Ceil(size);
			if (m_capacity < newBufferSize)
			{
				reserve(newBufferSize);
			}
			m_size = size;
		}

		void setSize(UInt size)
		{
			reserve(size);
			m_size = size;
		}

		void unsafeShrinkToSize(UInt size)
		{
			m_size = size;
		}

		void compress()
		{
			if (m_capacity > m_size && m_size > 0)
			{
				T* newBuffer = _allocate(m_size);
				for (UInt i = 0; i < m_size; i++)
					newBuffer[i] = static_cast<T&&>(m_buffer[i]);

				_deallocateBuffer();
				m_buffer = newBuffer;
				m_capacity = m_size;
			}
		}

		SLANG_FORCE_INLINE T& operator [](UInt idx) const
		{
            SLANG_ASSERT(idx >= 0 && idx <= m_size);
			return m_buffer[idx];
		}

		template<typename Func>
		UInt findFirstIndex(const Func& predicate) const
		{
			for (UInt i = 0; i < m_size; i++)
			{
				if (predicate(m_buffer[i]))
					return i;
			}
			return (UInt)-1;
		}

		template<typename Func>
		UInt findLastIndex(const Func& predicate) const
		{
			for (UInt i = m_size; i > 0; i--)
			{
				if (predicate(m_buffer[i-1]))
					return i-1;
			}
			return (UInt)-1;
		}

		template<typename T2>
		UInt indexOf(const T2& val) const
		{
			for (UInt i = 0; i < m_size; i++)
			{
				if (m_buffer[i] == val)
					return i;
			}
			return (UInt)-1;
		}

		template<typename T2>
		UInt lastIndexOf(const T2& val) const
		{
			for (int i = m_size; i > 0; i--)
			{
				if(m_buffer[i-1] == val)
					return i-1;
			}
			return (UInt)-1;
		}

		void sort()
		{
			sort([](const T& t1, const T& t2){return t1 < t2;});
		}

		bool contains(const T& val) const
		{
            return indexOf(val) != UInt(-1);
		}

		template<typename Comparer>
		void sort(Comparer compare)
		{
			//insertionSort(buffer, 0, _count - 1);
			//quickSort(buffer, 0, _count - 1, compare);
			std::sort(m_buffer, m_buffer + m_size, compare);
		}

		template <typename IterateFunc>
		void forEach(IterateFunc f) const
		{
			for (int i = 0; i< m_size; i++)
				f(m_buffer[i]);
		}

		template<typename Comparer>
		void quickSort(T* vals, int startIndex, int endIndex, Comparer comparer)
		{
			if(startIndex < endIndex)
			{
				if (endIndex - startIndex < MIN_QSORT_SIZE)
					insertionSort(vals, startIndex, endIndex, comparer);
				else
				{
					int pivotIndex = (startIndex + endIndex) >> 1;
					int pivotNewIndex = partition(vals, startIndex, endIndex, pivotIndex, comparer);
					quickSort(vals, startIndex, pivotNewIndex - 1, comparer);
					quickSort(vals, pivotNewIndex + 1, endIndex, comparer);
				}
			}

		}
		template<typename Comparer>
		int partition(T* vals, int left, int right, int pivotIndex, Comparer comparer)
		{
			T pivotValue = vals[pivotIndex];
			swapElements(vals, right, pivotIndex);
			int storeIndex = left;
			for (int i = left; i < right; i++)
			{
				if (comparer(vals[i], pivotValue))
				{
					swapElements(vals, i, storeIndex);
					storeIndex++;
				}
			}
			swapElements(vals, storeIndex, right);
			return storeIndex;
		}
		template<typename Comparer>
		void insertionSort(T* vals, int startIndex, int endIndex, Comparer comparer)
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

		inline void swapElements(T* vals, int index1, int index2)
		{
			if (index1 != index2)
			{
				T tmp = static_cast<T&&>(vals[index1]);
				vals[index1] = static_cast<T&&>(vals[index2]);
				vals[index2] = static_cast<T&&>(tmp);
			}
		}

		template<typename T2, typename Comparer>
		int binarySearch(const T2& obj, Comparer comparer)
		{
			int imin = 0, imax = m_size - 1;
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
		int binarySearch(const T2& obj)
		{
			return binarySearch(obj, 
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
    private:
        static const Int kInitialSize = 16;

        T*      m_buffer;           ///< A new T[N] allocated buffer. NOTE! All elements up to capacity are in some valid form for T.
        UInt    m_capacity;         ///< The total capacity of elements
        UInt    m_size;             ///< The amount of elements

        void _deallocateBuffer()
        {
            if (m_buffer)
            {
                AllocateMethod<T, TAllocator>::Free(m_buffer, m_capacity);
                m_buffer = nullptr;
            }
        }
        static inline T* _allocate(UInt size)
        {
            return AllocateMethod<T, TAllocator>::Alloc(size);
        }

        template<typename... Args>
        void _init(const T& val, Args... args)
        {
            add(val);
            _init(args...);
        }
	};

	template<typename T>
	T calcMin(const List<T>& list)
	{
		T minVal = list.getFirst();
		for (int i = 1; i < list.getSize(); i++)
			if (list[i] < minVal)
				minVal = list[i];
		return minVal;
	}

	template<typename T>
	T calcMax(const List<T>& list)
	{
		T maxVal = list.getFirst();
		for (int i = 1; i< list.getSize(); i++)
			if (list[i] > maxVal)
				maxVal = list[i];
		return maxVal;
	}
}

#endif
