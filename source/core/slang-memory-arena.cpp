
#include "slang-memory-arena.h"

namespace Slang {

MemoryArena::MemoryArena()
{
    // Mark as invalid so any alloc call will fail
    m_blockAlignment = 0;
    m_blockSize = 0;

    // Set up as empty
    m_blocks = nullptr;
    _setCurrentBlock(nullptr);
    m_blockFreeList.init(sizeof(Block), sizeof(void*), 16);
}

MemoryArena::~MemoryArena()
{
    _deallocateBlocks();
}

MemoryArena::MemoryArena(size_t blockSize, size_t blockAlignment)
{
    _initialize(blockSize, blockAlignment);
}

void MemoryArena::init(size_t blockSize, size_t blockAlignment)
{
    _deallocateBlocks();
    m_blockFreeList.reset();
    _initialize(blockSize, blockAlignment);
}
  
void MemoryArena::_initialize(size_t blockSize, size_t alignment)
{
    // Alignment must be a power of 2
    assert(((alignment - 1) & alignment) == 0);

    // Must be at least sizeof(void*) in size, as that is the minimum the backing allocator will be
    alignment = (alignment < kMinAlignment) ? kMinAlignment : alignment;

    // If alignment required is larger then the backing allocators then
    // make larger to ensure when alignment correction takes place it will be aligned
    if (alignment > kMinAlignment)
    {
        blockSize += alignment;
    }

    m_blockSize = blockSize;
    m_blockAlignment = alignment;
    m_blocks = nullptr;
    _setCurrentBlock(nullptr);
    m_blockFreeList.init(sizeof(Block), sizeof(void*), 16);
}

void* MemoryArena::_allocateAligned(size_t size, size_t alignment)
{
    assert(size);
    // Can't be space in the current block -> so we can either place in next, or in a new block
    _newCurrentBlock(size, alignment);
    uint8_t* const current = m_current;
    // If everything has gone to plan, must be space here...
    assert(current + size <= m_end);
    m_current = current + size;
    return current;
}

void MemoryArena::_deallocateBlocks()
{
    Block* currentBlock = m_blocks;
    while (currentBlock)
    {
        // Deallocate the block
        ::free(currentBlock->m_alloc);
        // next block
        currentBlock = currentBlock->m_next;
    }
    // Can deallocate all blocks to
    m_blockFreeList.deallocateAll();
}

void MemoryArena::_setCurrentBlock(Block* block)
{
    if (block)
    {
        m_end = block->m_end;
        m_start = block->m_start;
        m_current = m_start;
    }
    else
    {
        m_start = nullptr;
        m_end = nullptr;
        m_current = nullptr;
    }
    m_currentBlock = block;
}

MemoryArena::Block* MemoryArena::_newCurrentBlock(size_t size, size_t alignment)
{
    // Make sure init has been called (or has been set up in parameterized constructor)
    assert(m_blockSize > 0);
    // Alignment must be a power of 2
    assert(((alignment - 1) & alignment) == 0);

    // Alignment must at a minimum be block alignment (such if reused the constraints hold)
    alignment = (alignment < m_blockAlignment) ? m_blockAlignment : alignment;

    const size_t alignMask = alignment - 1;

    // First try the next block (if there is one)
    {
        Block* next = m_currentBlock ? m_currentBlock->m_next : m_blocks;
        if (next)
        {
            // Align could be done from the actual allocation start, but doing so would mean a pointer which
            // didn't hit the constraint of being between start/end
            // So have to align conservatively using start
            uint8_t* memory = (uint8_t*)((size_t(next->m_start) + alignMask) & ~alignMask);

            // Check if can fit block in
            if (memory + size <= next->m_end)
            {
                _setCurrentBlock(next);
                return next;
            }
        }
    }

    // The size of the block must be at least large enough to take into account alignment
    size_t allocSize = (alignment <= kMinAlignment) ? size : (size + alignment);

    // The minimum block size should be at least m_blockSize
    allocSize = (allocSize < m_blockSize) ? m_blockSize : allocSize;

    // Allocate block
    Block* block = (Block*)m_blockFreeList.allocate();
    if (!block)
    {
        return nullptr;
    }
    // Allocate the memory
    uint8_t* alloc = (uint8_t*)::malloc(allocSize);
    if (!alloc)
    {
        m_blockFreeList.deallocate(block);
        return nullptr;
    }
    // Do the alignment on the allocation
    uint8_t* const start = (uint8_t*)((size_t(alloc) + alignMask) & ~alignMask);

    // Setup the block
    block->m_alloc = alloc;
    block->m_start = start;
    block->m_end = alloc + allocSize;
    block->m_next = nullptr;

    // Insert block into list
    if (m_currentBlock)
    {
        // Insert after current block
        block->m_next = m_currentBlock->m_next;
        m_currentBlock->m_next = block;
    }
    else
    {
        // Add to start of the list of the blocks
        block->m_next = m_blocks;
        m_blocks = block;
    }
    _setCurrentBlock(block);
    return block;
}

MemoryArena::Block* MemoryArena::_findBlock(const void* alloc, Block* endBlock) const
{
    const uint8_t* ptr = (const uint8_t*)alloc;

    Block* block = m_blocks;
    while (block != endBlock)
    {
        if (ptr >= block->m_start && ptr < block->m_end)
        {
            return block;
        }
        block = block->m_next;
    }
    return nullptr;
}

MemoryArena::Block* MemoryArena::_findPreviousBlock(Block* block)
{
    Block* currentBlock = m_blocks;
    while (currentBlock)
    {
        if (currentBlock->m_next == block)
        {
            return currentBlock;
        }
        currentBlock = currentBlock->m_next;
    }
    return nullptr;
}

void MemoryArena::deallocateAll()
{
    Block** prev = &m_blocks;
    Block* block = m_blocks;

    while (block)
    {
        if (size_t(block->m_end - block->m_alloc) > m_blockSize)
        {
            // Oversized block so we need to free it and remove from the list
            Block* nextBlock = block->m_next;
            *prev = nextBlock;
            // Free the backing memory
            ::free(block->m_alloc);
            // Free the block
            m_blockFreeList.deallocate(block);
            // prev stays the same, now working on next tho
            block = nextBlock;
        }
        else
        {
            // Onto next
            prev = &block->m_next;
            block = block->m_next;
        }
    }

    // Make the first current (if any)
    _setCurrentBlock(m_blocks);
}

void MemoryArena::reset()
{
    _deallocateBlocks();
    m_blocks = nullptr;
    _setCurrentBlock(nullptr);
}

} // namespace Slang
