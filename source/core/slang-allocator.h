#ifndef SLANG_CORE_ALLOCATOR_H
#define SLANG_CORE_ALLOCATOR_H

#include "slang-common.h"

#include <stdlib.h>
#ifdef _MSC_VER
#   include <malloc.h>
#endif

#include <type_traits>

namespace Slang
{
	inline void* alignedAllocate(size_t size, size_t alignment)
	{
#ifdef _MSC_VER
		return _aligned_malloc(size, alignment);
#elif defined(__CYGWIN__)
        return aligned_alloc(alignment, size);
#else
		void* rs = nullptr;
		int succ = posix_memalign(&rs, alignment, size);
        return (succ == 0) ? rs : nullptr;
#endif
	}

	inline void alignedDeallocate(void* ptr)
	{
#ifdef _MSC_VER
		_aligned_free(ptr);
#else
		free(ptr);
#endif
	}

	class StandardAllocator
	{
	public:
		// not really called
		void* allocate(size_t size)
		{
			return ::malloc(size);
		}
		void deallocate(void * ptr)
		{
			return ::free(ptr);
		}
	};

	template<int ALIGNMENT>
	class AlignedAllocator
	{
	public:
		void* allocate(size_t size)
		{
			return alignedAllocate(size, ALIGNMENT);
		}
		void deallocate(void * ptr)
		{
			return alignedDeallocate(ptr);
		}
	};

    template<typename T, typename TAllocator>
    class AllocateMethod
    {
    public:
        static inline T* allocateArray(Index count)
        {
            TAllocator allocator;
            T* rs = (T*)allocator.allocate(count * sizeof(T));
            if (!std::is_trivially_constructible<T>::value)
            {
                for (Index i = 0; i < count; i++)
                    new (rs + i) T();
            }
            return rs;
        }
        static inline void deallocateArray(T* ptr, Index count)
        {
            TAllocator allocator;
            if (!std::is_trivially_destructible<T>::value)
            {
                for (Index i = 0; i < count; i++)
                    ptr[i].~T();
            }
            allocator.deallocate(ptr);
        }
    };

#if 0
    template<typename T>
    class AllocateMethod<T, StandardAllocator>
    {
    public:
        static inline T* allocateArray(Index count)
        {
            return new T[count];
        }
        static inline void deallocateArray(T* ptr, Index /*bufferSize*/)
        {
            delete[] ptr;
        }
    };
#endif
}

#endif
