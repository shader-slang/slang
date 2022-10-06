// slang-checksum-utils.h
#include "../../slang.h"
#include "../core/slang-basic.h"

namespace Slang
{
    // Returns the stored hash in checksum as a String.
    inline String checksumToString(slang::Checksum checksum)
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
