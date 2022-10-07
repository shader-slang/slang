// slang-checksum-utils.h
#pragma once
#include "../../slang.h"
#include "../core/slang-basic.h"
#include "../core/slang-md5.h"

namespace Slang
{
    // Combines the two provided hashes to produce the final shader cache entry key.
    inline slang::Checksum combineChecksum(const slang::Checksum& linkageHash, const slang::Checksum& programHash)
    {
        MD5Context context;
        MD5HashGen hashGen;
        hashGen.init(&context);
        hashGen.update(&context, linkageHash);
        hashGen.update(&context, programHash);

        slang::Checksum combined;
        hashGen.finalize(&context, &combined);
        return combined;
    }

    // Returns the stored hash in checksum as a String.
    inline StringBuilder checksumToString(const slang::Checksum& checksum)
    {
        StringBuilder filename;

        for (Index i = 0; i < 4; ++i)
        {
            auto hashSegmentString = String(checksum.checksum[i], 16);

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
