// unit-test-byte-encode.cpp

#include "../../source/core/slang-byte-encode-util.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "test-context.h"

#include "../../source/core/slang-random-generator.h"
#include "../../source/core/list.h"

using namespace Slang;


static void checkUInt32(uint32_t value)
{
    uint8_t buffer[ByteEncodeUtil::kMaxLiteEncodeUInt32 + 1];

    int writeLen = ByteEncodeUtil::encodeLiteUInt32(value, buffer);
    buffer[writeLen] = 0xcd;

    uint32_t decode;
    int readLen = ByteEncodeUtil::decodeLiteUInt32(buffer, &decode);

    SLANG_CHECK(readLen == writeLen && decode == value);
}

static void byteEncodeUnitTest()
{
    DefaultRandomGenerator randGen(0x5346536a);

    {
        checkUInt32(uint32_t(0));
        checkUInt32(uint32_t(0x7fffff));
        checkUInt32(uint32_t(0x7fff));
        checkUInt32(uint32_t(0x7f));
        checkUInt32(uint32_t(0x7fffffff));
        checkUInt32(uint32_t(0xffffffff));

        for (int64_t i = 0; i < SLANG_INT64(0x100000000); i += 371)
        {
            checkUInt32(uint32_t(i));
        }
    } 

}

SLANG_UNIT_TEST("ByteEncode", byteEncodeUnitTest);