#ifndef SLANG_MEMORY_ARENA_H
#define SLANG_MEMORY_ARENA_H

#include "../../slang.h"

#include <assert.h>

#include <stdlib.h>
#include <string.h>

#include "slang-free-list.h"

namespace Slang { 

/** MemoryArena provides provides very fast allocation of small blocks, by aggregating many small allocations 
over smaller amount of larger blocks. A typical small unaligned allocation is a pointer bump.  

Allocations are made contiguously from the current block. If there is no space in the current block, the 
next block (which is unused) if available is checked. If that works, an allocation is made from the next block. 
If not a new block is allocated that can hold at least the allocation with required alignment.
 
All memory allocated can be deallocated very quickly and without a client having to track any memory. 
All memory allocated will be freed on destruction - or with reset.

A memory arena can have requests larger than the block size. When that happens they will just be allocated
from the heap. As such 'odd blocks' are seen as unusual and potentially wasteful so they are deallocated
when deallocateAll is called, whereas regular size blocks will remain allocated for fast subsequent allocation.

It is intentional that blocks information is stored separately from the allocations that store the
user data. This is so that alignment permitting, block allocations sizes can be passed directly to underlying allocator.
For large power of 2 backing allocations this might mean a page/pages directly allocated by the OS for example.
Also means better cache coherency when traversing blocks -> as generally they will be contiguous in memory.

Note that allocateUnaligned can be used for slightly faster aligned allocations. All blocks allocated internally
are aligned to the blockAlignment passed to the constructor. If subsequent allocations (of any type) sizes are of that
alignment or larger then no alignment fixing is required (because allocations are contiguous) and so 'allocateUnaligned'
will return allocations of blockAlignment alignment.

If many 'odd' allocations occur it probably means that the block size should be increased. 
*/
class MemoryArena
{
public:
    typedef MemoryArena ThisType;

        /** The minimum alignment of the backing memory allocator.
        NOTE! That this should not be greater than the alignment of the underlying allocator.
        */
    static const size_t kMinAlignment = sizeof(void*);  
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

        /** Allocate some aligned memory of at least size bytes 
        @param size Size of allocation wanted (must be > 0).
        @return The allocation (or nullptr if unable to allocate).  */
    void* allocateUnaligned(size_t sizeInBytes);

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

        /// Allocate an array of a specified type, and zero it.
    template <typename T>
    T* allocateAndZeroArray(size_t size);

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

        /// Estimate of total amount of memory used in bytes. The number can never be smaller than actual used memory but may be larger
    size_t calcTotalMemoryUsed() const;
        /// Total memory allocated in bytes
    size_t calcTotalMemoryAllocated() const;

        /// Default Ctor
    MemoryArena();
        /// Construct with block size and alignment. Block alignment must be a power of 2.
    MemoryArena(size_t blockPayloadSize, size_t blockAlignment = kMinAlignment);

        /// Dtor
    ~MemoryArena();

protected:
    struct Block
    {
        Block* m_next;                  ///< Singly linked list of blocks
        uint8_t* m_alloc;               ///< Allocation start (ie what to free)
        uint8_t* m_start;               ///< Start of payload (takes into account alignment)
        uint8_t* m_end;                 ///< End of payload (m_start to m_end defines payload)
    };

    void _initialize(size_t blockPayloadSize, size_t blockAlignment);

        /// Delete the linked list of blocks specified by start
    void _deallocateBlocks(Block* start);
        /// Delete the linked list of blocks payloads specified by start
    void _deallocateBlocksPayload(Block* start);

    void _resetCurrentBlock();
    void _addCurrentBlock(Block* block);

    static Block* _joinBlocks(Block* pre, Block* post);

        /// Create a new block with regular block alignment 
    Block* _newNormalBlock();
        /// Allocates a new block with allocSize and alignment 
    Block* _newBlock(size_t allocSize, size_t alignment);

    void* _allocateAlignedFromNewBlock(size_t size, size_t alignment);

        /// Find block that contains data/size that is _NOT_ current (ie not first block in m_usedBlocks)
    const Block* _findNonCurrent(const void* data, size_t size) const;
    const Block* _findInBlocks(const Block* block, const void* data, size_t size) const;

    size_t _calcBlocksUsedMemory(const Block* block) const;
    size_t _calcBlocksAllocatedMemory(const Block* block) const;

    uint8_t* m_start;               ///< The start of the current block (pointed to by m_usedBlocks)
    uint8_t* m_end;                 ///< The end of the current block
    uint8_t* m_current;             ///< The current position in current block

    size_t m_blockPayloadSize;      ///< The size of the payload of a block
    size_t m_blockAllocSize;        ///< The size of a block allocation (must be the same size or bigger than m_blockPayloadSize)
    size_t m_blockAlignment;        ///< The alignment applied to used blocks

    Block* m_availableBlocks;       ///< Standard sized blocks that are available
    Block* m_usedBlocks;            ///< List of all normal sized used blocks. The first one is the 'current block'
    Block* m_usedOddBlocks;         ///< Used 'odd' blocks - blocks can actually be smaller than normal blocks, but are typically larger. 

    FreeList m_blockFreeList;       ///< Holds all of the blocks for fast allocation/free

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
    return (ptr >= m_start && ptr + size <= m_current) || _findNonCurrent(data, size) != nullptr;
}

// --------------------------------------------------------------------------
SLANG_FORCE_INLINE void* MemoryArena::allocateUnaligned(size_t size)
{
    assert(size > 0);
    // Align with the minimum alignment
    uint8_t* mem = m_current;
    uint8_t* end = mem + size;
    if (end <= m_end)
    {
        m_current = end;
        return mem;
    }
    else
    {
        return _allocateAlignedFromNewBlock(size, kMinAlignment);
    }
}

// --------------------------------------------------------------------------
SLANG_FORCE_INLINE void* MemoryArena::allocate(size_t size)
{
    assert(size > 0);
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
        return _allocateAlignedFromNewBlock(size, kMinAlignment);
    }
}

// --------------------------------------------------------------------------
inline void* MemoryArena::allocateAligned(size_t size, size_t alignment)
{
    assert(size > 0);
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
        return _allocateAlignedFromNewBlock(size, alignment);
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
    char* dst = (char*)allocateUnaligned(size + 1);
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
    char* dst = (char*)allocateUnaligned(charsCount + 1);
    ::memcpy(dst, chars, charsCount);

    // Add null-terminating zero
    dst[charsCount] = 0;
    return dst;
} 

// --------------------------------------------------------------------------
template <typename T>
inline T* MemoryArena::allocate()
{
    return reinterpret_cast<T*>(allocateAligned(sizeof(T), SLANG_ALIGN_OF(T)));
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
    SLANG_COMPILE_TIME_ASSERT(std::is_pod<T>::value);

    if (size > 0)
    {
        const size_t totalSize = sizeof(T) * size;
        void* ptr = allocateAligned(totalSize, SLANG_ALIGN_OF(T));
        ::memcpy(ptr, arr, totalSize);
        return reinterpret_cast<T*>(ptr);
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
template <typename T>
inline T* MemoryArena::allocateAndZeroArray(size_t size)
{
    if (size > 0)
    {
        const size_t totalSize = sizeof(T) * size;
        void* ptr = allocateAligned(totalSize, SLANG_ALIGN_OF(T));
        ::memset(ptr, 0, totalSize);
        return reinterpret_cast<T*>(ptr);
    }
    return nullptr;
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
        // We'll need a new block to make this alignment. Allocate a byte, and then rewind it.
        _allocateAlignedFromNewBlock(1, 1);
        m_current = m_usedBlocks->m_start; 
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