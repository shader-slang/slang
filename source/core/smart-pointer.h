#ifndef FUNDAMENTAL_LIB_SMART_POINTER_H
#define FUNDAMENTAL_LIB_SMART_POINTER_H

#include "type-traits.h"

namespace Slang
{
	class RefPtrDefaultDestructor
	{
	public:
		template<typename T>
		void operator ()(T * ptr)
		{
			delete ptr;
		}
	};

	class RefPtrArrayDestructor
	{
	public:
		template<typename T>
		void operator() (T * ptr)
		{
			delete [] ptr;
		}
	};

	class ReferenceCounted
	{
		template<typename T, bool b, typename Destructor>
		friend class RefPtrImpl;
	private:
		int _refCount = 0;
	public:
		ReferenceCounted() {}
		ReferenceCounted(const ReferenceCounted &)
		{
			_refCount = 0;
		}
	};


	class RefObject : public ReferenceCounted
	{
	public:
		virtual ~RefObject()
		{}
	};

	template<typename T, bool HasBuiltInCounter, typename Destructor>
	class RefPtrImpl
	{
	};

	template<typename T, typename Destructor = RefPtrDefaultDestructor>
	using RefPtr = RefPtrImpl<T, IsBaseOf<ReferenceCounted, T>::Value, Destructor>;

	template<typename T, typename Destructor>
	class RefPtrImpl<T, 0, Destructor>
	{
		template<typename T1, bool b, typename Destructor1>
		friend class RefPtrImpl;
	private:
		T * pointer;
		int * refCount;
			
	public:
		RefPtrImpl()
		{
			pointer = 0;
			refCount = 0;
		}
		RefPtrImpl(T * ptr)
			: pointer(0), refCount(0)
		{
			this->operator=(ptr);
		}
		RefPtrImpl(const RefPtrImpl<T, 0, Destructor> & ptr)
			: pointer(0), refCount(0)
		{
			this->operator=(ptr);
		}
		RefPtrImpl(RefPtrImpl<T, 0, Destructor> && str)
			: pointer(0), refCount(0)
		{
			this->operator=(static_cast<RefPtrImpl<T, 0, Destructor> &&>(str));
		}

		template <typename U>
		RefPtrImpl(const RefPtrImpl<U, 0, Destructor>& ptr,
			typename EnableIf<IsConvertible<T*, U*>::Value, void>::type * = 0)
			: pointer(0), refCount(0)
		{
			pointer = ptr.pointer;
			if (ptr)
			{
				refCount = ptr.refCount;
				(*refCount)++;
			}
			else
				refCount = 0;
		}

		template <typename U>
		typename EnableIf<IsConvertible<T*, U*>::value, RefPtrImpl<T, 0, Destructor>>::type&
			operator=(const RefPtrImpl<U,0,Destructor> & ptr)
		{
			Unreference();

			pointer = ptr;
			if (ptr)
			{
				refCount = ptr.refCount;
				(*refCount)++;
			}
			else
				refCount = 0;
			return *this;
		}

		RefPtrImpl<T, 0, Destructor>& operator=(const RefPtrImpl<T, 0, Destructor> & ptr)
		{
			Unreference();
			pointer = ptr.pointer;
			if (ptr)
			{
				refCount = ptr.refCount;
				(*refCount)++;
			}
			else
				refCount = 0;
			return *this;
		}

		RefPtrImpl<T, 0, Destructor>& operator=(T * ptr)
		{
			if (ptr != pointer)
			{
				Unreference();

				pointer = ptr;
				if (ptr)
				{
					refCount = new int;
					(*refCount) = 1;
				}
				else
					refCount = 0;
			}
			return *this;
		}
		int GetHashCode()
		{
			return (int)(long long)(void*)pointer;
		}
		bool operator == (const T * ptr) const
		{
			return pointer == ptr;
		}
		bool operator != (const T * ptr) const
		{
			return pointer != ptr;
		}
		template<typename U>
		bool operator == (const RefPtr<U, Destructor> & ptr) const
		{
			return pointer == ptr.pointer;
		}
		template<typename U>
		bool operator != (const RefPtr<U, Destructor> & ptr) const
		{
			return pointer != ptr.pointer;
		}
		template<typename U>
		RefPtrImpl<U, 0, Destructor> As() const
		{
			RefPtrImpl<U, 0, Destructor> result;
			if (pointer)
			{
				result.pointer = dynamic_cast<U*>(pointer);
				if (result.pointer)
				{
					result.refCount = refCount;
					(*refCount)++;
				}
			}
			return result;
		}

		T* operator +(int offset) const
		{
			return pointer+offset;
		}
		T& operator [](int idx) const
		{
			return *(pointer + idx);
		}
		RefPtrImpl<T, 0, Destructor>& operator=(RefPtrImpl<T, 0, Destructor> && ptr)
		{
			if(ptr.pointer != pointer)
			{
				Unreference();
				pointer = ptr.pointer;
				refCount = ptr.refCount;
				ptr.pointer = 0;
				ptr.refCount = 0;
			}
			return *this;
		}
		T* Release()
		{
			if(pointer)
			{
				if((*refCount) > 1)
				{
					(*refCount)--;
				}
				else
				{
					delete refCount;
				}
			}
			auto rs = pointer;
			refCount = 0;
			pointer = 0;
			return rs;
		}
		~RefPtrImpl()
		{
			Unreference();
		}

		void Unreference()
		{
			if(pointer)
			{
				if((*refCount) > 1)
				{
					(*refCount)--;
				}
				else
				{
					Destructor destructor;
					destructor(pointer);
					delete refCount;
				}
			}
		}
		T & operator *() const
		{
			return *pointer;
		}
		T * operator->() const
		{
			return pointer;
		}
		T * Ptr() const
		{
			return pointer;
		}
	public:
		explicit operator bool() const 
		{
			if (pointer)
				return true;
			else
				return false;
		}
	};


	template<typename T, typename Destructor>
	class RefPtrImpl<T, 1, Destructor>
	{
		template<typename T1, bool b, typename Destructor1>
		friend class RefPtrImpl;
			
	private:
		T * pointer;
	public:
		RefPtrImpl()
		{
			pointer = 0;
		}
		RefPtrImpl(T * ptr)
			: pointer(0)
		{
			this->operator=(ptr);
		}
		RefPtrImpl(const RefPtrImpl<T, 1, Destructor> & ptr)
			: pointer(0)
		{
			this->operator=(ptr);
		}
		RefPtrImpl(RefPtrImpl<T, 1, Destructor> && str)
			: pointer(0)
		{
			this->operator=(static_cast<RefPtrImpl<T, 1, Destructor> &&>(str));
		}
		template <typename U>
			RefPtrImpl(const RefPtrImpl<U, 1, Destructor>& ptr,
				typename EnableIf<IsConvertible<T*, U*>::Value, void>::type * = 0)
			: pointer(0)
		{
			pointer = ptr.pointer;
			if (ptr)
			{
				ptr->_refCount++;
			}
		}

		template <typename U>
		typename EnableIf<IsConvertible<T*, U*>::value, RefPtrImpl<T, 1, Destructor>&>::type
			operator=(const RefPtrImpl<U, 1, Destructor> & ptr)
		{
			Unreference();

			pointer = ptr.pointer;
			if (ptr)
			{
				ptr->_refCount++;
			}
			return *this;
		}
		RefPtrImpl<T, 1, Destructor>& operator=(T * ptr)
		{
			if (ptr != pointer)
			{
				Unreference();

				pointer = ptr;
				if (ptr)
				{
					ptr->_refCount++;
				}
			}
			return *this;
		}
		RefPtrImpl<T, 1, Destructor>& operator=(const RefPtrImpl<T, 1, Destructor> & ptr)
		{
			// Note: It is possible that the object this pointer references owns
			// (directly or indirectly) the storage for the argument `ptr`. If
			// that is the case and the `Unreference()` call below frees this
			// object, then the argument would become invalid.
			//
			// We copy the pointer value out of the argument first, in order
			// to protected against this case.
			T* ptrPointer = ptr.pointer;
			if (ptrPointer != pointer)
			{
				if (ptrPointer)
					ptrPointer->_refCount++;
				Unreference();
				pointer = ptrPointer;
			}
			return *this;
		}
		int GetHashCode()
		{
			return (int)(long long)(void*)pointer;
		}
		bool operator == (const T * ptr) const
		{
			return pointer == ptr;
		}
		bool operator != (const T * ptr) const
		{
			return pointer != ptr;
		}
		template<typename U>
		bool operator == (const RefPtr<U, Destructor> & ptr) const
		{
			return pointer == ptr.pointer;
		}
		template<typename U>
		bool operator != (const RefPtr<U, Destructor> & ptr) const
		{
			return pointer != ptr.pointer;
		}
		template<typename U>
		RefPtrImpl<U, 1, Destructor> As() const
		{
			RefPtrImpl<U, 1, Destructor> result;
			if (pointer)
			{
				result.pointer = dynamic_cast<U*>(pointer);
				if (result.pointer)
				{
					result.pointer->_refCount++;
				}
			}
			return result;
		}
		T* operator +(int offset) const
		{
			return pointer + offset;
		}
		T& operator [](int idx) const
		{
			return *(pointer + idx);
		}
		RefPtrImpl<T, 1, Destructor>& operator=(RefPtrImpl<T, 1, Destructor> && ptr)
		{
			if (ptr.pointer != pointer)
			{
				Unreference();
				pointer = ptr.pointer;
				ptr.pointer = nullptr;
			}
			return *this;
		}
		T* Release()
		{
			if (pointer)
			{
				pointer->_refCount--;
			}
			auto rs = pointer;
			pointer = 0;
			return rs;
		}
		~RefPtrImpl()
		{
			Unreference();
		}

		void Unreference()
		{
			if (pointer)
			{
				if (pointer->_refCount > 1)
				{
					pointer->_refCount--;
				}
				else
				{
					Destructor destructor;
					destructor(pointer);
				}
			}
		}
		T & operator *() const
		{
			return *pointer;
		}
		T * operator->() const
		{
			return pointer;
		}
		T * Ptr() const
		{
			return pointer;
		}
	public:
		explicit operator bool() const
		{
			if (pointer)
				return true;
			else
				return false;
		}
	};
}

#endif