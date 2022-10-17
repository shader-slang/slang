// string-digest-util.cpp
#include "slang-digest-util.h"

#include "../core/slang-basic.h"
#include "../core/slang-digest-builder.h"
#include "../core/slang-md5.h"
#include "../core/slang-char-util.h"

namespace Slang
{

/*static*/ Digest DigestUtil::computeDigestForStringSlice(UnownedStringSlice text)
{
    DigestBuilder builder;
    builder.addToDigest(text);
    return builder.finalize();
}

/*static*/ Digest DigestUtil::combine(const Digest& digestA, const Digest& digestB)
{
    DigestBuilder builder;
    builder.addToDigest(digestA);
    builder.addToDigest(digestB);
    return builder.finalize();
}

/*static*/ String DigestUtil::toString(const Digest& digest)
{
    StringBuilder hashString;

    uint8_t* uint8Hash = (uint8_t*)digest.values;

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

/*static*/ Digest DigestUtil::fromString(UnownedStringSlice hashString)
{
    uint8_t uint8Hash[16];

    // When the hash is converted to a String, ReverseInternalAscii is called
    // at the very end. Since there is no way to get a char* for hashString to pass
    // to ReverseInternalAscii to flip the string back, we instead loop starting from
    // the end and work backwards towards the beginning.
    for (Index i = 0; i < 16; i++)
    {
        uint8Hash[i] = (uint8_t)CharUtil::getHexDigitValue(hashString[i * 2]) * 16
            + (uint8_t)CharUtil::getHexDigitValue(hashString[i * 2 + 1]);
    }

    Digest digest;
    memcpy(digest.values, uint8Hash, 16);
    return digest;
}

}
