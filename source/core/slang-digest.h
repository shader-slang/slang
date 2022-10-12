#pragma once
#include "slang-md5.h"
#include "../../slang.h"

using slang::Digest;

namespace Slang
{
    // Wrapper struct that holds objects necessary for hashing.
    struct DigestBuilder
    {
    public:
        DigestBuilder()
        {
            hashGen.init(&context);
        }

        template <typename T>
        void addToDigest(T item)
        {
            hashGen.update(&context, item);
        }

        Digest finalize()
        {
            Digest hash;
            hashGen.finalize(&context, &hash);
            return hash;
        }

    private:
        MD5HashGen hashGen;
        MD5Context context;
    };
}
