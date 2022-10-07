// unit-test-checksum.cpp

#include "tools/unit-test/slang-unit-test.h"

#include "../../source/slang/slang-checksum-utils.h"

using namespace Slang;

SLANG_UNIT_TEST(checksum)
{
    {
        slang::Checksum testA;
        testA.checksum[0] = 1;
        testA.checksum[1] = 2;
        testA.checksum[2] = 3;
        testA.checksum[3] = 4;

        String testAString = checksumToString(testA);
        SLANG_CHECK(testAString.equals(String("00000001000000020000000300000004")));
    }

    {
        slang::Checksum testC;
        testC.checksum[0] = 0x11111111;
        testC.checksum[1] = 0x22222222;
        testC.checksum[2] = 0x33333333;
        testC.checksum[3] = 0x44444444;

        String testCString = checksumToString(testC);
        SLANG_CHECK(testCString.equals(String("11111111222222223333333344444444")));
    }
}
