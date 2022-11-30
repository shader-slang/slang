// unit-test-md5.cpp
#include "tools/unit-test/slang-unit-test.h"

#include "../../source/core/slang-md5.h"
#include "../../source/core/slang-string.h"
#include "../../source/core/slang-digest-util.h"

using namespace Slang;

SLANG_UNIT_TEST(md5hash)
{
    // Empty string
    {
        MD5Context ctx;
        MD5HashGen gen;
        gen.init(&ctx);
        slang::Digest digest;
        gen.finalize(&ctx, &digest);
        SLANG_CHECK(DigestUtil::toString(digest) == "D41D8CD98F00B204E9800998ECF8427E");
    }

    // One call to update()
    {
        MD5Context ctx;
        MD5HashGen gen;
        gen.init(&ctx);
        const String str("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
        gen.update(&ctx, str.getBuffer(), str.getLength());
        slang::Digest digest;
        gen.finalize(&ctx, &digest);
        SLANG_CHECK(DigestUtil::toString(digest) == "818C6E601A24F72750DA0F6C9B8EBE28");
    }

    // Two calls to update()
    {
        MD5Context ctx;
        MD5HashGen gen;
        gen.init(&ctx);
        const String str1("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.");
        const String str2("Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.");
        gen.update(&ctx, str1.getBuffer(), str1.getLength());
        gen.update(&ctx, str2.getBuffer(), str2.getLength());
        slang::Digest digest;
        gen.finalize(&ctx, &digest);
        SLANG_CHECK(DigestUtil::toString(digest) == "87D3CAECB0AB82FAAE84D60FDE994ACA");
    }
}
