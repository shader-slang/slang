#include "core/slang-allocator.h"
#include "core/slang-blob.h"
#include "core/slang-memory-arena.h"
#include "unit-test/slang-unit-test.h"

#include <stdint.h>

using namespace Slang;

SLANG_UNIT_TEST(defaultAllocator)
{
    void* allocation = StandardAllocator::allocate(32);
    SLANG_CHECK(allocation != nullptr);
    if (!allocation)
        return;

#if defined(SLANG_ENABLE_MIMALLOC)
    SLANG_CHECK(mi_check_owned(allocation));
#endif

    void* reallocated = StandardAllocator::reallocate(allocation, 64);
    SLANG_CHECK(reallocated != nullptr);
    if (!reallocated)
    {
        StandardAllocator::deallocate(allocation);
        return;
    }

#if defined(SLANG_ENABLE_MIMALLOC)
    SLANG_CHECK(mi_check_owned(reallocated));
#endif
    StandardAllocator::deallocate(reallocated);

    void* aligned = AlignedAllocator<64>::allocate(128);
    SLANG_CHECK(aligned != nullptr);
    if (!aligned)
        return;
    SLANG_CHECK((uintptr_t(aligned) & 63) == 0);
#if defined(SLANG_ENABLE_MIMALLOC)
    SLANG_CHECK(mi_check_owned(aligned));
#endif
    AlignedAllocator<64>::deallocate(aligned);
}

SLANG_UNIT_TEST(scopedAllocationOwnsAttachedStandardAllocation)
{
    void* standardAllocation = StandardAllocator::allocate(32);
    SLANG_CHECK(standardAllocation != nullptr);
    if (!standardAllocation)
        return;

    ScopedAllocation allocation;
    allocation.attach(standardAllocation, 32);
    allocation.reallocate(64);
    SLANG_CHECK(allocation.getData() != nullptr);
    if (!allocation.getData())
        return;

#if defined(SLANG_ENABLE_MIMALLOC)
    SLANG_CHECK(mi_check_owned(allocation.getData()));
#endif

    ScopedAllocation swappedAllocation;
    allocation.swap(swappedAllocation);
    SLANG_CHECK(allocation.getData() == nullptr);
    SLANG_CHECK(swappedAllocation.getData() != nullptr);

    ComPtr<ISlangBlob> blob = RawBlob::moveCreate(swappedAllocation);
    SLANG_CHECK(swappedAllocation.getData() == nullptr);
    SLANG_CHECK(blob->getBufferSize() == 64);
}

SLANG_UNIT_TEST(memoryArenaOwnsExternalStandardAllocation)
{
    {
        MemoryArena arena(256);
        void* allocation = StandardAllocator::allocate(64);
        SLANG_CHECK(allocation != nullptr);
        if (allocation)
        {
            arena.addExternalBlock(allocation, 64);
            arena.deallocateAll();
        }
    }

    {
        MemoryArena arena(256);
        void* allocation = StandardAllocator::allocate(64);
        SLANG_CHECK(allocation != nullptr);
        if (allocation)
        {
            arena.addExternalBlock(allocation, 64);
            arena.reset();
        }
    }

    MemoryArena arena(256);
    void* allocation = StandardAllocator::allocate(64);
    SLANG_CHECK(allocation != nullptr);
    if (allocation)
        arena.addExternalBlock(allocation, 64);
}
