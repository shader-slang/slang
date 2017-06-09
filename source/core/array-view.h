#ifndef CORE_LIB_ARRAY_VIEW_H
#define CORE_LIB_ARRAY_VIEW_H

#include "Exception.h"

namespace CoreLib
{
	namespace Basic
	{
		template<typename T>
		class ArrayView
		{
		private:
			T * _buffer;
			int _count;
			int stride;
		public:
			T* begin() const
			{
				return _buffer;
			}
			T* end() const
			{
				return (T*)((char*)_buffer + _count*stride);
			}
		public:
			ArrayView()
			{
				_buffer = 0;
				_count = 0;
			}
			ArrayView(const T & singleObj)
			{
				SetData((T*)&singleObj, 1, sizeof(T));
			}
			ArrayView(T * buffer, int count)
			{
				SetData(buffer, count, sizeof(T));
			}
			ArrayView(void * buffer, int count, int _stride)
			{
				SetData(buffer, count, _stride);
			}
			void SetData(void * buffer, int count, int _stride)
			{
				this->_buffer = (T*)buffer;
				this->_count = count;
				this->stride = _stride;
			}
			inline int GetCapacity() const
			{
				return _count;
			}
			inline int Count() const
			{
				return _count;
			}

			inline T & operator [](int id) const
			{
#if _DEBUG
				if (id >= _count || id < 0)
					throw IndexOutofRangeException("Operator[]: Index out of Range.");
#endif
				return *(T*)((char*)_buffer+id*stride);
			}

			inline T* Buffer() const
			{
				return _buffer;
			}

			template<typename T2>
			int IndexOf(const T2 & val) const
			{
				for (int i = 0; i < _count; i++)
				{
					if (*(T*)((char*)_buffer + i*stride) == val)
						return i;
				}
				return -1;
			}

			template<typename T2>
			int LastIndexOf(const T2 & val) const
			{
				for (int i = _count - 1; i >= 0; i--)
				{
					if (*(T*)((char*)_buffer + i*stride) == val)
						return i;
				}
				return -1;
			}

			template<typename Func>
			int FindFirst(const Func & predicate) const
			{
				for (int i = 0; i < _count; i++)
				{
					if (predicate(_buffer[i]))
						return i;
				}
				return -1;
			}

			template<typename Func>
			int FindLast(const Func & predicate) const
			{
				for (int i = _count - 1; i >= 0; i--)
				{
					if (predicate(_buffer[i]))
						return i;
				}
				return -1;
			}
		};

		template<typename T>
		ArrayView<T> MakeArrayView(const T & obj)
		{
			return ArrayView<T>(obj);
		}
		
		template<typename T>
		ArrayView<T> MakeArrayView(T * buffer, int count)
		{
			return ArrayView<T>(buffer, count);
		}
	}
}
#endif