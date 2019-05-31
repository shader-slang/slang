#ifndef SLANG_CORE_ALLOCATOR_H
#define SLANG_CORE_ALLOCATOR_H

#include <stdlib.h>
#ifdef _MSC_VER
#   include <malloc.h>
#endif

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
}

#endif
