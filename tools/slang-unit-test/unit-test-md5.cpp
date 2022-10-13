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
        SLANG_CHECK(testAString.equals(String("5BA171E20898BDD205639013746F2679")));
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
        SLANG_CHECK(testBString.equals(String("4352D88A78AA39750BF70CD6F27BCAA5")));
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
        SLANG_CHECK(testCString.equals(String("5D6CC58E1824A4DFD0CF57395B603316")));
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
        SLANG_CHECK(testDString.equals(String("DF5A79CC2170C7401CF0A506CEB0CE24")));
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
        SLANG_CHECK(testEString.equals(String("4AE71336E44BF9BF79D2752E234818A5")));
    }
}
