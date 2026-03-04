// unit-test-sha1.cpp
#include "../../source/core/slang-crypto.h"
#include "unit-test/slang-unit-test.h"

using namespace Slang;

SLANG_UNIT_TEST(crypto)
{
    // HashDigest
    {
        using Digest = HashDigest<8>;
        Digest empty;
        SLANG_CHECK(empty.data[0] == 0 && empty.data[1] == 0);
        SLANG_CHECK(empty.toString() == "0000000000000000");

        Digest all = Digest("ffffffffffffffff");
        SLANG_CHECK(all.data[0] == 0xffffffff && all.data[1] == 0xffffffff);
        SLANG_CHECK(all.toString() == "ffffffffffffffff");

        SLANG_CHECK(empty == Digest("0000000000000000"));
        SLANG_CHECK(all == Digest("ffffffffffffffff"));
        SLANG_CHECK(empty != all);

        SLANG_CHECK(Digest("invalid") == empty);
        SLANG_CHECK(Digest("X000000000000000") == empty);
        SLANG_CHECK(Digest(" 000000000000000") == empty);

        SLANG_CHECK(Digest("0123456789abcdef").toString() == "0123456789abcdef");

        Slang::ComPtr<ISlangBlob> blob = Digest("0123456789abcdef").toBlob();
        const uint8_t check[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
        SLANG_CHECK(blob->getBufferSize() == 8);
        SLANG_CHECK(::memcmp(blob->getBufferPointer(), check, 8) == 0);

        SLANG_CHECK(Digest(blob) == Digest("0123456789abcdef"));
    }

    // SHA1

    // Empty string
    {
        SHA1 sha1;
        auto digest = sha1.finalize();
        SLANG_CHECK(digest.toString() == "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    }

    // One call to update()
    {
        SHA1 sha1;
        const String str("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                         "tempor incididunt ut labore et dolore magna aliqua.");
        sha1.update(str.getBuffer(), str.getLength());
        auto digest = sha1.finalize();
        SLANG_CHECK(digest.toString() == "cca0871ecbe200379f0a1e4b46de177e2d62e655");
    }

    // Two calls to update()
    {
        SHA1 sha1;
        const String str1("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                          "tempor incididunt ut labore et dolore magna aliqua.");
        const String str2("Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi "
                          "ut aliquip ex ea commodo consequat.");
        sha1.update(str1.getBuffer(), str1.getLength());
        sha1.update(str2.getBuffer(), str2.getLength());
        auto digest = sha1.finalize();
        SLANG_CHECK(digest.toString() == "7a8213edf9976d2e693f27bbc7dc41546bcfcc97");
    }

    // compute()
    {
        SLANG_CHECK(
            SHA1::compute(nullptr, 0).toString() == "da39a3ee5e6b4b0d3255bfef95601890afd80709");
        const String str("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod "
                         "tempor incididunt ut labore et dolore magna aliqua.");
        SLANG_CHECK(
            SHA1::compute(str.getBuffer(), str.getLength()).toString() ==
            "cca0871ecbe200379f0a1e4b46de177e2d62e655");
    }

    // DigestBuider

    // Raw numerical values, etc.
    {
        DigestBuilder<SHA1> builder;

        int64_t valueA = -1;
        uint64_t valueB = 1;
        builder.append(valueA);
        builder.append(valueB);

        auto digest = builder.finalize();
        SLANG_CHECK(digest.toString() == "d3ff95cedce4cfca2e4f3184040032fa7e683185");
    }

    // List
    {
        DigestBuilder<SHA1> builder;

        List<int64_t> listA;
        listA.add(1);
        listA.add(2);
        listA.add(3);
        listA.add(4);
        builder.append(listA);

        auto digest = builder.finalize();
        SLANG_CHECK(digest.toString() == "1074bd0a31dfaae87e1c96888a19f6589ac77cc2");
    }

    // UnownedStringSlice
    {
        DigestBuilder<SHA1> builder;

        UnownedStringSlice stringSlice = UnownedStringSlice("String Slice Test");
        builder.append(stringSlice);

        auto digest = builder.finalize();
        SLANG_CHECK(digest.toString() == "5e6369cbe14190ff45a2690b69a4741c9282611f");
    }

    // String
    {
        DigestBuilder<SHA1> builder;

        String str = String("String Test");
        builder.append(str);

        auto digest = builder.finalize();
        SLANG_CHECK(digest.toString() == "1748cfb982b6198515d238ceb172947aa39a057c");
    }

    // Digest
    {
        DigestBuilder<SHA1> builder;

        SHA1::Digest hash;
        builder.append(hash);

        auto digest = builder.finalize();
        SLANG_CHECK(digest.toString() == "6768033e216468247bd031a0a2d9876d79818f8f");
    }
}
