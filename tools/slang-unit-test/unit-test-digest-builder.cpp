// unit-test-digest-utils.cpp

#include "tools/unit-test/slang-unit-test.h"

#include "../../source/core/slang-digest-builder.h"
#include "../../source/core/slang-digest-util.h"

using namespace Slang;

SLANG_UNIT_TEST(digestBuilder)
{
    // Raw numerical values, etc.
    {
        DigestBuilder builder;

        int64_t valueA = -1;
        uint64_t valueB = 1;
        builder.append(valueA);
        builder.append(valueB);

        slang::Digest digest = builder.finalize();
        SLANG_CHECK(DigestUtil::toString(digest) == "5BA171E20898BDD205639013746F2679");
    }

    // List
    {
        DigestBuilder builder;

        List<int64_t> listA;
        listA.add(1);
        listA.add(2);
        listA.add(3);
        listA.add(4);
        builder.append(listA);

        slang::Digest digest = builder.finalize();
        SLANG_CHECK(DigestUtil::toString(digest) == "9F66C130786A1A05E4731F71A3C5F172");
    }

    // UnownedStringSlice
    {
        DigestBuilder builder;

        UnownedStringSlice stringSlice = UnownedStringSlice("String Slice Test");
        builder.append(stringSlice);

        slang::Digest digest = builder.finalize();
        SLANG_CHECK(DigestUtil::toString(digest) == "5D6CC58E1824A4DFD0CF57395B603316");
    }

    // String
    {
        DigestBuilder builder;

        String str = String("String Test");
        builder.append(str);

        slang::Digest digest = builder.finalize();
        SLANG_CHECK(DigestUtil::toString(digest) == "DF5A79CC2170C7401CF0A506CEB0CE24");
    }

    // Digest
    {
        DigestBuilder builder;

        slang::Digest hash;
        builder.append(hash);

        slang::Digest digest = builder.finalize();
        SLANG_CHECK(DigestUtil::toString(digest) == "4AE71336E44BF9BF79D2752E234818A5");
    }
}
