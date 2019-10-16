// unit-test-path.cpp

#include "../../source/core/slang-relative-container.h"

#include "test-context.h"

using namespace Slang;

static void _checkEncodeDecode(uint32_t size)
{
    uint8_t encode[RelativeString::kMaxSizeEncodeSize];

    size_t encodeSize = RelativeString::calcEncodedSize(size, encode);

    size_t decodedSize;
    const char* chars = RelativeString::decodeSize((const char*)encode, decodedSize);

    SLANG_CHECK(decodedSize == size);
    SLANG_CHECK(chars - (const char*)encode == encodeSize);
}

static void relativeContainerUnitTest()
{
    _checkEncodeDecode(253);

    for (int64_t i = 0; i < 0x100000000; i += (i / 2) + 1)
    {
        _checkEncodeDecode(uint32_t(i));
    }
}

SLANG_UNIT_TEST("RelativeContainer", relativeContainerUnitTest);
