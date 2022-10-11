#pragma once
#include "slang-md5.h"
#include "../../slang.h"

namespace Slang
{
    // Wrapper struct that holds objects necessary for hashing.
    struct DigestBuilder
    {
        MD5HashGen hashGen;
        MD5Context context;

        DigestBuilder()
        {
            hashGen.init(&context);
        }

        template <typename T>
        void addToDigest(T item)
        {
            hashGen.update(&context, item);
        }

        void finalize(slang::Digest* outHash)
        {
            hashGen.finalize(&context, outHash);
        }
    };
}
