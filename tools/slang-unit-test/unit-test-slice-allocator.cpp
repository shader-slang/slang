// unit-test-slice-allocator.cpp
//
// Contract under test
// -------------------
// `SliceAllocator` is a small wrapper around `MemoryArena` that
// returns `TerminatedCharSlice`s. Used by the artifact / blob
// machinery to hand out slang-API-shaped string slices without
// each caller managing its own arena lifetime. The contract:
//
//   * `allocate(...)` copies the input into the arena and returns
//     a TerminatedCharSlice whose `.begin()` points at a NUL-
//     terminated copy. The original input may freely change after
//     the call (independence guarantee).
//   * Length-preserving — embedded NULs in slice inputs are kept
//     verbatim. The terminator is always one byte past `.count`.
//   * `deallocateAll()` invalidates all prior slices but leaves
//     the allocator usable for fresh allocations.
//   * `getArena()` exposes the underlying MemoryArena for direct
//     allocations of non-string objects from the same arena.
//
// The independence guarantee is the load-bearing one — many
// callers store a SliceAllocator alongside a List<TerminatedCharSlice>
// and rely on the slices remaining valid after the source Strings
// go out of scope.

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

    // Allocate from a mutable buffer, then mutate the buffer
    // in-place. If the allocator borrowed the source memory rather
    // than copying it, the slice would observe the mutation. (A
    // String reassignment wouldn't catch this — the original
    // String storage might still be readable post-rebind under
    // common allocators, hiding a use-after-free.)
    char buf[12] = {'s', 'n', 'a', 'p', 's', 'h', 'o', 't', '-', 'm', 'e', '\0'};
    TerminatedCharSlice s = alloc.allocate(UnownedStringSlice(buf, buf + 11));

    // Overwrite every byte of the source.
    for (int i = 0; i < 11; ++i)
        buf[i] = 'X';

    // The slice must retain the original content.
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
    SLANG_CHECK(empty2.begin()[0] == '\0');
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
    (void)alloc.allocate("first");
    (void)alloc.allocate("second");

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

SLANG_UNIT_TEST(sliceAllocatorArenaIsTheSameArena)
{
    // The arena exposed via getArena() must be the *same* arena
    // that allocate(...) uses, so callers can safely co-allocate
    // string slices and ad-hoc structures together. Verify identity
    // by checking that the slice and a direct arena allocation are
    // both reported as valid by the same arena (`isValid` walks the
    // arena's block list).
    SliceAllocator alloc;
    TerminatedCharSlice s = alloc.allocate("test-string");
    SLANG_CHECK(s.count == 11);

    MemoryArena& arena = alloc.getArena();
    char* p = static_cast<char*>(arena.allocate(64));
    SLANG_CHECK(p != nullptr);
    SLANG_CHECK(arena.isValid(s.begin(), s.count));
    SLANG_CHECK(arena.isValid(p, 64));

    // Both allocations must survive a subsequent allocate() call —
    // the arena doesn't relocate existing pointers.
    TerminatedCharSlice s2 = alloc.allocate("another");
    SLANG_CHECK(strcmp(s.begin(), "test-string") == 0);
    SLANG_CHECK(strcmp(s2.begin(), "another") == 0);
    p[0] = 'X';
    SLANG_CHECK(p[0] == 'X');
}
