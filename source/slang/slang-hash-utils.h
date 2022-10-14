// slang-hash-utils.h - Utility functions specifically designed to be used with slang::Digest
#pragma once
#include "../../slang.h"
#include "../core/slang-basic.h"
#include "../core/slang-md5.h"
#include "../core/slang-digest.h"

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
    inline Digest combineHashes(const Digest& hashA, const Digest& hashB)
    {
        DigestBuilder builder;
        builder.addToDigest(hashA);
        builder.addToDigest(hashB);
        return builder.finalize();
    }

    // Returns the stored hash in checksum as a String.
    inline StringBuilder hashToString(const Digest& hash)
    {
        StringBuilder filename;

        uint8_t* uint8Hash = (uint8_t*)hash.values;

        for (Index i = 0; i < 16; ++i)
        {
            auto hashSegmentString = String(uint8Hash[i], 16);

            if (hashSegmentString.getLength() == 1)
            {
                filename.append("0");
            }
            filename.append(hashSegmentString.getBuffer());
        }

        return filename;
    }

    inline Digest stringToHash(String hashString)
    {
        uint8_t uint8Hash[16];

        // When the hash is converted to a String, ReverseInternalAscii is called
        // at the very end. Since there is no way to get a char* for hashString to pass
        // to ReverseInternalAscii to flip the string back, we instead loop starting from
        // the end and work backwards towards the beginning.
        for (Index i = 15; i >= 0; --i)
        {
            uint8_t hashSegment = 0;
            for (Index j = 0; j < 2; ++j)
            {
                auto character = hashString[2 * i + j];
                uint8_t charBits;
                if (character - 'A' < 0)
                {
                    // If the difference is negative, this character is a number.
                    charBits = character - '0';
                }
                else
                {
                    // If the difference is 0 or positive, this character is a letter.
                    charBits = character - 'A' + 10;
                }
                // Each character represents 4 bits of data in the original value, so
                // we left shift charBits in order to mask over the corresponding bits in
                // hashSegment.
                hashSegment |= charBits << (4 * (1 - j));
            }
            uint8Hash[i] = hashSegment;
        }

        Digest digest;
        memcpy(digest.values, uint8Hash, 16);
        return digest;
    }
}
