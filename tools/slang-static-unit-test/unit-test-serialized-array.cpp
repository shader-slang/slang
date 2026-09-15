// unit-test-serialized-array.cpp

#include "core/slang-blob-builder.h"
#include "slang-com-ptr.h"
#include "slang/slang-fossil.h"
#include "slang/slang-serialize-fossil.h"
#include "slang/slang-serialize.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

// Tests the value semantics of `SerializedArray<T>`.
//
// The type is two states behind one interface: a *view* pointing into a serialized blob it
// does not own, and an *owned* array holding its own storage. `_data` is a raw pointer that
// for an owned array points into `_owned`'s buffer, which is why copy, move and assignment
// are written out rather than defaulted -- a defaulted copy would give the destination its
// own `_owned` while `_data` still referred to the *source's* buffer, so the copy would
// dangle as soon as the source grew or died.
//
// That failure is silent: the copy keeps reading plausible values out of freed or
// reallocated memory. Nothing else in the suite constructs these directly, so without this
// file the hand-written operators are exercised only incidentally, through whatever shapes
// the builtin modules happen to produce.
//
// Each test below forces the source to reallocate after the copy, since a copy that
// aliases the source's buffer only misbehaves once that buffer moves.

namespace
{

/// Fills `array` with `count` elements whose values are derived from their index, so a
/// wrong buffer shows up as wrong values rather than as a crash.
void _fillOwned(SerializedArray<Int64>& array, Count count)
{
    array.beginOwned(0);
    for (Index i = 0; i < count; ++i)
        array.add(Int64(i * 7 + 1));
}

bool _matchesPattern(const SerializedArray<Int64>& array, Count count)
{
    if (array.getCount() != count)
        return false;
    for (Index i = 0; i < count; ++i)
    {
        if (array[i] != Int64(i * 7 + 1))
            return false;
    }
    return true;
}

/// Appends enough elements to guarantee the buffer is reallocated, so that anything still
/// pointing at the old allocation is now pointing at freed memory.
void _forceReallocation(SerializedArray<Int64>& array)
{
    for (Index i = 0; i < 4096; ++i)
        array.add(Int64(-1));
}

} // namespace

SLANG_UNIT_TEST(serializedArrayCopyOfOwnedDoesNotAliasSource)
{
    SerializedArray<Int64> source;
    _fillOwned(source, 8);

    SerializedArray<Int64> copy(source);
    SLANG_CHECK(!copy.isView());
    SLANG_CHECK(_matchesPattern(copy, 8));

    // The copy must own its storage, not point into the source's.
    SLANG_CHECK(copy.getBuffer() != source.getBuffer());

    // Under a defaulted copy this is where it breaks: the source's buffer moves and the
    // copy is left reading the old one.
    _forceReallocation(source);
    SLANG_CHECK(_matchesPattern(copy, 8));
}

SLANG_UNIT_TEST(serializedArrayCopyAssignmentOfOwnedDoesNotAliasSource)
{
    SerializedArray<Int64> source;
    _fillOwned(source, 8);

    SerializedArray<Int64> copy;
    _fillOwned(copy, 3); // Non-empty first, so assignment has to replace existing storage.
    copy = source;

    SLANG_CHECK(!copy.isView());
    SLANG_CHECK(copy.getBuffer() != source.getBuffer());
    _forceReallocation(source);
    SLANG_CHECK(_matchesPattern(copy, 8));
}

SLANG_UNIT_TEST(serializedArrayMoveOfOwnedLeavesDestinationSelfContained)
{
    SerializedArray<Int64> source;
    _fillOwned(source, 8);

    SerializedArray<Int64> moved(static_cast<SerializedArray<Int64>&&>(source));
    SLANG_CHECK(!moved.isView());
    SLANG_CHECK(_matchesPattern(moved, 8));

    // The moved-from object must be left empty and owning, not a view onto storage it
    // handed away.
    SLANG_CHECK(source.getCount() == 0);
    SLANG_CHECK(!source.isView());

    // Growing the destination must not disturb its contents; it holds the only reference
    // to that storage now.
    moved.add(Int64(99));
    SLANG_CHECK(moved.getCount() == 9);
    SLANG_CHECK(moved[0] == Int64(1));
}

SLANG_UNIT_TEST(serializedArrayViewCopiesAndMovesAsAView)
{
    const Int64 backing[4] = {10, 20, 30, 40};

    SerializedArray<Int64> view;
    view.adoptView(backing, 4);
    SLANG_CHECK(view.isView());
    SLANG_CHECK(view.getBuffer() == backing);

    // A view copies as a view: it has no owned storage to re-point into, and copying the
    // pointer is correct because the blob outlives both.
    SerializedArray<Int64> copied(view);
    SLANG_CHECK(copied.isView());
    SLANG_CHECK(copied.getBuffer() == backing);
    SLANG_CHECK(copied.getCount() == 4);

    SerializedArray<Int64> moved(static_cast<SerializedArray<Int64>&&>(view));
    SLANG_CHECK(moved.isView());
    SLANG_CHECK(moved.getBuffer() == backing);
    SLANG_CHECK(moved.getCount() == 4);

    // A moved-from view must not still claim to be one, or it would report a dependency
    // on a blob it no longer refers to.
    SLANG_CHECK(!view.isView());
    SLANG_CHECK(view.getCount() == 0);
}

SLANG_UNIT_TEST(serializedArrayEmptyViewIsStillAView)
{
    // The case that motivated storing `_isView` instead of deriving it. Inferring "view"
    // from "`_count` non-zero and `_owned` empty" classified a zero-length view as owned,
    // which would let the mutators -- all of which assert `!isView()` -- run on it.
    const Int64 backing[1] = {7};
    SerializedArray<Int64> emptyView;
    emptyView.adoptView(backing, 0);

    SLANG_CHECK(emptyView.isView());
    SLANG_CHECK(emptyView.getCount() == 0);

    // And it survives a copy as a view rather than silently becoming writable.
    SerializedArray<Int64> copied(emptyView);
    SLANG_CHECK(copied.isView());
}

SLANG_UNIT_TEST(serializedArrayMakeOwnedDetachesFromTheBlob)
{
    List<Int64> blob;
    blob.add(5);
    blob.add(6);
    blob.add(7);

    SerializedArray<Int64> array;
    array.adoptView(blob.getBuffer(), blob.getCount());
    SLANG_CHECK(array.isView());

    array.makeOwned();
    SLANG_CHECK(!array.isView());
    SLANG_CHECK(array.getBuffer() != blob.getBuffer());
    SLANG_CHECK(array.getCount() == 3);

    // The point of makeOwned: the data must survive the blob going away. Overwriting the
    // original storage stands in for freeing it, and would be visible through a view.
    for (Index i = 0; i < blob.getCount(); ++i)
        blob[i] = -1;

    SLANG_CHECK(array[0] == 5);
    SLANG_CHECK(array[1] == 6);
    SLANG_CHECK(array[2] == 7);
}

SLANG_UNIT_TEST(serializedArrayMakeOwnedOnOwnedIsANoOp)
{
    SerializedArray<Int64> array;
    _fillOwned(array, 5);
    const Int64* before = array.getBuffer();

    array.makeOwned();

    SLANG_CHECK(!array.isView());
    SLANG_CHECK(array.getBuffer() == before);
    SLANG_CHECK(_matchesPattern(array, 5));
}

SLANG_UNIT_TEST(serializedArrayBeginOwnedDiscardsAView)
{
    const Int64 backing[3] = {1, 2, 3};
    SerializedArray<Int64> array;
    array.adoptView(backing, 3);
    SLANG_CHECK(array.isView());

    array.beginOwned(0);
    SLANG_CHECK(!array.isView());
    SLANG_CHECK(array.getCount() == 0);

    // Now writable, and unrelated to the blob it used to point at.
    array.add(Int64(42));
    SLANG_CHECK(array.getCount() == 1);
    SLANG_CHECK(array[0] == Int64(42));
    SLANG_CHECK(array.getBuffer() != backing);
}

SLANG_UNIT_TEST(serializedArraySelfAssignmentPreservesContents)
{
    // Both assignment operators short-circuit on self-assignment. Without that guard the
    // move path would move `_owned` into itself and then reset the source -- which is this
    // same object -- clearing it.
    SerializedArray<Int64> array;
    _fillOwned(array, 6);

    SerializedArray<Int64>* alias = &array;
    array = *alias;
    SLANG_CHECK(_matchesPattern(array, 6));

    array = static_cast<SerializedArray<Int64>&&>(*alias);
    SLANG_CHECK(_matchesPattern(array, 6));
}


namespace
{

/// A record covering each way the bulk scalar paths can end.
///
/// `empty` is refused by `canReadContiguousScalars` and falls back to the per-element
/// loop; `single` takes the fast path with nothing left to copy after the validating first
/// read; `many` takes it with a real bulk copy behind it; `borrowed` goes through
/// `tryBorrowContiguousScalars` instead and comes back as a view; `narrow` is a second
/// element width. The scalar fields on either side are sentinels.
struct BulkReadProbe
{
    Int64 leading = 0;
    List<Int64> empty;
    List<Int64> single;
    List<Int64> many;
    SerializedArray<Int64> borrowed;
    List<uint8_t> narrow;
    Int64 trailing = 0;
};

template<typename S>
void serialize(S const& serializer, BulkReadProbe& value)
{
    SLANG_SCOPED_SERIALIZER_STRUCT(serializer);
    serialize(serializer, value.leading);
    serialize(serializer, value.empty);
    serialize(serializer, value.single);
    serialize(serializer, value.many);
    serialize(serializer, value.borrowed);
    serialize(serializer, value.narrow);
    serialize(serializer, value.trailing);
}

/// Writes `in` to a fossil blob and reads it back into `out`, returning the blob so the
/// caller can keep it alive -- `out.borrowed` may be a view into it.
ComPtr<ISlangBlob> _roundTripThroughFossil(BulkReadProbe& in, BulkReadProbe& out)
{
    BlobBuilder blobBuilder;
    {
        Fossil::SerialWriter writer(blobBuilder);
        Serializer<Fossil::SerialWriter, void> serializer(&writer);
        serialize(serializer, in);
    }

    ComPtr<ISlangBlob> blob;
    blobBuilder.writeToBlob(blob.writeRef());

    Fossil::AnyValPtr rootValPtr = Fossil::getRootValue(blob);
    SLANG_ASSERT(rootValPtr);

    Fossil::SerialReader::ReadContext readContext;
    Fossil::SerialReader reader(readContext, rootValPtr);
    Serializer<Fossil::SerialReader, void> serializer(&reader);
    serialize(serializer, out);

    return blob;
}

} // namespace

// Checks that the bulk scalar read paths are actually taken, and round-trip, at each of
// the sizes that decide whether they are.
//
// Everything else in the suite reaches `tryReadContiguousScalars` and
// `tryBorrowContiguousScalars` through whole-module round trips, where every array is
// large, the fast path is always taken, and the result is checked only for its own
// contents. Nothing covered the refusal (an empty array, which `canReadContiguousScalars`
// declines, leaving the per-element loop to do it), the boundary (one element, so the
// validating first read consumes the whole array and there is nothing left to copy), or
// the fact that the borrowing path is reached at all -- make `canReadContiguousScalars`
// refuse everything and the module round trips still pass, only slower.
//
// What this deliberately does *not* claim to catch is an error in the `count - 1`
// arithmetic itself. That cursor is local to the container being read, and the container
// is consumed whole, so nothing downstream reads from it: stepping one element too far
// leaves every value correct and writes a single element past the destination buffer.
// That is a memory error, not a wrong answer, and it belongs to the sanitizer build --
// where every module load already exercises the same code.
SLANG_UNIT_TEST(serializedArrayBulkScalarPathsAreTakenAtEverySize)
{
    BulkReadProbe in;
    in.leading = 0x0102030405060708ll;
    in.single.add(-99);
    for (Index i = 0; i < 37; ++i)
        in.many.add(Int64(i * 7 + 1));
    in.borrowed.beginOwned(0);
    for (Index i = 0; i < 11; ++i)
        in.borrowed.add(Int64(i * 3 - 5));
    for (Index i = 0; i < 5; ++i)
        in.narrow.add(uint8_t(i * 13 + 2));
    in.trailing = 0x1112131415161718ll;

    BulkReadProbe out;
    ComPtr<ISlangBlob> blob = _roundTripThroughFossil(in, out);

    SLANG_CHECK(out.leading == in.leading);
    SLANG_CHECK(out.trailing == in.trailing);

    SLANG_CHECK(out.empty.getCount() == 0);

    SLANG_CHECK(out.single.getCount() == in.single.getCount());
    for (Index i = 0; i < out.single.getCount(); ++i)
        SLANG_CHECK(out.single[i] == in.single[i]);

    SLANG_CHECK(out.many.getCount() == in.many.getCount());
    for (Index i = 0; i < out.many.getCount(); ++i)
        SLANG_CHECK(out.many[i] == in.many[i]);

    SLANG_CHECK(out.narrow.getCount() == in.narrow.getCount());
    for (Index i = 0; i < out.narrow.getCount(); ++i)
        SLANG_CHECK(out.narrow[i] == in.narrow[i]);

    SLANG_CHECK(out.borrowed.getCount() == in.borrowed.getCount());
    for (Index i = 0; i < out.borrowed.getCount(); ++i)
        SLANG_CHECK(out.borrowed[i] == in.borrowed[i]);

    // The borrowing read is the one with a lifetime obligation, so pin that it really did
    // borrow: an owned copy here would mean `tryBorrowContiguousScalars` was never taken
    // and the checks above said nothing about it.
    SLANG_CHECK(out.borrowed.isView());

    // Detach before the blob goes, which is the obligation `tryBorrowContiguousScalars`
    // hands its caller.
    out.borrowed.makeOwned();
    blob = nullptr;
    SLANG_CHECK(out.borrowed.getCount() == in.borrowed.getCount());
}
