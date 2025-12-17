// unit-test-byte-encode.cpp

#include "../../source/core/slang-byte-encode-util.h"
#include "unit-test/slang-unit-test.h"

#include <stdlib.h>

using namespace Slang;

SLANG_UNIT_TEST(byteEncode)
{
    {
        SLANG_CHECK(ByteEncodeUtil::calcMsb8(0) == -1);
        SLANG_CHECK(ByteEncodeUtil::calcMsb8(1) == 0);
        SLANG_CHECK(ByteEncodeUtil::calcMsb8(0x81) == 7);
    }

    {
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0) == -1);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x81) == 7);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x00000001) == 0);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x00000081) == 7);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x00000181) == 8);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x00008181) == 15);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x00018181) == 16);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x00818181) == 23);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x01818181) == 24);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0x81818181) == 31);
        SLANG_CHECK(ByteEncodeUtil::calcMsb32(0xffffffff) == 31);
    }
}
