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

FossilizedValRef getRootValue(ISlangBlob* blob)
{
    return getRootValue(blob->getBufferPointer(), blob->getBufferSize());
}

FossilizedValRef getRootValue(void const* data, Size size)
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

    return FossilizedValRef(
        rootValueVariant->getContentData(),
        rootValueVariant->getContentLayout());
}

} // namespace Fossil

Size FossilizedStringObj::getSize() const
{
    auto sizePtr = (FossilUInt*)this - 1;
    return Size(*sizePtr);
}

UnownedTerminatedStringSlice FossilizedStringObj::getValue() const
{
    auto size = getSize();
    return UnownedTerminatedStringSlice((char*)this, size);
}

Count FossilizedContainerObj::getElementCount() const
{
    auto countPtr = (FossilUInt*)this - 1;
    return Size(*countPtr);
}

FossilizedValLayout* FossilizedVariantObj::getContentLayout() const
{
    auto layoutPtrPtr = (FossilizedPtr<FossilizedValLayout>*)this - 1;
    return (*layoutPtrPtr).get();
}

FossilizedValRef getPtrTarget(FossilizedPtrValRef ptrRef)
{
    auto ptrLayout = ptrRef.getLayout();
    auto ptrPtr = ptrRef.getData();
    return FossilizedValRef(ptrPtr->getTargetData(), ptrLayout->elementLayout);
}

bool hasValue(FossilizedOptionalObjRef optionalRef)
{
    return optionalRef.getData() != nullptr;
}

FossilizedValRef getValue(FossilizedOptionalObjRef optionalRef)
{
    auto optionalLayout = optionalRef.getLayout();
    auto valuePtr = optionalRef.getData();
    return FossilizedValRef(valuePtr, optionalLayout->elementLayout);
}

Count getElementCount(FossilizedContainerObjRef containerRef)
{
    if (!containerRef)
        return 0;

    auto containerPtr = containerRef.getData();
    return containerPtr->getElementCount();
}

FossilizedValRef getElement(FossilizedContainerObjRef containerRef, Index index)
{
    SLANG_ASSERT(index >= 0);
    SLANG_ASSERT(index < getElementCount(containerRef));

    auto containerLayout = containerRef.getLayout();
    auto elementLayout = containerLayout->elementLayout.get();
    auto elementStride = containerLayout->elementStride;

    auto elementsPtr = (Byte*)containerRef.getData();
    auto elementPtr = (FossilizedVal*)(elementsPtr + elementStride * index);
    return FossilizedValRef(elementPtr, elementLayout);
}

Count getFieldCount(FossilizedRecordValRef recordRef)
{
    auto recordLayout = recordRef.getLayout();
    return recordLayout->fieldCount;
}

FossilizedRecordElementLayout* FossilizedRecordLayout::getField(Index index) const
{
    SLANG_ASSERT(index >= 0);
    SLANG_ASSERT(index < fieldCount);

    auto fieldsPtr = (FossilizedRecordElementLayout*)(this + 1);
    return fieldsPtr + index;
}


FossilizedValRef getField(FossilizedRecordValRef recordRef, Index index)
{
    SLANG_ASSERT(index >= 0);
    SLANG_ASSERT(index < getFieldCount(recordRef));

    auto recordLayout = recordRef.getLayout();
    auto field = recordLayout->getField(index);

    auto fieldsPtr = (Byte*)recordRef.getData();
    auto fieldPtr = (FossilizedVal*)(fieldsPtr + field->offset);
    return FossilizedValRef(fieldPtr, field->layout);
}

FossilizedValRef getVariantContent(FossilizedVariantObjRef variantRef)
{
    return getVariantContent(variantRef.getData());
}

FossilizedValRef getVariantContent(FossilizedVariantObj* variantPtr)
{
    return FossilizedValRef(variantPtr->getContentData(), variantPtr->getContentLayout());
}

} // namespace Slang
