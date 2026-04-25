// unit-test-slice-allocator.cpp

#include "../../source/compiler-core/slang-slice-allocator.h"
#include "../../source/core/slang-string.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

SLANG_UNIT_TEST(sliceAllocatorAllocateFromCStr)
{
    SliceAllocator alloc;
    TerminatedCharSlice s = alloc.allocate("hello");
    SLANG_CHECK(s.count == 5);
    SLANG_CHECK(strcmp(s.begin(), "hello") == 0);
    // TerminatedCharSlice guarantees a NUL after the last byte.
    SLANG_CHECK(s.begin()[s.count] == '\0');
}

SLANG_UNIT_TEST(sliceAllocatorAllocateFromSlice)
{
    SliceAllocator alloc;
    UnownedStringSlice in = toSlice("hello world");
    TerminatedCharSlice s = alloc.allocate(in);
    SLANG_CHECK(s.count == in.getLength());
    SLANG_CHECK(memcmp(s.begin(), in.begin(), in.getLength()) == 0);
    SLANG_CHECK(s.begin()[s.count] == '\0');
}

SLANG_UNIT_TEST(sliceAllocatorAllocateFromString)
{
    SliceAllocator alloc;
    String in = "owned-string";
    TerminatedCharSlice s = alloc.allocate(in);
    SLANG_CHECK(s.count == in.getLength());
    SLANG_CHECK(strcmp(s.begin(), in.getBuffer()) == 0);
}

SLANG_UNIT_TEST(sliceAllocatorAllocateFromRange)
{
    SliceAllocator alloc;
    const char* full = "abcdefg";
    TerminatedCharSlice s = alloc.allocate(full + 1, full + 4); // "bcd"
    SLANG_CHECK(s.count == 3);
    SLANG_CHECK(s.begin()[0] == 'b');
    SLANG_CHECK(s.begin()[1] == 'c');
    SLANG_CHECK(s.begin()[2] == 'd');
    SLANG_CHECK(s.begin()[3] == '\0');
}

SLANG_UNIT_TEST(sliceAllocatorAllocatedDataIsIndependent)
{
    SliceAllocator alloc;

    // The backing buffer of the input string can change without
    // affecting the allocated slice.
    String input = "snapshot-me";
    TerminatedCharSlice s = alloc.allocate(input);

    input = "different";
    SLANG_CHECK(strcmp(s.begin(), "snapshot-me") == 0);
}

SLANG_UNIT_TEST(sliceAllocatorListAllocate)
{
    SliceAllocator alloc;
    List<String> in;
    in.add("alpha");
    in.add("beta");
    in.add("gamma");

    Slice<TerminatedCharSlice> out = alloc.allocate(in);
    SLANG_CHECK(out.count == 3);
    SLANG_CHECK(strcmp(out[0].begin(), "alpha") == 0);
    SLANG_CHECK(strcmp(out[1].begin(), "beta") == 0);
    SLANG_CHECK(strcmp(out[2].begin(), "gamma") == 0);
}

SLANG_UNIT_TEST(sliceAllocatorEmptyInputs)
{
    SliceAllocator alloc;

    TerminatedCharSlice empty1 = alloc.allocate("");
    SLANG_CHECK(empty1.count == 0);
    SLANG_CHECK(empty1.begin()[0] == '\0');

    TerminatedCharSlice empty2 = alloc.allocate(toSlice(""));
    SLANG_CHECK(empty2.count == 0);
}

SLANG_UNIT_TEST(sliceAllocatorMultipleAllocations)
{
    SliceAllocator alloc;
    const Index count = 256;
    List<TerminatedCharSlice> all;
    for (Index i = 0; i < count; ++i)
    {
        StringBuilder b;
        b << "item-" << Index(i);
        all.add(alloc.allocate(b.produceString()));
    }
    // Each allocation should still be valid after later allocations.
    for (Index i = 0; i < count; ++i)
    {
        StringBuilder expected;
        expected << "item-" << Index(i);
        SLANG_CHECK(strcmp(all[i].begin(), expected.produceString().getBuffer()) == 0);
    }
}

SLANG_UNIT_TEST(sliceAllocatorDeallocateAllClearsArena)
{
    SliceAllocator alloc;
    alloc.allocate("first");
    alloc.allocate("second");

    // Reset the arena and re-allocate. We can't safely deref the
    // earlier slices after deallocateAll, so just check that the
    // allocator continues to function.
    alloc.deallocateAll();

    TerminatedCharSlice fresh = alloc.allocate("after-reset");
    SLANG_CHECK(strcmp(fresh.begin(), "after-reset") == 0);
}

SLANG_UNIT_TEST(sliceAllocatorPreservesEmbeddedNuls)
{
    // The slice form should preserve the full input length, even if
    // it contains an embedded NUL — but the `const char*` form stops
    // at the first NUL (per C convention). Document both shapes.
    SliceAllocator alloc;

    const char buf[] = {'a', '\0', 'b', '\0', 'c'};
    UnownedStringSlice in(buf, buf + 5);
    TerminatedCharSlice s = alloc.allocate(in);
    SLANG_CHECK(s.count == 5);
    SLANG_CHECK(s.begin()[0] == 'a');
    SLANG_CHECK(s.begin()[1] == '\0');
    SLANG_CHECK(s.begin()[2] == 'b');
    SLANG_CHECK(s.begin()[3] == '\0');
    SLANG_CHECK(s.begin()[4] == 'c');
    // Trailing terminator after length.
    SLANG_CHECK(s.begin()[5] == '\0');
}

SLANG_UNIT_TEST(sliceAllocatorArenaAccessor)
{
    SliceAllocator alloc;
    // Sanity: the public arena accessor is callable and returns a
    // reference to a usable arena (block-allocate something).
    MemoryArena& arena = alloc.getArena();
    void* p = arena.allocate(64);
    SLANG_CHECK(p != nullptr);
}
