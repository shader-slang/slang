// unit-test-digest-utils.cpp

#include "tools/unit-test/slang-unit-test.h"

#include "../../source/slang/slang-hash-utils.h"

using namespace Slang;

SLANG_UNIT_TEST(digestUtils)
{
    {
        slang::Digest testA;
        testA.values[0] = 1;
        testA.values[1] = 2;
        testA.values[2] = 3;
        testA.values[3] = 4;

        String testAString = hashToString(testA);
        SLANG_CHECK(testAString.equals(String("01000000020000000300000004000000")));
    }

    {
        slang::Digest testC;
        testC.values[0] = 0x11111111;
        testC.values[1] = 0x22222222;
        testC.values[2] = 0x33333333;
        testC.values[3] = 0x44444444;

        String testCString = hashToString(testC);
        SLANG_CHECK(testCString.equals(String("11111111222222223333333344444444")));
    }

    {
        String digestString = String("5D6CC58E1824A4DFD0CF57395B603316");
        slang::Digest digest = stringToHash(digestString);

        SLANG_CHECK(digest.values[0] == 2395303005);
        SLANG_CHECK(digest.values[1] == 3752076312);
        SLANG_CHECK(digest.values[2] == 962056144);
        SLANG_CHECK(digest.values[3] == 372465755);
    }

    {
        String digestString = String("01000000020000000300000004000000");
        slang::Digest digest = stringToHash(digestString);

        SLANG_CHECK(digest.values[0] == 1);
        SLANG_CHECK(digest.values[1] == 2);
        SLANG_CHECK(digest.values[2] == 3);
        SLANG_CHECK(digest.values[3] == 4);
    }
}
