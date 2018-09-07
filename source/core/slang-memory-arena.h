#ifndef SLANG_MEMORY_ARENA_H
#define SLANG_MEMORY_ARENA_H

#include "../../slang.h"

#include <assert.h>

#include <stdlib.h>
#include <string.h>

#include "slang-free-list.h"

namespace Slang { 

/** Defines arena allocator where allocations are made very quickly, but that deallocations
can only be performed in reverse order, or with the client code knowing a previous deallocation (say with
 deallocateAllFrom), automatically deallocates everything after it.

It works by allocating large blocks and then cutting out smaller pieces as requested. If a piece of memory is
deallocated, it either MUST be in reverse allocation order OR the subsequent allocations are implicitly 
deallocated too, and therefore accessing their memory is now undefined behavior. Allocations are made 
contiguously from the current block. If there is no space in the current block, the 
next block (which is unused) if available is checked. If that works, an allocation is made from the next block. 
If not a new block is allocated that can hold at least the allocation with required alignment.
 
All memory allocated can be deallocated very quickly and without a client having to track any memory. 
All memory allocated will be freed on destruction - or with reset.

A memory arena can have requests larger than the block size. When that happens they will just be allocated
from the heap. As such 'oversized blocks' are seen as unusual and potentially wasteful they are deallocated
when deallocateAll is called, whereas regular size blocks will remain allocated for fast subsequent allocation.

It is intentional that blocks information is stored separately from the allocations that store the
user data. This is so that alignment permitting, block allocations sizes can be passed directly to underlying allocator.
For large power of 2 backing allocations this might mean a page/pages directly allocated by the OS for example.
Also means better cache coherency when traversing blocks -> as generally they will be contiguous in memory.

Also note that allocateUnaligned can be used for slightly faster aligned allocations. All blocks allocated internally
are aligned to the blockAlignment passed to the constructor. If subsequent allocations (of any type) sizes are of that
alignment or larger then no alignment fixing is required (because allocations are contiguous) and so 'allocateUnaligned'
will return allocations of blockAlignment alignment.
*/
class MemoryArena
{
public:
    typedef MemoryArena ThisType;

    static const size_t kMinAlignment = sizeof(void*);  ///< The minimum alignment of the backing memory allocator.

        /** Determines if an allocation is consistent with an allocation from this arena.

        The test cannot say definitively if this was such an allocation, because the exact details
        of each allocation is not kept.
        @param alloc The start of the allocation
        @param sizeInBytes The size of the allocation
        @return true if allocation could have been from this Arena */
    bool isValid(const void* alloc, size_t sizeInBytes) const;

        /** Initialize the arena with specified block size and alignment 
        If the arena has been previously initialized will free and deallocate all memory */
    void init(size_t blockSize, size_t blockAlignment = kMinAlignment);

        /** Allocate some memory of at least size bytes without having any specific alignment.

         Can be used for slightly faster *aligned* allocations if caveats in class description are met. The
         Unaligned, means the method will not enforce alignment - but a client call to allocateUnaligned can control
         subsequent allocations alignments via it's size.

         @param size The size of the allocation requested (in bytes and must be > 0).
         @return The allocation. Can be nullptr if backing allocator was not able to request required memory */
    void* allocate(size_t sizeInBytes);

        /** Allocate some aligned memory of at least size bytes 
         @param size Size of allocation wanted (must be > 0).
         @param alignment Alignment of allocation - must be a power of 2.
         @return The allocation (or nullptr if unable to allocate). Will be at least 'alignment' alignment or better. */
    void* allocateAligned(size_t sizeInBytes, size_t alignment);

        /** Allocates a null terminated string.
        @param str A null-terminated string
        @return A copy of the string held on the arena */
    const char* allocateString(const char* str);

        /** Allocates a null terminated string.     
         @param chars Pointer to first character
         @param charCount The amount of characters NOT including terminating 0.
         @return A copy of the string held on the arena. */
    const char* allocateString(const char* chars, size_t numChars);

        /// Allocate an element of the specified type. Note: Constructor for type is not executed.
    template <typename T>
    T* allocate();

        /// Allocate an array of a specified type. NOTE Constructor of T is NOT executed.
    template <typename T>
    T* allocateArray(size_t size);

        /// Allocate an array of a specified type, and copy array passed into it.
    template <typename T>
    T* allocateAndCopyArray(const T* src, size_t size);  

        /// Deallocate the last allocation. If data is not from the last allocation then the behavior is undefined.
    void deallocateLast(void* data);

        /// Deallocate this allocation and all remaining after it.
    void deallocateAllFrom(void* dataStart);

        /** Deallocates all allocated memory. That backing memory will generally not be released so
         subsequent allocation will be fast, and from the same memory. Note though that 'oversize' blocks
         will be deallocated. */
    void deallocateAll();

        /// Resets to the initial state when constructed (and all backing memory will be deallocated)  
    void reset();
        /// Adjusts such that the next allocate will be at least to the block alignment.
    void adjustToBlockAlignment();
 
        /// Gets the block alignment that is passed at initialization otherwise 0 an invalid block alignment.
    size_t getBlockAlignment() const { return m_blockAlignment; }

        /// Default Ctor
    MemoryArena();
        /// Construct with block size and alignment. Alignment must be a power of 2.
    MemoryArena(size_t blockSize, size_t alignment = kMinAlignment);

        /// Dtor
    ~MemoryArena();

protected:
    struct Block
    {
        Block* m_next;
        uint8_t* m_alloc;
        uint8_t* m_start;
        uint8_t* m_end;
    };

    void _initialize(size_t blockSize, size_t blockAlignment);

    void* _allocateAligned(size_t size, size_t alignment);
    void _deallocateBlocks();

    void _setCurrentBlock(Block* block);

    Block* _newCurrentBlock(size_t size, size_t alignment);
    Block* _findBlock(const void* alloc, Block* endBlock = nullptr) const;
    Block* _findPreviousBlock(Block* block);

    uint8_t* m_start;
    uint8_t* m_end;
    uint8_t* m_current;
    size_t m_blockSize; 
    size_t m_blockAlignment;
    Block* m_blocks;
    Block* m_currentBlock;

    FreeList m_blockFreeList;

    private:
    // Disable
    MemoryArena(const ThisType& rhs) = delete;
    void operator=(const ThisType& rhs) = delete;
};

// --------------------------------------------------------------------------
inline bool MemoryArena::isValid(const void* data, size_t size) const
{
    assert(size);

    uint8_t* ptr = (uint8_t*)data;
    // Is it in current
    if (ptr >= m_start && ptr + size <= m_current)
    {
        return true;
    }
    // Is it in a previous block?
    Block* block = _findBlock(data, m_currentBlock);
    return block && (ptr >= block->m_start && (ptr + size) <= block->m_end);
}

// --------------------------------------------------------------------------
SLANG_FORCE_INLINE void* MemoryArena::allocate(size_t size)
{
    // Align with the minimum alignment
    const size_t alignMask = kMinAlignment - 1;
    uint8_t* mem = (uint8_t*)((size_t(m_current) + alignMask) & ~alignMask);

    if (mem + size <= m_end)
    {
        m_current = mem + size;
        return mem;
    }
    else
    {
        return _allocateAligned(size, kMinAlignment);
    }
}

// --------------------------------------------------------------------------
inline void* MemoryArena::allocateAligned(size_t size, size_t alignment)
{
    // Alignment must be a power of 2
    assert(((alignment - 1) & alignment) == 0);

    // Align the pointer
    const size_t alignMask = alignment - 1;
    uint8_t* memory = (uint8_t*)((size_t(m_current) + alignMask) & ~alignMask);

    if (memory + size <= m_end)
    {
        m_current = memory + size;
        return memory;
    }
    else
    {
        return _allocateAligned(size, alignment);
    }
}

// --------------------------------------------------------------------------
inline const char* MemoryArena::allocateString(const char* str)
{
    size_t size = ::strlen(str);
    if (size == 0)
    {
        return "";
    }
    char* dst = (char*)allocate(size + 1);
    ::memcpy(dst, str, size + 1);
    return dst;
}

// --------------------------------------------------------------------------
inline const char* MemoryArena::allocateString(const char* chars, size_t charsCount)
{
    if (charsCount == 0)
    {
        return "";
    }
    char* dst = (char*)allocate(charsCount + 1);
    ::memcpy(dst, chars, charsCount);

    // Add null-terminating zero
    dst[charsCount] = 0;
    return dst;
} 

// --------------------------------------------------------------------------
template <typename T>
inline T* MemoryArena::allocate()
{
    return reinterpret_cast<T*>(allocateAligned(sizeof(T), CARB_ALIGN_OF(T)));
}

// --------------------------------------------------------------------------
template <typename T>
inline T* MemoryArena::allocateArray(size_t count)
{
    return (count > 0) ? reinterpret_cast<T*>(allocateAligned(sizeof(T) * count, SLANG_ALIGN_OF(T))) : nullptr;
}

// --------------------------------------------------------------------------
template <typename T>
inline T* MemoryArena::allocateAndCopyArray(const T* arr, size_t size)
{
    if (size > 0)
    {
        const size_t totalSize = sizeof(T) * size;
        void* ptr = allocateAligned(totalSize, SLANG_ALIGN_OF(T));
        ::memcpy(ptr, arr, totalSize);
        return reinterpret_cast<T*>(ptr);
    }
    return nullptr;
}

// --------------------------------------------------------------------------
inline void MemoryArena::deallocateLast(void* data)
{
    // See if it's in current block
    uint8_t* ptr = (uint8_t*)data;
    if (ptr >= m_start && ptr < m_current)
    {
        // Then just go back
        m_current = ptr;
    }
    else
    {
        // Only called if not in the current block. Therefore can only be in previous
        Block* prevBlock = _findPreviousBlock(m_currentBlock);
        if (prevBlock == nullptr || (!(ptr >= prevBlock->m_start && ptr < prevBlock->m_end)))
        {
            assert(!"Allocation not found");
            return;
        }

        // Make the previous block the current
        _setCurrentBlock(prevBlock);
        // Make the current the alloc freed
        m_current = ptr;
    }
}

// --------------------------------------------------------------------------
inline void MemoryArena::deallocateAllFrom(void* data)
{
    // See if it's in current block, and is allocated (ie < m_current)
    uint8_t* ptr = (uint8_t*)data;
    if (ptr >= m_start && ptr < m_current)
    {
        // If it's in current block, then just go back
        m_current = ptr;
        return;
    }

    // Search all blocks prior to current block
    Block* block = _findBlock(data, m_currentBlock);
    assert(block);
    if (!block)
    {
        return;
    }
    // Make this current block
    _setCurrentBlock(block);

    // Move the pointer to the allocations position
    m_current = ptr;
}

// --------------------------------------------------------------------------
inline void MemoryArena::adjustToBlockAlignment()
{
    const size_t alignMask = m_blockAlignment - 1;
    uint8_t* ptr = (uint8_t*)((size_t(m_current) + alignMask) & ~alignMask);

    // Alignment might push beyond end of block... if so allocate a new block
    // This test could be avoided if we aligned m_end, but depending on block alignment that might waste some space
    if (ptr > m_end)
    {
        // We'll need a new block to make this alignment
        _newCurrentBlock(0, m_blockAlignment);
    }
    else
    {
        // Set the position
        m_current = ptr;
    }
    assert(size_t(m_current) & alignMask);
}

} // namespace Slang

#endif // SLANG_MEMORY_ARENA_H