// slang-digest-utils.h - Utility functions specifically designed to be used with slang::Digest
#pragma once
#include "../../slang.h"
#include "slang-string.h"

namespace Slang
{
    using slang::Digest;

    struct DigestUtil
    {
        // Compute the digest for an UnownedStringSlice
        static Digest computeDigestForStringSlice(UnownedStringSlice text);

        // Combines the two provided digests.
        static Digest combine(const Digest& digestA, const Digest& digestB);

        // Returns the hash stored in digest as a String.
        static String toString(const Digest& digest);

        // Returns the hash represented by hashString as a Digest.
        static Digest fromString(UnownedStringSlice hashString);
    };

    inline StringBuilder& operator<<(StringBuilder& sb, const Digest& d)
    {
        for (Index i = 0; i < 4; ++i)
        {
            char temp[8];
            IntToAscii(temp, d.values[i], 16);
            sb << temp;
        }
        return sb;
    }
}
