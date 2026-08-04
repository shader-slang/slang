#ifndef SLANG_CORE_ALLOCATOR_H
#define SLANG_CORE_ALLOCATOR_H

#include "slang-common.h"

#include <stdlib.h>
#if SLANG_WINDOWS_FAMILY
#include <malloc.h>
#endif
#if defined(SLANG_ENABLE_MIMALLOC)
#include <mimalloc.h>
#endif

#include <type_traits>

namespace Slang
{
class StandardAllocator
{
public:
    /// Allocates memory with the configured allocator for Slang-owned allocations.
    static void* allocate(size_t size)
    {
#if defined(SLANG_ENABLE_MIMALLOC)
        return mi_malloc(size);
#else
        return ::malloc(size);
#endif
    }

    /// Reallocates memory that was returned by `allocate`.
    static void* reallocate(void* ptr, size_t size)
    {
#if defined(SLANG_ENABLE_MIMALLOC)
        return mi_realloc(ptr, size);
#else
        return ::realloc(ptr, size);
#endif
    }

    // Marked SLANG_NO_INLINE: GCC 13 inlines this through long destructor chains
    // and then falsely reports that the freed pointer may be uninitialized
    // (-Wmaybe-uninitialized). Preventing inlining breaks the chain.
#if SLANG_GCC
    static SLANG_NO_INLINE void deallocate(void* ptr)
#else
    static void deallocate(void* ptr)
#endif
    {
#if defined(SLANG_ENABLE_MIMALLOC)
        mi_free(ptr);
#else
        ::free(ptr);
#endif
    }
};

template<int ALIGNMENT>
class AlignedAllocator
{
public:
    static void* allocate(size_t size)
    {
#if defined(SLANG_ENABLE_MIMALLOC)
        return mi_malloc_aligned(size, ALIGNMENT);
#elif SLANG_WINDOWS_FAMILY
        return _aligned_malloc(size, ALIGNMENT);
#elif defined(__CYGWIN__)
        return aligned_alloc(ALIGNMENT, size);
#else
        void* rs = nullptr;
        int succ = posix_memalign(&rs, ALIGNMENT, size);
        return (succ == 0) ? rs : nullptr;
#endif
    }

    static void deallocate(void* ptr)
    {
#if defined(SLANG_ENABLE_MIMALLOC)
        mi_free(ptr);
#elif SLANG_WINDOWS_FAMILY
        _aligned_free(ptr);
#else
        free(ptr);
#endif
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
    // Marked SLANG_NO_INLINE: GCC 13 inlines this through long destructor chains
    // into callers and then falsely reports the pointer argument as potentially
    // uninitialized (-Wmaybe-uninitialized). Preventing inlining breaks the chain.
#if SLANG_GCC
    static SLANG_NO_INLINE void deallocateArray(T* ptr, Index count)
#else
    static void deallocateArray(T* ptr, Index count)
#endif
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
} // namespace Slang

#endif
