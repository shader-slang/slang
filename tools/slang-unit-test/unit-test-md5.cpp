// unit-test-md5.cpp

#include "tools/unit-test/slang-unit-test.h"

#include "../../source/compiler-core/slang-md5.h"
#include "../../source/core/slang-string.h"
#include "../../source/slang/slang-checksum-utils.h"

using namespace Slang;

SLANG_UNIT_TEST(md5hash)
{
    {
        // Raw numerical values, etc.
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        SlangInt valueA = -1;
        SlangUInt valueB = 1;
        testHashGen.update(&testCtx, valueA);
        testHashGen.update(&testCtx, valueB);

        slang::Checksum testA;
        testHashGen.finalize(&testCtx, &testA);

        String testAString = checksumToString(testA);
        SLANG_CHECK(testAString.equals(String("E271A15BD2BD98081390630579266F74")));
    }

    {
        // List
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        List<SlangInt> listA;
        listA.add(1);
        listA.add(2);
        listA.add(3);
        listA.add(4);
        testHashGen.update(&testCtx, listA);

        slang::Checksum testB;
        testHashGen.finalize(&testCtx, &testB);

        String testBString = checksumToString(testB);
        SLANG_CHECK(testBString.equals(String("8AD852437539AA78D60CF70BA5CA7BF2")));
    }

    {
        // UnownedStringSlice
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        UnownedStringSlice stringSlice = UnownedStringSlice("String Slice Test");
        testHashGen.update(&testCtx, stringSlice);

        slang::Checksum testC;
        testHashGen.finalize(&testCtx, &testC);

        String testCString = checksumToString(testC);
        SLANG_CHECK(testCString.equals(String("8EC56C5DDFA424183957CFD01633605B")));
    }

    {
        // String
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        String str = String("String Test");
        testHashGen.update(&testCtx, str);

        slang::Checksum testD;
        testHashGen.finalize(&testCtx, &testD);

        String testDString = checksumToString(testD);
        SLANG_CHECK(testDString.equals(String("CC795ADF40C7702106A5F01C24CEB0CE")));
    }

    {
        // Checksum
        MD5Context testCtx;
        MD5HashGen testHashGen;
        testHashGen.init(&testCtx);

        slang::Checksum checksum;
        testHashGen.update(&testCtx, checksum);

        slang::Checksum testE;
        testHashGen.finalize(&testCtx, &testE);

        String testEString = checksumToString(testE);
        SLANG_CHECK(testEString.equals(String("3613E74ABFF94BE42E75D279A5184823")));
    }
}
