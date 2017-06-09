#ifndef CORE_LIB_ALLOCATOR_H
#define CORE_LIB_ALLOCATOR_H

#include <stdlib.h>

namespace CoreLib
{
	namespace Basic
	{
		inline void * AlignedAlloc(size_t size, size_t alignment)
		{
#ifdef _MSC_VER
			return _aligned_malloc(size, alignment);
#else
			void * rs = 0;
			int succ = posix_memalign(&rs, alignment, size);
			if (succ!=0)
				rs = 0;
			return rs;
#endif
		}

		inline void AlignedFree(void * ptr)
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
			void * Alloc(size_t size)
			{
				return malloc(size);
			}
			void Free(void * ptr)
			{
				return free(ptr);
			}
		};

		template<int alignment>
		class AlignedAllocator
		{
		public:
			void * Alloc(size_t size)
			{
				return AlignedAlloc(size, alignment);
			}
			void Free(void * ptr)
			{
				return AlignedFree(ptr);
			}
		};
	}
}

#endif