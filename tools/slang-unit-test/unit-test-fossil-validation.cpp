// unit-test-fossil-validation.cpp

#include "core/slang-exception.h"
#include "slang/slang-fossil.h"
#include "unit-test/slang-unit-test.h"

#include <string.h>

using namespace Slang;

#if SLANG_ENABLE_VALIDATION_FOSSIL

//
// The fossil validation walk exists to *reject* malformed serialized blobs, so
// the interesting cases are the negative ones. These tests build a small blob by
// hand, check that it is accepted, and then corrupt one field at a time and check
// that each corruption is rejected.
//
// The blob is hand-built rather than produced by `SerialWriter` because the point
// is to control individual bytes -- an offset, a count, a stride -- which is
// exactly what a hostile file gets to choose and what a writer would never emit.
//
// These tests only exist when validation is compiled in; with the option off,
// `Fossil::getRootValue` does no walk and there is nothing to test.
//

namespace
{

/// Builds a fossil blob one field at a time, addressed by byte offset.
///
/// Every write is placed at an explicit offset rather than appended, because the
/// tests are written against a known layout and need to corrupt one field of it
/// without disturbing the rest.
///
struct BlobBuilder
{
    List<uint8_t> bytes;

    /// Set the blob's total length in bytes, zero-filling any bytes added.
    void resize(Size size) { bytes.setCount(Index(size)); }

    /// Write a 32-bit unsigned value at `offset`.
    void putU32(Size offset, uint32_t value) { memcpy(bytes.getBuffer() + offset, &value, 4); }

    /// Write a 32-bit signed value at `offset`. Used for raw relative-pointer
    /// offsets, including deliberately malformed ones that `putRelativePtr` could
    /// not express.
    void putI32(Size offset, int32_t value) { memcpy(bytes.getBuffer() + offset, &value, 4); }

    /// Write a 64-bit unsigned value at `offset`.
    void putU64(Size offset, uint64_t value) { memcpy(bytes.getBuffer() + offset, &value, 8); }

    /// Write the relative pointer stored at `at` so that it targets `target`.
    ///
    /// Fossil pointers are measured from the address of the pointer itself, so the
    /// stored value is the difference between the two offsets rather than `target`.
    void putRelativePtr(Size at, Size target) { putI32(at, int32_t(Int64(target) - Int64(at))); }

    /// Write the 32-byte fossil header, pointing its root value at `rootValueOffset`.
    ///
    /// The header is magic bytes, a total size, flags, and a relative pointer to the
    /// root value. The total size is written as zero on purpose: `SerialWriter`
    /// never back-patches that field, so every blob Slang emits reports its own size
    /// as zero and a faithful test blob has to do the same.
    void putHeader(Size rootValueOffset)
    {
        memcpy(bytes.getBuffer(), Fossil::Header::kMagic, sizeof(Fossil::Header::kMagic));
        putU64(16, 0);
        putU32(24, 0);
        putRelativePtr(28, rootValueOffset);
    }

    /// Return a pointer to the start of the blob.
    void const* getData() const { return bytes.getBuffer(); }

    /// Return the blob's length in bytes.
    Size getSize() const { return Size(bytes.getCount()); }
};

/// Return true if `Fossil::getRootValue` accepts this blob.
///
/// Validation failures surface as a thrown `InternalError`, because
/// `SLANG_UNEXPECTED` routes through `handleSignal`.
///
bool isAccepted(BlobBuilder const& blob)
{
    try
    {
        return Fossil::getRootValue(blob.getData(), blob.getSize()) != Fossil::AnyValPtr();
    }
    catch (const InternalError&)
    {
        return false;
    }
}

/// Build the smallest well-formed blob: a root variant holding a single `uint32`.
///
///   0..31   header
///   32..35  layout of the content (kind = UInt32)
///   36..39  the variant's content-layout pointer, which sits *before* the variant
///   40..43  the variant's content
///
BlobBuilder makeScalarBlob()
{
    BlobBuilder blob;
    blob.resize(44);
    blob.putHeader(40);
    blob.putU32(32, uint32_t(FossilizedValKind::UInt32));
    blob.putRelativePtr(36, 32);
    blob.putU32(40, 0xABCDEF01);
    return blob;
}

/// Build a well-formed blob whose root variant holds a one-element array.
///
///   0..31   header
///   32..43  array layout (kind, element-layout pointer, element stride)
///   44..47  element layout (kind = UInt32)
///   48..51  the variant's content-layout pointer
///   52..55  the variant's content: a pointer to the array object
///   56..59  the array's element count, which sits *before* the array object
///   60..63  the single element
///
BlobBuilder makeArrayBlob()
{
    BlobBuilder blob;
    blob.resize(64);
    blob.putHeader(52);

    blob.putU32(32, uint32_t(FossilizedValKind::ArrayObj));
    blob.putRelativePtr(36, 44);
    blob.putU32(40, 4);

    blob.putU32(44, uint32_t(FossilizedValKind::UInt32));

    blob.putRelativePtr(48, 32);
    blob.putRelativePtr(52, 60);
    blob.putU32(56, 1);
    blob.putU32(60, 0xABCDEF01);
    return blob;
}

/// Build a well-formed blob whose root variant holds a string.
///
///   0..31   header
///   32..35  layout of the content (kind = StringObj)
///   36..39  the variant's content-layout pointer
///   40..43  the variant's content: a pointer to the string object
///   44..47  the string's length, which sits *before* the string object
///   48..51  the string's bytes, followed by their terminator
///
BlobBuilder makeStringBlob()
{
    BlobBuilder blob;
    blob.resize(52);
    blob.putHeader(40);

    blob.putU32(32, uint32_t(FossilizedValKind::StringObj));
    blob.putRelativePtr(36, 32);
    blob.putRelativePtr(40, 48);

    blob.putU32(44, 3);
    blob.bytes[48] = 'a';
    blob.bytes[49] = 'b';
    blob.bytes[50] = 'c';
    blob.bytes[51] = 0;
    return blob;
}

/// Build a well-formed blob whose root variant holds a struct with one `uint32`
/// field.
///
///   0..31   header
///   32..39  record layout (kind, field count)
///   40..47  the single field's entry: its layout pointer and its offset
///   48..51  the field's layout (kind = UInt32)
///   52..55  the variant's content-layout pointer
///   56..59  the variant's content: the record, whose only field is at offset 0
///
BlobBuilder makeStructBlob()
{
    BlobBuilder blob;
    blob.resize(60);
    blob.putHeader(56);

    blob.putU32(32, uint32_t(FossilizedValKind::Struct));
    blob.putU32(36, 1);
    blob.putRelativePtr(40, 48);
    blob.putU32(44, 0);

    blob.putU32(48, uint32_t(FossilizedValKind::UInt32));

    blob.putRelativePtr(52, 32);
    blob.putU32(56, 0xABCDEF01);
    return blob;
}

} // namespace

SLANG_UNIT_TEST(fossilValidationAcceptsWellFormedBlobs)
{
    SLANG_CHECK(isAccepted(makeScalarBlob()));
    SLANG_CHECK(isAccepted(makeArrayBlob()));
    SLANG_CHECK(isAccepted(makeStringBlob()));
    SLANG_CHECK(isAccepted(makeStructBlob()));
}

SLANG_UNIT_TEST(fossilValidationRejectsMalformedString)
{
    // A stored length that runs past the end of the buffer.
    auto blob = makeStringBlob();
    blob.putU32(44, 1000);
    SLANG_CHECK(!isAccepted(blob));

    // A length that is in bounds but whose terminator is not zero. Consumers hand
    // the bytes out as a NUL-terminated slice, so without this check a reader
    // would run off the end of the string looking for a terminator.
    blob = makeStringBlob();
    blob.bytes[51] = 'X';
    SLANG_CHECK(!isAccepted(blob));

    // A length that places the terminator exactly one byte past the buffer.
    blob = makeStringBlob();
    blob.putU32(44, 4);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsMalformedRecord)
{
    // A field count whose field array runs past the end of the buffer.
    auto blob = makeStructBlob();
    blob.putU32(36, 0x0FFFFFFF);
    SLANG_CHECK(!isAccepted(blob));

    // A field whose layout pointer is null. Every field must name a layout,
    // because the walk cannot describe the field's data without one.
    blob = makeStructBlob();
    blob.putI32(40, 0);
    SLANG_CHECK(!isAccepted(blob));

    // A field placed past the end of the record's own storage.
    blob = makeStructBlob();
    blob.putU32(44, 0xFFFF);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsZeroContainerStride)
{
    // A zero stride with a non-zero element count. The stride divides the blob
    // size in the extent check, so this must be rejected rather than divide by
    // zero.
    auto blob = makeArrayBlob();
    blob.putU32(40, 0);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsOutOfBoundsRootValue)
{
    // A root pointer aimed far outside the blob, in both directions.
    auto blob = makeScalarBlob();
    blob.putI32(28, 0x7F000000);
    SLANG_CHECK(!isAccepted(blob));

    blob = makeScalarBlob();
    blob.putI32(28, -0x7F000000);
    SLANG_CHECK(!isAccepted(blob));

    // And one aimed just past the end, which is the case most likely to be read
    // as adjacent heap memory rather than to fault.
    blob = makeScalarBlob();
    blob.putRelativePtr(28, blob.getSize());
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsPointerCollidingWithNullSentinel)
{
    // A relative pointer whose target computes to exactly -1, one byte before the
    // blob. The walk represents "no pointer" as the offset -1, so this must not be
    // mistaken for a null pointer: the reader keys null off the raw stored offset
    // being zero, which this is not, so it would follow the pointer out of bounds
    // while the walk skipped everything behind it.
    auto blob = makeScalarBlob();
    blob.putI32(36, int32_t(-1 - 36));
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsBadLayoutKind)
{
    auto blob = makeScalarBlob();
    blob.putU32(32, 0xFFFFFFFF);
    SLANG_CHECK(!isAccepted(blob));

    // One past the last valid kind, which a bounds check written as `<` rather
    // than `<=` would let through.
    blob = makeScalarBlob();
    blob.putU32(32, uint32_t(FossilizedValKind::VariantObj) + 1);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsOversizedContainerExtent)
{
    // An element count that runs past the end of the blob.
    auto blob = makeArrayBlob();
    blob.putU32(56, 1000);
    SLANG_CHECK(!isAccepted(blob));

    // A count and stride whose product overflows a signed 64-bit value. Computing
    // the extent before checking it would wrap to a negative size, which widens
    // the bound instead of narrowing it and lets the extent through.
    blob = makeArrayBlob();
    blob.putU32(56, 0xFFFFFFFF);
    blob.putU32(40, 0xFFFFFFFF);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsTruncatedBlob)
{
    // A blob whose declared root value no longer fits once the buffer is cut short.
    auto blob = makeScalarBlob();
    blob.resize(42);
    SLANG_CHECK(!isAccepted(blob));
}

#endif // SLANG_ENABLE_VALIDATION_FOSSIL
