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

        slang::Digest textHash;
        builder.finalize(&textHash);

        return textHash;
    }

    // Combines the two provided hashes.
    inline slang::Digest combineHashes(const slang::Digest& hashA, const slang::Digest& hashB)
    {
        DigestBuilder builder;
        builder.addToDigest(hashA);
        builder.addToDigest(hashB);

        slang::Digest combined;
        builder.finalize(&combined);
        return combined;
    }

    // Returns the stored hash in checksum as a String.
    inline StringBuilder hashToString(const slang::Digest& hash)
    {
        StringBuilder filename;

        for (Index i = 0; i < 4; ++i)
        {
            auto hashSegmentString = String(hash.values[i], 16);

            auto leadingZeroCount = 8 - hashSegmentString.getLength();
            for (Index j = 0; j < leadingZeroCount; ++j)
            {
                filename.append("0");
            }
            filename.append(hashSegmentString.getBuffer());
        }

        return filename;
    }

    // TODO: fromString implementation?
}
