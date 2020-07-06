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
		void * rs = 0;
		int succ = posix_memalign(&rs, alignment, size);
		if (succ!=0)
			rs = 0;
		return rs;
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

    // Helper utilties for calling allocators.
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
            for (int i = 0; i < size; i++)
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
        static inline T* allocateArray(Index count)
        {
            TAllocator allocator;
            T* rs = (T*)allocator.allocate(count * sizeof(T));
            Initializer<T, std::is_pod<T>::value>::initialize(rs, count);
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
}

#endif
