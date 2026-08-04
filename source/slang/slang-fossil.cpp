// slang-fossil.cpp
#include "slang-fossil.h"

namespace Slang
{
namespace Fossil
{

const char Fossil::Header::kMagic[16] = {
    '\xAB', // byte 0
    'f',    // byte 1
    'o',    // byte 2
    's',    // byte 3
    's',    // byte 4
    'i',    // byte 5
    'l',    // byte 6
    ' ',    // byte 7
    '1',    // byte 8
    '0',    // byte 9
    '0',    // byte 10
    '\xBB', // byte 11
    '\r',   // byte 12
    '\n',   // byte 13
    '\x1A', // byte 14
    '\n'    // byte 15
};

Fossil::AnyValPtr getRootValue(ISlangBlob* blob)
{
    return getRootValue(blob->getBufferPointer(), blob->getBufferSize());
}

#if SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS

/// Return the offset, in bytes, of `ptr` from the start of the blob at `base`.
///
/// `ptr` may legitimately land outside the blob, since it comes from following an
/// untrusted relative pointer, so the two are cast to integers before subtracting;
/// subtracting pointers into different objects would be undefined behavior.
///
static Int64 getOffsetFromBlobBase(void const* base, void const* ptr)
{
    return Int64(intptr_t(ptr)) - Int64(intptr_t(base));
}

/// Determine whether the byte range `[offset, offset + rangeSize)` lies entirely
/// within a blob of `blobSize` bytes.
///
static bool isRangeInBounds(Int64 offset, Int64 rangeSize, Int64 blobSize)
{
    SLANG_ASSERT(rangeSize >= 0);
    SLANG_ASSERT(blobSize >= 0);

    if (offset < 0)
        return false;

    // Subtract from the bound rather than adding to `offset`, so a large `offset`
    // cannot overflow.
    //
    return offset <= blobSize - rangeSize;
}

/// Validate that the root value of the blob at `data` (of `size` bytes), and the
/// layout describing its content, both lie inside that blob.
///
/// A variant stores its content layout in a relative pointer sitting *before* the
/// variant's own address, and `getVariantContentPtr` reads that pointer right away,
/// so it is that preceding word which must be in bounds.
///
/// This validates only the two hops needed to reach the root value; the rest of the
/// graph is still traversed unchecked.
///
static void validateRootValueIsInBounds(
    void const* data,
    Size size,
    FossilizedVariantObj* rootValueVariant)
{
    // `totalSizeIncludingHeader` is written as zero and never back-patched, so the
    // caller-supplied `size` is the only trustworthy extent.
    //
    Int64 blobSize = Int64(size);

    using ContentLayoutPtr = FossilizedPtr<FossilizedValLayout>;
    auto rootValueOffset = getOffsetFromBlobBase(data, rootValueVariant);
    SLANG_SERIALIZE_FOSSIL_VALIDATE(isRangeInBounds(
        rootValueOffset - Int64(sizeof(ContentLayoutPtr)),
        Int64(sizeof(ContentLayoutPtr)),
        blobSize));

    // A null layout is left for the caller to reject, since the `expect*ValOfType`
    // helpers already test for it before reading through it.
    //
    auto contentLayout = rootValueVariant->getContentLayout();
    if (contentLayout)
    {
        auto contentLayoutOffset = getOffsetFromBlobBase(data, contentLayout);
        SLANG_SERIALIZE_FOSSIL_VALIDATE(
            isRangeInBounds(contentLayoutOffset, Int64(sizeof(FossilizedValLayout)), blobSize));
    }
}

#endif

Fossil::AnyValPtr getRootValue(void const* data, Size size)
{
    if (!data)
    {
        SLANG_UNEXPECTED("bad format for fossil");
    }

    // There must be enough data to at least hold the header.
    //
    // (In practice there would need to be more data than
    // just the header, but checking this invariant is a start).
    //
    if (size < sizeof(Fossil::Header))
    {
        SLANG_UNEXPECTED("bad format for fossil");
    }

    // Once we've checked that there's enough data, we can read
    // the contents of the header.
    //
    auto header = reinterpret_cast<Fossil::Header const*>(data);

    // The "magic" bytes at the start of the header must be
    // what we expect (which is the contents of `Fossil::Header::kMagic`).
    //
    if (memcmp(header->magic, Fossil::Header::kMagic, sizeof(Fossil::Header::kMagic)) != 0)
    {
        SLANG_UNEXPECTED("bad format for fossil");
    }

    auto reportedSize = header->totalSizeIncludingHeader;
    if (reportedSize > size)
    {
        SLANG_UNEXPECTED("bad format for fossil");
    }

    auto rootValueVariant = header->rootValue.get();
    if (!rootValueVariant)
    {
        SLANG_UNEXPECTED("bad format for fossil");
    }

#if SLANG_SERIALIZE_FOSSIL_ENABLE_VALIDATION_CHECKS
    validateRootValueIsInBounds(data, size, rootValueVariant);
#endif

    return getVariantContentPtr(rootValueVariant);
}

} // namespace Fossil

Fossil::AnyValRef Fossil::ValRef<FossilizedContainerObjBase>::getElement(Index index) const
{
    SLANG_ASSERT(index >= 0);
    SLANG_ASSERT(index < getElementCount());

    auto containerLayout = getLayout();
    auto elementLayout = containerLayout->elementLayout.get();
    auto elementStride = containerLayout->elementStride;

    auto elementsPtr = (Byte*)getDataPtr();
    auto elementPtr = (void*)(elementsPtr + elementStride * index);
    return Fossil::AnyValRef(elementPtr, elementLayout);
}

FossilizedRecordElementLayout* FossilizedRecordLayout::getField(Index index) const
{
    SLANG_ASSERT(index >= 0);
    SLANG_ASSERT(index < fieldCount);

    auto fieldsPtr = (FossilizedRecordElementLayout*)(this + 1);
    return fieldsPtr + index;
}

Fossil::AnyValRef Fossil::ValRef<FossilizedRecordVal>::getField(Index index) const
{
    SLANG_ASSERT(index >= 0);
    SLANG_ASSERT(index < getFieldCount());

    auto recordLayout = getLayout();
    auto fieldInfo = recordLayout->getField(index);

    auto fieldsPtr = (Byte*)getDataPtr();
    auto fieldPtr = (void*)(fieldsPtr + fieldInfo->offset);
    return Fossil::AnyValRef(fieldPtr, fieldInfo->layout);
}

} // namespace Slang
