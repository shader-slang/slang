#pragma once
#include "slang-md5.h"
#include "../../slang.h"
#include "../core/slang-string.h"
#include "../core/slang-list.h"

namespace Slang
{
    using slang::Digest;

    // Wrapper struct that holds objects necessary for hashing.
    struct DigestBuilder
    {
    public:
        DigestBuilder()
        {
            hashGen.init(&context);
        }

        template<typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, bool> = true>
        DigestBuilder& operator<<(const T value)
        {
            append(value);
            return *this;
        }

        DigestBuilder& operator<<(const String& str)
        {
            append(str);
            return *this;
        }

        DigestBuilder& operator<<(const StringSlice& str)
        {
            append(str);
            return *this;
        }

        DigestBuilder& operator<<(const UnownedStringSlice& str)
        {
            append(str);
            return *this;
        }

        DigestBuilder& operator<<(ISlangBlob* blob)
        {
            append(blob);
            return *this;
        }

        DigestBuilder& operator<<(const slang::Digest& digest)
        {
            append(digest);
            return *this;
        }

        template<typename T, std::enable_if_t<std::is_pod_v<T>, bool> = true>
        DigestBuilder& operator<<(const List<T>& list)
        {
            append(list);
            return *this;
        }

        void append(const void* data, SlangInt size)
        {
            hashGen.update(&context, data, size);
        }

        template<typename T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>, bool> = true>
        void append(const T value)
        {
            append(&value, sizeof(T));
        }

        void append(const String& str)
        {
            append(str.getBuffer(), str.getLength());
        }

        void append(const StringSlice& str)
        {
            append(str.begin(), str.getLength());
        }

        void append(const UnownedStringSlice& str)
        {
            append(str.begin(), str.getLength());
        }

        void append(ISlangBlob* blob)
        {
            append(blob->getBufferPointer(), blob->getBufferSize());
        }

        void append(const slang::Digest& digest)
        {
            append(&digest, sizeof(digest));
        }

        template<typename T, std::enable_if_t<std::is_pod_v<T>, bool> = true>
        void append(const List<T>& list)
        {
            append(list.getBuffer(), list.getCount() * sizeof(T));
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
