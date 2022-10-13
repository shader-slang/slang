// slang-hash-utils.h - Utility functions specifically designed to be used with slang::Digest
#pragma once
#include "../../slang.h"
#include "../core/slang-basic.h"
#include "../core/slang-md5.h"
#include "../core/slang-digest.h"

namespace Slang
{
    // Compute the hash for an UnownedStringSlice
    inline slang::Digest computeHashForStringSlice(UnownedStringSlice text)
    {
        DigestBuilder builder;
        builder.addToDigest(text);
        return builder.finalize();
    }

    // Combines the two provided hashes.
    inline slang::Digest combineHashes(const slang::Digest& hashA, const slang::Digest& hashB)
    {
        DigestBuilder builder;
        builder.addToDigest(hashA);
        builder.addToDigest(hashB);
        return builder.finalize();
    }

    // Returns the stored hash in checksum as a String.
    inline StringBuilder hashToString(const slang::Digest& hash)
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

    // TODO: fromString implementation?
}
