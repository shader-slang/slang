// unit-test-checksum.cpp

#include "tools/unit-test/slang-unit-test.h"

#include "../../source/slang/slang-hash-utils.h"

using namespace Slang;

SLANG_UNIT_TEST(checksum)
{
    {
        slang::Hash testA;
        testA.value[0] = 1;
        testA.value[1] = 2;
        testA.value[2] = 3;
        testA.value[3] = 4;

        String testAString = hashToString(testA);
        SLANG_CHECK(testAString.equals(String("00000001000000020000000300000004")));
    }

    {
        slang::Hash testC;
        testC.value[0] = 0x11111111;
        testC.value[1] = 0x22222222;
        testC.value[2] = 0x33333333;
        testC.value[3] = 0x44444444;

        String testCString = hashToString(testC);
        SLANG_CHECK(testCString.equals(String("11111111222222223333333344444444")));
    }
}
