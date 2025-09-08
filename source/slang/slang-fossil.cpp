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
