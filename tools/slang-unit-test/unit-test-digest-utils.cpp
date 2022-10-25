// unit-test-digest-utils.cpp

#include "tools/unit-test/slang-unit-test.h"

#include "../../source/core/slang-digest-util.h"

using namespace Slang;

SLANG_UNIT_TEST(digestUtils)
{
    {
        slang::Digest testA;
        testA.values[0] = 1;
        testA.values[1] = 2;
        testA.values[2] = 3;
        testA.values[3] = 4;

        String testAString = DigestUtil::toString(testA);
        SLANG_CHECK(testAString.equals(String("01000000020000000300000004000000")));
    }

    {
        slang::Digest testC;
        testC.values[0] = 0x11111111;
        testC.values[1] = 0x22222222;
        testC.values[2] = 0x33333333;
        testC.values[3] = 0x44444444;

        String testCString = DigestUtil::toString(testC);
        SLANG_CHECK(testCString.equals(String("11111111222222223333333344444444")));
    }

    {
        auto digestString = UnownedStringSlice("5D6CC58E1824A4DFD0CF57395B603316");
        slang::Digest digest = DigestUtil::fromString(digestString);
        auto resultString = DigestUtil::toString(digest);
        SLANG_CHECK(resultString == digestString);
    }

    {
        auto digestString = UnownedStringSlice("01000000020000000300000004000000");
        slang::Digest digest = DigestUtil::fromString(digestString);
        auto resultString = DigestUtil::toString(digest);
        SLANG_CHECK(resultString == digestString);
    }

    {
        slang::Digest testD;
        testD.values[0] = 1;
        testD.values[1] = 2;
        testD.values[2] = 3;
        testD.values[3] = 4;

        StringBuilder testDSb;
        testDSb << testD;
        SLANG_CHECK(testDSb.equals(String("01000000020000000300000004000000")));
    }
}
