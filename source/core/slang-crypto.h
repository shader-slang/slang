#pragma once
#include "../../slang.h"
#include "../core/slang-string.h"
#include "../core/slang-blob.h"
#include "../core/slang-list.h"

namespace Slang
{
    struct DigestUtil
    {
        /// Convert a binary digest to a string (lower-case hexadecimal).
        /// Returned string is double the length of the digest.
        static String digestToString(const void* digest, SlangInt digestSize);

        /// Convert a string to a binary digest.
        /// Expects a string of double the length of the digest size in hexadecimal format.
        /// Sets the digest to all zeros if the string is invalid.
        /// Returns true if string was converted successfully.
        static bool stringToDigest(const char* str, SlangInt strLength, void *digest, SlangInt digestSize);
    };

    /// Represents a hash digest. Only sizes of multiple of 4 are supported.
    template<SlangInt N>
    class Digest
    {
    public:
        static_assert(N % 4 == 0, "size must be multiple of 4");
        uint32_t data[N / 4] = { 0 };

        Digest() = default;

        Digest(const char* str)
        {
            DigestUtil::stringToDigest(str, ::strlen(str), data, N);
        }

        Digest(const String& str)
        {
            DigestUtil::stringToDigest(str.getBuffer(), str.getLength(), data, N);
        }

        Digest(const UnownedStringSlice& str)
        {
            DigestUtil::stringToDigest(str.begin(), str.getLength(), data, N);
        }

        Digest(ISlangBlob* blob)
        {
            if (blob->getBufferSize() == N)
            {
                ::memcpy(data, blob->getBufferPointer(), N);
            }
        }

        String toString() const
        {
            return DigestUtil::digestToString(data, N);
        }

        ComPtr<ISlangBlob> toBlob() const
        {
            return RawBlob::create(data, sizeof(data));
        }

        bool operator==(const Digest& other) const
        {
            return ::memcmp(data, other.data, sizeof(data)) == 0;
        }

        bool operator!=(const Digest& other) const
        {
            return !(*this == other);
        }

        uint32_t getHashCode() const
        {
            return data[0];
        }
    };

    /// MD5 hash generator implementing https://www.ietf.org/rfc/rfc1321.txt
    class MD5
    {
    public:
        using Digest = Digest<16>;

        MD5();

        void init();
        void update(const void* data, SlangInt size);
        Digest finalize();

        static Digest compute(const void* data, SlangInt size);

    private:
        const void* processBlock(const void* data, SlangInt size);

        uint32_t m_lo, m_hi;
        uint32_t m_a, m_b, m_c, m_d;
        uint32_t m_block[16];
        uint8_t m_buffer[64];
    };

    /// SHA1 hash generator implementing https://www.ietf.org/rfc/rfc3174.txt
    class SHA1
    {
    public:
        using Digest = Digest<20>;

        SHA1();

        void init();
        void update(const void* data, SlangInt size);
        Digest finalize();

        static Digest compute(const void* data, SlangInt size);

    private:
        void addByte(uint8_t x);
        void processBlock(const uint8_t* ptr);

        uint32_t m_index;
        uint64_t m_bits;
        uint32_t m_state[5];
        uint8_t m_buf[64];
    };

    // Helper class for building hashes.
    template<typename Hash>
    struct DigestBuilder
    {
    public:
        void append(const void* data, SlangInt size)
        {
            m_hash.update(data, size);
        }

        template<typename T, typename std::enable_if<std::is_arithmetic<T>::value || std::is_enum<T>::value, int>::type = 0>
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

        template<SlangInt N>
        void append(const Digest<N>& digest)
        {
            append(digest.data, sizeof(digest.data));
        }

        template<typename T, typename std::enable_if<std::is_pod<T>::value, int>::type = 0>
        void append(const List<T>& list)
        {
            append(list.getBuffer(), list.getCount() * sizeof(T));
        }

        typename Hash::Digest finalize()
        {
            return m_hash.finalize();
        }

    private:
        Hash m_hash;
    };
}
