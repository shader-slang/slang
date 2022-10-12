// unit-test-md5.cpp
#include "tools/unit-test/slang-unit-test.h"

#include "../../source/core/slang-md5.h"
#include "../../source/core/slang-string.h"
#include "../../source/slang/slang-hash-utils.h"

using namespace Slang;

SLANG_UNIT_TEST(md5hash)
{
    {
        // Raw numerical values, etc.
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        int64_t valueA = -1;
        uint64_t valueB = 1;
        testHashGen.update(&testCtx, valueA);
        testHashGen.update(&testCtx, valueB);

        slang::Digest testA;
        testHashGen.finalize(&testCtx, &testA);

        String testAString = hashToString(testA);
        SLANG_CHECK(testAString.equals(String("E271A15BD2BD98081390630579266F74")));
    }

    {
        // List
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        List<int64_t> listA;
        listA.add(1);
        listA.add(2);
        listA.add(3);
        listA.add(4);
        testHashGen.update(&testCtx, listA);

        slang::Digest testB;
        testHashGen.finalize(&testCtx, &testB);

        String testBString = hashToString(testB);
        SLANG_CHECK(testBString.equals(String("8AD852437539AA78D60CF70BA5CA7BF2")));
    }

    {
        // UnownedStringSlice
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        UnownedStringSlice stringSlice = UnownedStringSlice("String Slice Test");
        testHashGen.update(&testCtx, stringSlice);

        slang::Digest testC;
        testHashGen.finalize(&testCtx, &testC);

        String testCString = hashToString(testC);
        SLANG_CHECK(testCString.equals(String("8EC56C5DDFA424183957CFD01633605B")));
    }

    {
        // String
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        String str = String("String Test");
        testHashGen.update(&testCtx, str);

        slang::Digest testD;
        testHashGen.finalize(&testCtx, &testD);

        String testDString = hashToString(testD);
        SLANG_CHECK(testDString.equals(String("CC795ADF40C7702106A5F01C24CEB0CE")));
    }

    {
        // Hash
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        slang::Digest Hash;
        testHashGen.update(&testCtx, Hash);

        slang::Digest testE;
        testHashGen.finalize(&testCtx, &testE);

        String testEString = hashToString(testE);
        SLANG_CHECK(testEString.equals(String("3613E74ABFF94BE42E75D279A5184823")));
    }
}
