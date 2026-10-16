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

#if SLANG_ENABLE_VALIDATION_FOSSIL

/// Return true if the `byteCount` bytes starting at `ptr` lie entirely within the
/// `blobSize` bytes starting at `blobBegin`. `ptr` comes from resolving a relative
/// pointer read out of a file, so the arithmetic is ordered so it cannot wrap.
///
static bool _isRangeWithinBlob(
    void const* ptr,
    Size byteCount,
    void const* blobBegin,
    Size blobSize)
{
    auto address = uintptr_t(ptr);
    auto begin = uintptr_t(blobBegin);
    if (address < begin)
        return false;

    auto offset = Size(address - begin);
    if (offset > blobSize)
        return false;

    return byteCount <= blobSize - offset;
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

#if SLANG_ENABLE_VALIDATION_FOSSIL
    // Bound against `size` rather than `reportedSize`: `size` is the extent of
    // the buffer we were handed, while `reportedSize` is self-reported by the
    // file, and reads zero in blobs written before the writer populated it.
    //
    // A variant keeps the relative pointer to its content layout in the word
    // *before* its own address, where its content starts, so checking that slot
    // also covers the start of the content.
    //
    using ContentLayoutPtr = FossilizedPtr<FossilizedValLayout>;
    auto contentLayoutPtrPtr = (Byte const*)rootValueVariant - sizeof(ContentLayoutPtr);
    SLANG_SERIALIZE_FOSSIL_VALIDATE(
        _isRangeWithinBlob(contentLayoutPtrPtr, sizeof(ContentLayoutPtr), data, size));

    // The reader immediately reads `layout->kind`. Only the root of the layout is
    // checked here, not the graph reachable from it.
    //
    auto contentLayout = rootValueVariant->getContentLayout();
    SLANG_SERIALIZE_FOSSIL_VALIDATE(contentLayout != nullptr);
    SLANG_SERIALIZE_FOSSIL_VALIDATE(
        _isRangeWithinBlob(contentLayout, sizeof(FossilizedValLayout), data, size));
#endif

    return getVariantContentPtr(rootValueVariant);
}

} // namespace Fossil

Fossil::AnyValRef Fossil::ValRef<FossilizedContainerObjBase>::getElement(Index index) const
{
    SLANG_SERIALIZE_FOSSIL_VALIDATE(index >= 0);
    SLANG_SERIALIZE_FOSSIL_VALIDATE(index < getElementCount());

    auto containerLayout = getLayout();
    auto elementLayout = containerLayout->elementLayout.get();
    auto elementStride = containerLayout->elementStride;

    auto elementsPtr = (Byte*)getDataPtr();
    auto elementPtr = (void*)(elementsPtr + elementStride * index);
    return Fossil::AnyValRef(elementPtr, elementLayout);
}

FossilizedRecordElementLayout* FossilizedRecordLayout::getField(Index index) const
{
    SLANG_SERIALIZE_FOSSIL_VALIDATE(index >= 0);
    SLANG_SERIALIZE_FOSSIL_VALIDATE(index < fieldCount);

    auto fieldsPtr = (FossilizedRecordElementLayout*)(this + 1);
    return fieldsPtr + index;
}

Fossil::AnyValRef Fossil::ValRef<FossilizedRecordVal>::getField(Index index) const
{
    SLANG_SERIALIZE_FOSSIL_VALIDATE(index >= 0);
    SLANG_SERIALIZE_FOSSIL_VALIDATE(index < getFieldCount());

    auto recordLayout = getLayout();
    auto fieldInfo = recordLayout->getField(index);

    auto fieldsPtr = (Byte*)getDataPtr();
    auto fieldPtr = (void*)(fieldsPtr + fieldInfo->offset);
    return Fossil::AnyValRef(fieldPtr, fieldInfo->layout);
}

} // namespace Slang
