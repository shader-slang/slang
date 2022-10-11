// slang-hash-utils.h - Utility functions specifically designed to be used with slang::Digest
#pragma once
#include "../../slang.h"
#include "../core/slang-basic.h"
#include "../core/slang-md5.h"

namespace Slang
{
    // Compute the hash for an UnownedStringSlice
    inline slang::Digest computeHashForStringSlice(UnownedStringSlice text)
    {
        MD5Context context;
        MD5HashGen hashGen;
        hashGen.init(&context);
        hashGen.update(&context, text);

        slang::Digest textHash;
        hashGen.finalize(&context, &textHash);

        return textHash;
    }

    // Combines the two provided hashes to produce the final shader cache entry key.
    inline slang::Digest combineHashes(const slang::Digest& linkageHash, const slang::Digest& programHash)
    {
        MD5Context context;
        MD5HashGen hashGen;
        hashGen.init(&context);
        hashGen.update(&context, linkageHash);
        hashGen.update(&context, programHash);

        slang::Digest combined;
        hashGen.finalize(&context, &combined);
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
