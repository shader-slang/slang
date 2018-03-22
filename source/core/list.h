#ifndef FUNDAMENTAL_LIB_LIST_H
#define FUNDAMENTAL_LIB_LIST_H

#include "allocator.h"
#include "slang-math.h"
#include "array-view.h"
#include "slang-defines.h"

#include <algorithm>
#include <new>
#include <type_traits>

const int MIN_QSORT_SIZE = 32;

namespace Slang
{
	template<typename T, int isPOD>
	class Initializer
	{

	};

	template<typename T>
	class Initializer<T, 0>
	{
	public:
		static void Initialize(T * buffer, int size)
		{
			for (int i = 0; i<size; i++)
				new (buffer + i) T();
		}
	};
    template<typename T>
    class Initializer<T, 1>
    {
    public:
        static void Initialize(T * buffer, int size)
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
		static inline void Free(T * ptr, UInt bufferSize)
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

		inline T * Allocate(UInt size)
		{
			return AllocateMethod<T, TAllocator>::Alloc(size);
				
		}
	private:
		static const int InitialSize = 16;
		TAllocator allocator;
	private:
		T*      buffer;
		UInt    _count;
		UInt    bufferSize;
		void FreeBuffer()
		{
			AllocateMethod<T, TAllocator>::Free(buffer, bufferSize);
			buffer = 0;
		}
		void Free()
		{
			if (buffer)
			{
				FreeBuffer();
			}
			buffer = 0;
			_count = bufferSize = 0;
		}
	public:
		T* begin() const
		{
			return buffer;
		}
		T* end() const
		{
			return buffer+_count;
		}
	private:
		template<typename... Args>
		void Init(const T & val, Args... args)
		{
			Add(val);
			Init(args...);
		}
	public:
		List()
			: buffer(0), _count(0), bufferSize(0)
		{
		}
		template<typename... Args>
		List(const T & val, Args... args)
		{
			Init(val, args...);
		}
		List(const List<T> & list)
			: buffer(0), _count(0), bufferSize(0)
		{
			this->operator=(list);
		}
		List(List<T> && list)
			: buffer(0), _count(0), bufferSize(0)
		{
			this->operator=(static_cast<List<T>&&>(list));
		}
		static List<T> Create(const T & val, int count)
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
		List<T> & operator=(const List<T> & list)
		{
			Free();
			AddRange(list);

			return *this;
		}

		List<T> & operator=(List<T> && list)
		{
			Free();
			_count = list._count;
			bufferSize = list.bufferSize;
			buffer = list.buffer;

			list.buffer = 0;
			list._count = 0;
			list.bufferSize = 0;
			return *this;
		}

		T & First() const
		{
#ifdef _DEBUG
			if (_count == 0)
				throw "Index out of range.";
#endif
			return buffer[0];
		}

		T & Last() const
		{
#ifdef _DEBUG
			if (_count == 0)
				throw "Index out of range.";
#endif
			return buffer[_count-1];
		}

		inline void SwapWith(List<T, TAllocator> & other)
		{
			T* tmpBuffer = this->buffer;
			this->buffer = other.buffer;
			other.buffer = tmpBuffer;
			auto tmpBufferSize = this->bufferSize;
			this->bufferSize = other.bufferSize;
			other.bufferSize = tmpBufferSize;
			auto tmpCount = this->_count;
			this->_count = other._count;
			other._count = tmpCount;
			TAllocator tmpAlloc = _Move(this->allocator);
			this->allocator = _Move(other.allocator);
			other.allocator = _Move(tmpAlloc);
		}

		T* ReleaseBuffer()
		{
			T* rs = buffer;
			buffer = nullptr;
			_count = 0;
			bufferSize = 0;
			return rs;
		}

		inline ArrayView<T> GetArrayView() const
		{
			return ArrayView<T>(buffer, _count);
		}

		inline ArrayView<T> GetArrayView(int start, int count) const
		{
#ifdef _DEBUG
			if (start + count > _count || start < 0 || count < 0)
				throw "Index out of range.";
#endif
			return ArrayView<T>(buffer + start, count);
		}

		void Add(T && obj)
		{
			if (bufferSize < _count + 1)
			{
				UInt newBufferSize = InitialSize;
				if (bufferSize)
					newBufferSize = (bufferSize << 1);

				Reserve(newBufferSize);
			}
			buffer[_count++] = static_cast<T&&>(obj);
		}

		void Add(const T & obj)
		{
			if (bufferSize < _count + 1)
			{
				UInt newBufferSize = InitialSize;
				if (bufferSize)
					newBufferSize = (bufferSize << 1);

				Reserve(newBufferSize);
			}
			buffer[_count++] = obj;

		}

		UInt Count() const
		{
			return _count;
		}

		T * Buffer() const
		{
			return buffer;
		}

		UInt Capacity() const
		{
			return bufferSize;
		}

		void Insert(UInt id, const T & val)
		{
			InsertRange(id, &val, 1);
		}

		void InsertRange(UInt id, const T * vals, UInt n)
		{
			if (bufferSize < _count + n)
			{
				UInt newBufferSize = InitialSize;
				while (newBufferSize < _count + n)
					newBufferSize = newBufferSize << 1;

				T * newBuffer = Allocate(newBufferSize);
				if (bufferSize)
				{
					/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
					{
						memcpy(newBuffer, buffer, sizeof(T) * id);
						memcpy(newBuffer + id + n, buffer + id, sizeof(T) * (_count - id));
					}
					else*/
					{
						for (UInt i = 0; i < id; i++)
							newBuffer[i] = buffer[i];
						for (UInt i = id; i < _count; i++)
							newBuffer[i + n] = T(static_cast<T&&>(buffer[i]));
					}
					FreeBuffer();
				}
				buffer = newBuffer;
				bufferSize = newBufferSize;
			}
			else
			{
				/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
					memmove(buffer + id + n, buffer + id, sizeof(T) * (_count - id));
				else*/
				{
					for (UInt i = _count; i > id; i--)
						buffer[i + n - 1] = static_cast<T&&>(buffer[i - 1]);
				}
			}
			/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
				memcpy(buffer + id, vals, sizeof(T) * n);
			else*/
				for (UInt i = 0; i < n; i++)
					buffer[id + i] = vals[i];

			_count += n;
		}

		//slower than original edition
		//void Add(const T & val)
		//{
		//	InsertRange(_count, &val, 1);
		//}

		void InsertRange(int id, const List<T> & list)
		{
			InsertRange(id, list.buffer, list._count);
		}

		void AddRange(ArrayView<T> list)
		{
			InsertRange(_count, list.Buffer(), list.Count());
		}

		void AddRange(const T * vals, UInt n)
		{
			InsertRange(_count, vals, n);
		}

		void AddRange(const List<T> & list)
		{
			InsertRange(_count, list.buffer, list._count);
		}

		void RemoveRange(UInt id, UInt deleteCount)
		{
#if _DEBUG
			if (id >= _count)
				throw "Remove: Index out of range.";
#endif
			UInt actualDeleteCount = ((id + deleteCount) >= _count)? (_count - id) : deleteCount;
			for (UInt i = id + actualDeleteCount; i < _count; i++)
				buffer[i - actualDeleteCount] = static_cast<T&&>(buffer[i]);
			_count -= actualDeleteCount;
		}

		void RemoveAt(UInt id)
		{
			RemoveRange(id, 1);
		}

		void Remove(const T & val)
		{
			int idx = IndexOf(val);
			if (idx != -1)
				RemoveAt(idx);
		}

		void Reverse()
		{
			for (UInt i = 0; i < (_count >> 1); i++)
			{
				Swap(buffer, i, _count - i - 1);
			}
		}

		void FastRemove(const T & val)
		{
			int idx = IndexOf(val);
			FastRemoveAt(idx);
		}

		void FastRemoveAt(UInt idx)
		{
			if (idx != -1 && _count - 1 != idx)
			{
				buffer[idx] = _Move(buffer[_count - 1]);
			}
			_count--;
		}

		void Clear()
		{
			_count = 0;
		}

		void Reserve(UInt size)
		{
			if(size > bufferSize)
			{
				T * newBuffer = Allocate(size);
				if (bufferSize)
				{
					/*if (std::has_trivial_copy_assign<T>::value && std::has_trivial_destructor<T>::value)
						memcpy(newBuffer, buffer, _count * sizeof(T));
					else*/
					{
						for (UInt i = 0; i < _count; i++)
							newBuffer[i] = static_cast<T&&>(buffer[i]);

                        // Default-initialize the remaining elements
                        for(UInt i = _count; i < size; i++)
                        {
                            new(newBuffer + i) T();
                        }
					}
					FreeBuffer();
				}
				buffer = newBuffer;
				bufferSize = size;
			}
		}

		void GrowToSize(UInt size)
		{
			UInt newBufferSize = UInt(1) << Math::Log2Ceil(size);
			if (bufferSize < newBufferSize)
			{
				Reserve(newBufferSize);
			}
			this->_count = size;
		}

		void SetSize(UInt size)
		{
			Reserve(size);
			_count = size;
		}

		void UnsafeShrinkToSize(UInt size)
		{
			_count = size;
		}

		void Compress()
		{
			if (bufferSize > _count && _count > 0)
			{
				T * newBuffer = Allocate(_count);
				for (int i = 0; i < _count; i++)
					newBuffer[i] = static_cast<T&&>(buffer[i]);
				FreeBuffer();
				buffer = newBuffer;
				bufferSize = _count;
			}
		}

		SLANG_FORCE_INLINE T & operator [](UInt id) const
		{
#if _DEBUG
			if(id >= _count)
				throw IndexOutofRangeException("Operator[]: Index out of Range.");
#endif
			return buffer[id];
		}

		template<typename Func>
		UInt FindFirst(const Func & predicate) const
		{
			for (UInt i = 0; i < _count; i++)
			{
				if (predicate(buffer[i]))
					return i;
			}
			return (UInt)-1;
		}

		template<typename Func>
		UInt FindLast(const Func & predicate) const
		{
			for (UInt i = _count; i > 0; i--)
			{
				if (predicate(buffer[i-1]))
					return i-1;
			}
			return (UInt)-1;
		}

		template<typename T2>
		UInt IndexOf(const T2 & val) const
		{
			for (int i = 0; i < _count; i++)
			{
				if (buffer[i] == val)
					return i;
			}
			return (UInt)-1;
		}

		template<typename T2>
		UInt LastIndexOf(const T2 & val) const
		{
			for (int i = _count; i > 0; i--)
			{
				if(buffer[i-1] == val)
					return i-1;
			}
			return (UInt)-1;
		}

		void Sort()
		{
			Sort([](T const& t1, T const& t2){return t1<t2;});
		}

		bool Contains(const T & val)
		{
			for (UInt i = 0; i<_count; i++)
				if (buffer[i] == val)
					return true;
			return false;
		}

		template<typename Comparer>
		void Sort(Comparer compare)
		{
			//InsertionSort(buffer, 0, _count - 1);
			//QuickSort(buffer, 0, _count - 1, compare);
			std::sort(buffer, buffer + _count, compare);
		}

		template <typename IterateFunc>
		void ForEach(IterateFunc f) const
		{
			for (int i = 0; i<_count; i++)
				f(buffer[i]);
		}

		template<typename Comparer>
		void QuickSort(T * vals, int startIndex, int endIndex, Comparer comparer)
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
		int Partition(T * vals, int left, int right, int pivotIndex, Comparer comparer)
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
		void InsertionSort(T * vals, int startIndex, int endIndex, Comparer comparer)
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

		inline void Swap(T * vals, int index1, int index2)
		{
			if (index1 != index2)
			{
				T tmp = static_cast<T&&>(vals[index1]);
				vals[index1] = static_cast<T&&>(vals[index2]);
				vals[index2] = static_cast<T&&>(tmp);
			}
		}

		template<typename T2, typename Comparer>
		int BinarySearch(const T2 & obj, Comparer comparer)
		{
			int imin = 0, imax = _count - 1;
			while (imax >= imin)
			{
				int imid = (imin + imax) >> 1;
				int compareResult = comparer(buffer[imid], obj);
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
		int BinarySearch(const T2 & obj)
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
	T Min(const List<T> & list)
	{
		T minVal = list.First();
		for (int i = 1; i<list.Count(); i++)
			if (list[i] < minVal)
				minVal = list[i];
		return minVal;
	}

	template<typename T>
	T Max(const List<T> & list)
	{
		T maxVal = list.First();
		for (int i = 1; i<list.Count(); i++)
			if (list[i] > maxVal)
				maxVal = list[i];
		return maxVal;
	}
}

#endif
