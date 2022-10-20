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
        // Must cast to uint8_t* first in order to correctly account for
        // endianness.
        uint8_t* uint8Hash = (uint8_t*)d.values;

        for (Index i = 0; i < 16; ++i)
        {
            int hashSegment = uint8Hash[i];
            // Check if we need to append a leading zero.
            if (hashSegment < 16)
            {
                sb << "0";
            }
            sb.append(hashSegment, 16);
        }
        return sb;
    }
}
