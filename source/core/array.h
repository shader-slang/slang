#ifndef CORE_LIB_ARRAY_H
#define CORE_LIB_ARRAY_H

#include "exception.h"
#include "array-view.h"

namespace CoreLib
{
	namespace Basic
	{
		template<typename T, int size>
		class Array
		{
		private:
			T _buffer[size];
			int _count = 0;
		public:
			T* begin() const
			{
				return (T*)_buffer;
			}
			T* end() const
			{
				return (T*)_buffer + _count;
			}
		public:
			inline int GetCapacity() const
			{
				return size;
			}
			inline int Count() const
			{
				return _count;
			}
			inline T & First() const
			{
				return const_cast<T&>(_buffer[0]);
			}
			inline T & Last() const
			{
				return const_cast<T&>(_buffer[_count - 1]);
			}
			inline void SetSize(int newSize)
			{
#ifdef _DEBUG
				if (newSize > size)
					throw IndexOutofRangeException("size too large.");
#endif
				_count = newSize;
			}
			inline void Add(const T & item)
			{
#ifdef _DEBUG
				if (_count == size)
					throw IndexOutofRangeException("out of range access to static array.");
#endif
				_buffer[_count++] = item;
			}
			inline void Add(T && item)
			{
#ifdef _DEBUG
				if (_count == size)
					throw IndexOutofRangeException("out of range access to static array.");
#endif
				_buffer[_count++] = _Move(item);
			}

			inline T & operator [](int id) const
			{
#if _DEBUG
				if (id >= _count || id < 0)
					throw IndexOutofRangeException("Operator[]: Index out of Range.");
#endif
				return ((T*)_buffer)[id];
			}

			inline T* Buffer() const
			{
				return (T*)_buffer;
			}

			inline void Clear()
			{
				_count = 0;
			}

			template<typename T2>
			int IndexOf(const T2 & val) const
			{
				for (int i = 0; i < _count; i++)
				{
					if (_buffer[i] == val)
						return i;
				}
				return -1;
			}

			template<typename T2>
			int LastIndexOf(const T2 & val) const
			{
				for (int i = _count - 1; i >= 0; i--)
				{
					if (_buffer[i] == val)
						return i;
				}
				return -1;
			}

			inline ArrayView<T> GetArrayView() const
			{
				return ArrayView<T>((T*)_buffer, _count);
			}
			inline ArrayView<T> GetArrayView(int start, int count) const
			{
				return ArrayView<T>((T*)_buffer + start, count);
			}
		};

		template<typename T, typename ...TArgs>
		struct FirstType
		{
			typedef T type;
		};


		template<typename T, int size>
		void InsertArray(Array<T, size> &) {}

		template<typename T, typename ...TArgs, int size>
		void InsertArray(Array<T, size> & arr, const T & val, TArgs... args)
		{
			arr.Add(val);
			InsertArray(arr, args...);
		}

		template<typename ...TArgs>
		auto MakeArray(TArgs ...args) -> Array<typename FirstType<TArgs...>::type, sizeof...(args)>
		{
			Array<typename FirstType<TArgs...>::type, sizeof...(args)> rs;
			InsertArray(rs, args...);
			return rs;
		}
	}
}

#endif