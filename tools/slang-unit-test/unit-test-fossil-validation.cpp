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
bool isAccepted(BlobBuilder const& blob, Fossil::Trust trust = Fossil::Trust::Untrusted)
{
    try
    {
        return Fossil::getRootValue(blob.getData(), blob.getSize(), trust) != Fossil::AnyValPtr();
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
BlobBuilder makeStructBlobImpl()
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

/// Build the record blob with a chosen record `kind`, so `Tuple` is exercised and
/// not only `Struct`.
///
BlobBuilder makeRecordBlobOfKind(FossilizedValKind kind)
{
    auto blob = makeStructBlobImpl();
    blob.putU32(32, uint32_t(kind));
    return blob;
}

/// Build the struct blob. Kept as a named wrapper so existing callers read the
/// same as before.
///
BlobBuilder makeStructBlob()
{
    return makeStructBlobImpl();
}

/// Build a well-formed blob whose root variant holds a pointer to a `uint32`, or
/// an optional holding one when `kind` is `OptionalObj`.
///
/// The two kinds share a layout shape and a walk path: both store a relative
/// pointer in place and describe what it points at with the layout's element
/// layout, and in both cases the target is reached as a pointer target rather
/// than in place.
///
///   0..31   header
///   32..39  pointer-like layout (kind, element-layout pointer)
///   40..43  the element's layout (kind = UInt32)
///   44..47  the variant's content-layout pointer
///   48..51  the variant's content: the pointer itself
///   52..55  the pointed-to value
///
BlobBuilder makePointerLikeBlob(FossilizedValKind kind)
{
    BlobBuilder blob;
    blob.resize(56);
    blob.putHeader(48);

    blob.putU32(32, uint32_t(kind));
    blob.putRelativePtr(36, 40);

    blob.putU32(40, uint32_t(FossilizedValKind::UInt32));

    blob.putRelativePtr(44, 32);
    blob.putRelativePtr(48, 52);
    blob.putU32(52, 0xABCDEF01);
    return blob;
}

/// Build a well-formed blob whose object graph contains a cycle: a struct whose
/// second field is a pointer back to the struct itself.
///
/// The walk memoizes visited locations and runs off a work list precisely so a
/// graph like this terminates instead of looping forever. The struct needs a
/// leading field so the back-pointer sits at a non-zero offset -- a pointer whose
/// stored offset is zero is how the format spells null, so a self-pointer stored
/// at the struct's own address could not express the cycle.
///
///   0..31   header
///   32..39  record layout (kind, field count = 2)
///   40..47  field 0: layout pointer, offset 0
///   48..55  field 1: layout pointer, offset 4
///   56..59  field 0's layout (kind = UInt32)
///   60..67  field 1's layout: a pointer back to the record layout
///   68..71  the variant's content-layout pointer
///   72..79  the record: a `uint32`, then the pointer back to the record
///
BlobBuilder makeCyclicBlob()
{
    BlobBuilder blob;
    blob.resize(80);
    blob.putHeader(72);

    blob.putU32(32, uint32_t(FossilizedValKind::Struct));
    blob.putU32(36, 2);

    blob.putRelativePtr(40, 56);
    blob.putU32(44, 0);
    blob.putRelativePtr(48, 60);
    blob.putU32(52, 4);

    blob.putU32(56, uint32_t(FossilizedValKind::UInt32));

    blob.putU32(60, uint32_t(FossilizedValKind::Ptr));
    blob.putRelativePtr(64, 32);

    blob.putRelativePtr(68, 32);
    blob.putU32(72, 0xABCDEF01);
    blob.putRelativePtr(76, 72);
    return blob;
}

/// Build a well-formed blob whose root variant holds a dictionary of one
/// key/value pair.
///
/// Dictionaries share `_visitContainerObj` with arrays, but their elements are
/// records rather than scalars, so this is what exercises a container element
/// that is itself an aggregate.
///
///   0..31   header
///   32..43  container layout (kind, element-layout pointer, element stride)
///   44..51  the element's record layout (kind, field count = 2)
///   52..67  the two field entries: layout pointer and offset for each
///   68..71  both fields' layout (kind = UInt32)
///   72..75  the variant's content-layout pointer
///   76..79  the variant's content: a pointer to the dictionary object
///   80..83  the element count, which sits *before* the object
///   84..91  the single key/value pair
///
BlobBuilder makeDictionaryBlob()
{
    BlobBuilder blob;
    blob.resize(92);
    blob.putHeader(76);

    blob.putU32(32, uint32_t(FossilizedValKind::DictionaryObj));
    blob.putRelativePtr(36, 44);
    blob.putU32(40, 8);

    blob.putU32(44, uint32_t(FossilizedValKind::Struct));
    blob.putU32(48, 2);
    blob.putRelativePtr(52, 68);
    blob.putU32(56, 0);
    blob.putRelativePtr(60, 68);
    blob.putU32(64, 4);

    blob.putU32(68, uint32_t(FossilizedValKind::UInt32));

    blob.putRelativePtr(72, 32);
    blob.putRelativePtr(76, 84);
    blob.putU32(80, 1);
    blob.putU32(84, 0xAAAAAAAA);
    blob.putU32(88, 0xBBBBBBBB);
    return blob;
}

/// Build a well-formed blob whose root variant holds a struct whose second field
/// is a string.
///
/// Every other well-formed blob bottoms out in a scalar leaf. This one exercises
/// the walk's core descent: a field reached in place, found to be an object kind,
/// its relative pointer read, and the object re-queued as a pointer target.
///
///   0..31   header
///   32..39  record layout (kind, field count = 2)
///   40..55  the two field entries
///   56..59  field 0's layout (kind = UInt32)
///   60..63  field 1's layout (kind = StringObj)
///   64..67  the variant's content-layout pointer
///   68..75  the record: a `uint32`, then a pointer to the string object
///   76..79  the string's length
///   80..83  the string's bytes and terminator
///
BlobBuilder makeRecordWithObjectFieldBlob()
{
    BlobBuilder blob;
    blob.resize(84);
    blob.putHeader(68);

    blob.putU32(32, uint32_t(FossilizedValKind::Struct));
    blob.putU32(36, 2);
    blob.putRelativePtr(40, 56);
    blob.putU32(44, 0);
    blob.putRelativePtr(48, 60);
    blob.putU32(52, 4);

    blob.putU32(56, uint32_t(FossilizedValKind::UInt32));
    blob.putU32(60, uint32_t(FossilizedValKind::StringObj));

    blob.putRelativePtr(64, 32);
    blob.putU32(68, 0xABCDEF01);
    blob.putRelativePtr(72, 80);

    blob.putU32(76, 3);
    blob.bytes[80] = 'a';
    blob.bytes[81] = 'b';
    blob.bytes[82] = 'c';
    blob.bytes[83] = 0;
    return blob;
}

/// Build a well-formed blob whose root variant holds a struct whose second field
/// is itself a variant.
///
/// Every other blob makes the variant the *root*, which the walk enters directly
/// through `validateFrom`, queueing the variant's content. A variant reached from
/// inside the graph takes two different arms instead -- read the in-place pointer
/// and re-queue the object as a pointer target, then descend through the object's
/// preceding content-layout word. Real serialized modules nest variants inside
/// structs and arrays routinely, so both arms run in production.
///
///   0..31   header
///   32..39  record layout (kind, field count = 2)
///   40..55  the two field entries
///   56..59  field 0's layout (kind = UInt32), reused as the nested content's
///   60..63  field 1's layout (kind = VariantObj)
///   64..67  the root variant's content-layout pointer
///   68..75  the record: a `uint32`, then a pointer to the nested variant object
///   76..79  the nested variant's content-layout pointer
///   80..83  the nested variant's content
///
BlobBuilder makeRecordWithVariantFieldBlob()
{
    BlobBuilder blob;
    blob.resize(84);
    blob.putHeader(68);

    blob.putU32(32, uint32_t(FossilizedValKind::Struct));
    blob.putU32(36, 2);
    blob.putRelativePtr(40, 56);
    blob.putU32(44, 0);
    blob.putRelativePtr(48, 60);
    blob.putU32(52, 4);

    blob.putU32(56, uint32_t(FossilizedValKind::UInt32));
    blob.putU32(60, uint32_t(FossilizedValKind::VariantObj));

    blob.putRelativePtr(64, 32);
    blob.putU32(68, 0xABCDEF01);
    blob.putRelativePtr(72, 80);

    blob.putRelativePtr(76, 56);
    blob.putU32(80, 0xABCDEF01);
    return blob;
}

/// Build a blob that is entirely in bounds but whose reachable location set is
/// quadratic in its own size.
///
/// Every location here passes the range checks, so nothing is rejected on those
/// grounds. The blow-up comes from the memoization key: an array of `count`
/// elements whose element layout is a record with `fieldCount` fields that all
/// sit at offset 0 but name *distinct* layouts. Each element reaches `fieldCount`
/// distinct keys at one address -- distinct only because the layout offset
/// differs -- so the walk reaches `count * fieldCount` of them.
///
///   0..31             header
///   32..43            container layout (kind, element-layout pointer, stride)
///   44..51            the element's record layout (kind, field count)
///   52..               one entry per field: layout pointer, offset 0
///   fieldLayouts..     one distinct `UInt32` layout per field
///   ..                 the variant's content-layout pointer and content
///   ..                 the element count, then the elements themselves
///
BlobBuilder makeQuadraticBlob(Size count, Size fieldCount)
{
    const Size fieldEntries = 52;
    const Size fieldLayouts = fieldEntries + 8 * fieldCount;
    const Size variantLayoutPtr = fieldLayouts + 4 * fieldCount;
    const Size variantContent = variantLayoutPtr + 4;
    const Size countWord = variantContent + 4;
    const Size elements = countWord + 4;

    BlobBuilder blob;
    blob.resize(elements + 4 * count);
    blob.putHeader(variantContent);

    blob.putU32(32, uint32_t(FossilizedValKind::ArrayObj));
    blob.putRelativePtr(36, 44);
    blob.putU32(40, 4);

    blob.putU32(44, uint32_t(FossilizedValKind::Struct));
    blob.putU32(48, uint32_t(fieldCount));

    for (Size i = 0; i < fieldCount; ++i)
    {
        blob.putRelativePtr(fieldEntries + 8 * i, fieldLayouts + 4 * i);
        blob.putU32(fieldEntries + 8 * i + 4, 0);
        blob.putU32(fieldLayouts + 4 * i, uint32_t(FossilizedValKind::UInt32));
    }

    blob.putRelativePtr(variantLayoutPtr, 32);
    blob.putRelativePtr(variantContent, elements);
    blob.putU32(countWord, uint32_t(count));
    return blob;
}

/// Build a well-formed blob whose root variant holds a pointer to an optional.
///
/// An optional reached *through a pointer* holds no storage of its own: the walk
/// re-queues at the same address under the optional's element layout. That arm
/// is only reachable via a shape like `Ptr<Optional<T>>`, an array of optionals,
/// or a nested optional -- never by an optional stored in place.
///
///   0..31   header
///   32..39  pointer layout (kind, element-layout pointer -> the optional layout)
///   40..47  optional layout (kind, element-layout pointer -> the scalar layout)
///   48..51  the scalar layout (kind = UInt32)
///   52..55  the variant's content-layout pointer
///   56..59  the variant's content: the pointer
///   60..63  the value, which the optional aliases rather than points at
///
BlobBuilder makePointerToOptionalBlob()
{
    BlobBuilder blob;
    blob.resize(64);
    blob.putHeader(56);

    blob.putU32(32, uint32_t(FossilizedValKind::Ptr));
    blob.putRelativePtr(36, 40);

    blob.putU32(40, uint32_t(FossilizedValKind::OptionalObj));
    blob.putRelativePtr(44, 48);

    blob.putU32(48, uint32_t(FossilizedValKind::UInt32));

    blob.putRelativePtr(52, 32);
    blob.putRelativePtr(56, 60);
    blob.putU32(60, 0xABCDEF01);
    return blob;
}

/// Build the scalar blob with a chosen scalar `kind`, so the differing in-place
/// sizes are exercised rather than only the 4-byte one.
///
BlobBuilder makeScalarBlobOfKind(FossilizedValKind kind)
{
    auto blob = makeScalarBlob();
    blob.putU32(32, uint32_t(kind));
    return blob;
}
/// Build a well-formed blob whose root variant holds an array of two strings.
///
/// The object-kind descent has only ever been reached through a *record field*.
/// Here it is reached through a container stride instead, so the element address
/// arithmetic and the descent interact.
///
///   0..31   header
///   32..43  array layout (kind, element-layout pointer, stride = 4)
///   44..47  the element layout (kind = StringObj)
///   48..51  the variant's content-layout pointer
///   52..55  the variant's content: a pointer to the array object
///   56..59  the element count
///   60..67  the two elements, each a pointer to its string object
///   68..71  first string length, 72..75 its bytes
///   76..79  second string length, 80..83 its bytes
///
BlobBuilder makeArrayOfStringsBlob()
{
    BlobBuilder blob;
    blob.resize(84);
    blob.putHeader(52);

    blob.putU32(32, uint32_t(FossilizedValKind::ArrayObj));
    blob.putRelativePtr(36, 44);
    blob.putU32(40, 4);

    blob.putU32(44, uint32_t(FossilizedValKind::StringObj));

    blob.putRelativePtr(48, 32);
    blob.putRelativePtr(52, 60);
    blob.putU32(56, 2);

    blob.putRelativePtr(60, 72);
    blob.putRelativePtr(64, 80);

    blob.putU32(68, 3);
    blob.bytes[72] = 'a';
    blob.bytes[73] = 'b';
    blob.bytes[74] = 'c';
    blob.bytes[75] = 0;

    blob.putU32(76, 3);
    blob.bytes[80] = 'x';
    blob.bytes[81] = 'y';
    blob.bytes[82] = 'z';
    blob.bytes[83] = 0;
    return blob;
}

} // namespace

SLANG_UNIT_TEST(fossilValidationBoundsTotalWork)
{
    // Memoizing on (data, layout, form) stops the walk looping on a cyclic graph,
    // but it does not bound how many distinct locations exist: one address paired
    // with many distinct layouts multiplies out. Without a work cap this blob is
    // accepted, after churning through count * fieldCount locations -- which for a
    // slightly larger blob is enough memory to take the process down. Since the
    // walk exists to make hostile input safe to load, running out of memory on it
    // is a denial of service rather than an acceptable outcome, so it must be
    // rejected instead.
    SLANG_CHECK(!isAccepted(makeQuadraticBlob(768, 768)));

    // The same shape at a size the cap does allow still validates, so the bound
    // does not reject ordinary blobs that merely share layouts between fields.
    SLANG_CHECK(isAccepted(makeQuadraticBlob(4, 4)));
}

SLANG_UNIT_TEST(fossilValidationHandlesOptionalAsPointerTarget)
{
    SLANG_CHECK(isAccepted(makePointerToOptionalBlob()));

    // The pointer that reaches the optional leaves the blob.
    auto blob = makePointerToOptionalBlob();
    blob.putI32(56, 0x7F000000);
    SLANG_CHECK(!isAccepted(blob));

    // The optional names no element layout, so nothing describes the value it
    // aliases.
    blob = makePointerToOptionalBlob();
    blob.putI32(44, 0);
    SLANG_CHECK(!isAccepted(blob));

    // The aliased value runs off the end.
    blob = makePointerToOptionalBlob();
    blob.resize(62);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationChecksScalarSizes)
{
    // Only 4-byte scalars flowed through the other blobs, so a wrong in-place
    // size for a wider or narrower kind would under-check without being noticed.
    // The scalar blob has exactly four bytes of content, so an 8-byte kind must
    // be rejected and the narrow kinds must be accepted.
    SLANG_CHECK(!isAccepted(makeScalarBlobOfKind(FossilizedValKind::Int64)));
    SLANG_CHECK(!isAccepted(makeScalarBlobOfKind(FossilizedValKind::Float64)));

    SLANG_CHECK(isAccepted(makeScalarBlobOfKind(FossilizedValKind::Bool)));
    SLANG_CHECK(isAccepted(makeScalarBlobOfKind(FossilizedValKind::Int16)));
    SLANG_CHECK(isAccepted(makeScalarBlobOfKind(FossilizedValKind::Float32)));
}

SLANG_UNIT_TEST(fossilValidationAcceptsWellFormedBlobs)
{
    SLANG_CHECK(isAccepted(makeScalarBlob()));
    SLANG_CHECK(isAccepted(makeArrayBlob()));
    SLANG_CHECK(isAccepted(makeStringBlob()));
    SLANG_CHECK(isAccepted(makeStructBlob()));
    SLANG_CHECK(isAccepted(makePointerLikeBlob(FossilizedValKind::Ptr)));
    SLANG_CHECK(isAccepted(makePointerLikeBlob(FossilizedValKind::OptionalObj)));
    SLANG_CHECK(isAccepted(makeDictionaryBlob()));
    SLANG_CHECK(isAccepted(makeRecordWithObjectFieldBlob()));
    SLANG_CHECK(isAccepted(makeRecordWithVariantFieldBlob()));
    SLANG_CHECK(isAccepted(makeArrayOfStringsBlob()));

    // `Tuple` shares its arm with `Struct`, so this is coverage against a future
    // change that special-cases one of them rather than a distinct branch today.
    SLANG_CHECK(isAccepted(makeRecordBlobOfKind(FossilizedValKind::Tuple)));
}

SLANG_UNIT_TEST(fossilValidationHandlesContainerOfObjects)
{
    // An element that is an object kind, reached through a container stride rather
    // than a record field, so the stride arithmetic and the object descent
    // interact.
    auto blob = makeArrayOfStringsBlob();
    blob.putI32(64, 0x7F000000);
    SLANG_CHECK(!isAccepted(blob));

    // The second element's string overruns, which is only reachable if the walk
    // strided to that element correctly.
    blob = makeArrayOfStringsBlob();
    blob.putU32(76, 1000);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationAcceptsEmptyString)
{
    // The smallest legal string: no bytes, just the terminator. Its length check
    // reduces to one byte, which is the boundary the `stringSize + 1` bound sits
    // on.
    auto blob = makeStringBlob();
    blob.putU32(44, 0);
    blob.bytes[48] = 0;
    SLANG_CHECK(isAccepted(blob));

    // Zero length with a non-zero terminator is still malformed.
    blob = makeStringBlob();
    blob.putU32(44, 0);
    blob.bytes[48] = 'x';
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationHandlesNestedVariant)
{
    // A nested variant whose in-place pointer leaves the blob.
    auto blob = makeRecordWithVariantFieldBlob();
    blob.putI32(72, 0x7F000000);
    SLANG_CHECK(!isAccepted(blob));

    // A nested variant whose content-layout word -- which sits before the object,
    // and so is only reached by descending into it -- leaves the blob.
    blob = makeRecordWithVariantFieldBlob();
    blob.putI32(76, 0x7F000000);
    SLANG_CHECK(!isAccepted(blob));

    // A nested variant whose content whose layout says `UInt32` runs off the end.
    blob = makeRecordWithVariantFieldBlob();
    blob.resize(82);
    SLANG_CHECK(!isAccepted(blob));

    // A variant with no content layout describes no content, so there is nothing
    // to walk and nothing to reject.
    blob = makeRecordWithVariantFieldBlob();
    blob.putI32(76, 0);
    SLANG_CHECK(isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationSkipsTheWalkWhenTrusted)
{
    // The core module is loaded as `Trusted` so that startup does not pay for the
    // walk. Nothing else pins that `Trusted` actually skips it: every other test
    // goes through the `Untrusted` arm, so a mis-wired gate that validated the
    // core module anyway would silently bring the startup cost back.
    //
    // The corruption has to be one that only the walk catches. An out-of-bounds
    // *root* pointer would not work here: `getRootValue` hands the root variant to
    // `getVariantContentPtr`, which reads the content-layout word stored before it,
    // so skipping the walk would make the test itself perform the wild read this
    // change exists to prevent. A string whose length runs off the end keeps every
    // pointer `getRootValue` follows inside the blob, while still being rejected by
    // `_visitStringObj`'s extent check.
    //
    auto blob = makeStringBlob();
    blob.putU32(44, 0x7F000000);

    SLANG_CHECK(!isAccepted(blob, Fossil::Trust::Untrusted));
    SLANG_CHECK(isAccepted(blob, Fossil::Trust::Trusted));
}

SLANG_UNIT_TEST(fossilValidationAcceptsEmptyRecord)
{
    // A record with no fields consults no field entry, so the trailing field array
    // is empty and the walk has nothing to descend into. This is the record-side
    // counterpart of the empty-string and empty-container boundaries.
    auto blob = makeStructBlob();
    blob.putU32(36, 0);
    SLANG_CHECK(isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationAcceptsEmptyContainer)
{
    // A container with no elements skips the extent and element walk entirely, so
    // neither the stride nor the element layout is consulted.
    auto blob = makeArrayBlob();
    blob.putU32(56, 0);
    SLANG_CHECK(isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationRejectsMalformedAggregateElements)
{
    // A dictionary whose element records run past the end of the blob, which the
    // extent check has to catch before any field of them is walked.
    auto blob = makeDictionaryBlob();
    blob.putU32(80, 4);
    SLANG_CHECK(!isAccepted(blob));

    // A dictionary element whose second field is placed past the element itself.
    // This only fails if field offsets are applied relative to the element, which
    // is the arithmetic a container of aggregates exercises.
    blob = makeDictionaryBlob();
    blob.putU32(64, 0xFFF0);
    SLANG_CHECK(!isAccepted(blob));

    // A record field that is an object kind, whose pointer leaves the blob. The
    // walk has to follow the field in place, see the object kind, and range-check
    // the target it points at.
    blob = makeRecordWithObjectFieldBlob();
    blob.putI32(72, 0x7F000000);
    SLANG_CHECK(!isAccepted(blob));

    // The same field's string object, with a length that overruns. Reaching this
    // check at all requires the descent through the record to have worked.
    blob = makeRecordWithObjectFieldBlob();
    blob.putU32(76, 1000);
    SLANG_CHECK(!isAccepted(blob));
}

SLANG_UNIT_TEST(fossilValidationHandlesPointerAndOptional)
{
    for (auto kind : {FossilizedValKind::Ptr, FossilizedValKind::OptionalObj})
    {
        // A pointer whose target is outside the blob.
        auto blob = makePointerLikeBlob(kind);
        blob.putI32(48, 0x7F000000);
        SLANG_CHECK(!isAccepted(blob));

        // A layout that names no element layout, so nothing describes what the
        // pointer points at.
        blob = makePointerLikeBlob(kind);
        blob.putI32(36, 0);
        SLANG_CHECK(!isAccepted(blob));

        // A target that is in bounds but whose value runs off the end.
        blob = makePointerLikeBlob(kind);
        blob.putRelativePtr(48, 54);
        SLANG_CHECK(!isAccepted(blob));
    }

    // A null pointer is legal for both kinds -- for an optional it is how absence
    // is spelled -- and must be accepted rather than followed.
    for (auto kind : {FossilizedValKind::Ptr, FossilizedValKind::OptionalObj})
    {
        auto blob = makePointerLikeBlob(kind);
        blob.putI32(48, 0);
        SLANG_CHECK(isAccepted(blob));
    }
}

SLANG_UNIT_TEST(fossilValidationTerminatesOnCyclicGraph)
{
    // Accepting this at all is the assertion: without the visited set the walk
    // would follow the back-pointer forever.
    SLANG_CHECK(isAccepted(makeCyclicBlob()));

    // The cycle must not mask a bad location either -- corrupting the
    // back-pointer still has to be caught.
    auto blob = makeCyclicBlob();
    blob.putI32(76, 0x7F000000);
    SLANG_CHECK(!isAccepted(blob));
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
