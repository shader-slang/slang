// slang-hash-utils.h - Utility functions specifically designed to be used with slang::Digest
#pragma once
#include "../../slang.h"
#include "../core/slang-basic.h"
#include "../core/slang-md5.h"
#include "../core/slang-digest.h"
#include "../core/slang-char-util.h"

namespace Slang
{
    using slang::Digest;

    // Compute the hash for an UnownedStringSlice
    inline Digest computeHashForStringSlice(UnownedStringSlice text)
    {
        DigestBuilder builder;
        builder.addToDigest(text);
        return builder.finalize();
    }

    // Combines the two provided hashes.
    inline Digest combineDigests(const Digest& hashA, const Digest& hashB)
    {
        DigestBuilder builder;
        builder.addToDigest(hashA);
        builder.addToDigest(hashB);
        return builder.finalize();
    }

    // Returns the stored hash in checksum as a String.
    inline String hashToString(const Digest& hash)
    {
        StringBuilder hashString;

        uint8_t* uint8Hash = (uint8_t*)hash.values;

        for (Index i = 0; i < 16; ++i)
        {
            auto hashSegmentString = String(uint8Hash[i], 16);

            if (hashSegmentString.getLength() == 1)
            {
                hashString.append("0");
            }
            hashString.append(hashSegmentString.getBuffer());
        }

        return hashString;
    }

    inline Digest stringToHash(UnownedStringSlice hashString)
    {
        uint8_t uint8Hash[16];

        // When the hash is converted to a String, ReverseInternalAscii is called
        // at the very end. Since there is no way to get a char* for hashString to pass
        // to ReverseInternalAscii to flip the string back, we instead loop starting from
        // the end and work backwards towards the beginning.
        for (Index i = 0; i < 16; i++)
        {
            uint8Hash[i] = CharUtil::getHexDigitValue(hashString[i * 2]) * 16 + CharUtil::getHexDigitValue(hashString[i * 2 + 1]);
        }

        Digest digest;
        memcpy(digest.values, uint8Hash, 16);
        return digest;
    }
}
