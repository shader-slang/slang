// unit-test-path.cpp

#include "../../source/core/slang-relative-container.h"

#include "test-context.h"

using namespace Slang;

static void relativeContainerUnitTest()
{
    uint8_t encode[RelativeString::kMaxSizeEncodeSize];

    for (int64_t i = 0; i < 0x100000000; ++i)
    {
        size_t encodeSize = RelativeString::calcEncodedSize(uint32_t(i), encode);

        size_t decodedSize;
        const char* chars = RelativeString::decodeSize((const char*)encode, decodedSize);

        SLANG_CHECK(decodedSize == size_t(i));
        SLANG_CHECK(chars - (const char*)encode == encodeSize);
    }
}

SLANG_UNIT_TEST("RelativeContainer", relativeContainerUnitTest);
