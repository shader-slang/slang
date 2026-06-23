#ifndef SLANG_CORE_CHAR_ENCODE_H
#define SLANG_CORE_CHAR_ENCODE_H

#include "slang-basic.h"
#include "slang-secure-crt.h"

#include <cstdint>
#include <limits>

namespace Slang
{

// NOTE! Order must be kept the same to match up with
enum class CharEncodeType
{
    UTF8,
    UTF16,
    UTF16Reversed,
    UTF32,
    CountOf,
};

template<typename ReadByteFunc>
Char32 getUnicodePointFromUTF8(const ReadByteFunc& readByte, bool* outInvalid = 0)
{
    Char32 codePoint = 0;
    uint8_t leading = static_cast<uint8_t>(readByte());
    bool valid{true};

    if (leading <= 0x7FU)
    {
        // single byte character (ASCII)
        codePoint = leading;
    }
    else if (leading <= 0xBFU)
    {
        // cannot start with continuation byte 0b10xxxxxx
        valid = false;
    }
    else if (leading <= 0xFEU)
    {
        // The leading byte encodes the number of total bytes as follows:
        //
        // 0b110xxxxx - 2 bytes (max payload: 5 + 1*6 = 11 bits, min value: 1 << 7)
        // 0b1110xxxx - 3 bytes (max payload: 4 + 2*6 = 16 bits, min value: 1 << 11)
        // 0b11110xxx - 4 bytes (max payload: 3 + 3*6 = 21 bits, min value: 1 << 16)
        // 0b111110xx - 5 bytes (max payload: 2 + 4*6 = 26 bits, min value: 1 << 21)
        // 0b1111110x - 6 bytes (max payload: 1 + 5*6 = 31 bits, min value: 1 << 26)
        // 0b11111110 - 7 bytes (max payload: 0 + 6*6 = 36 bits, min value: 1 << 31)
        //
        // Unicode is currently defined to have at most 0x10FFFF code points
        // (4 bytes in UTF-8). We'll allow the full 32-bit code point range.

        uint64_t value{};
        uint64_t minValue{0x80U}; // overlong encoding detection

        // count leading ones, start by 2 since we know that the initial byte is at least 0xC0
        uint8_t mask = 0x20U;
        uint8_t byteCount = 2U;

        while (mask & leading)
        {
            ++byteCount;
            minValue = uint64_t{1} << (11U + (byteCount - 3U) * 5U);
            mask >>= 1U;
        }

        // consume leading byte payload bits
        value = leading & (mask - 1U);

        // rest of the bytes
        for (uint8_t i = 1U; i < byteCount; ++i)
        {
            uint8_t byte = static_cast<uint8_t>(readByte());

            // check that we have a continuation byte
            if ((byte & 0xC0U) != 0x80U)
            {
                // invalid continuation byte
                valid = false;
                break;
            }

            // add payload bits
            value <<= 6U;
            value |= (byte & 0x3FU);
        }

        // check that the value is not over-encoded
        if (value < minValue)
            valid = false;

        // check that the value is within 32 bits
        if (value > std::numeric_limits<uint32_t>::max())
            valid = false;

        codePoint = static_cast<Char32>(value);
    }
    else
        valid = false;

    // return
    if (outInvalid)
        *outInvalid = !valid;
    return valid ? codePoint : 0U;
}

template<typename ReadByteFunc>
Char32 getUnicodePointFromUTF16(const ReadByteFunc& readByte)
{
    uint32_t byte0 = Byte(readByte());
    uint32_t byte1 = Byte(readByte());
    uint32_t word0 = byte0 + (byte1 << 8);
    if (word0 >= 0xD800 && word0 <= 0xDFFF)
    {
        uint32_t byte2 = Byte(readByte());
        uint32_t byte3 = Byte(readByte());
        uint32_t word1 = byte2 + (byte3 << 8);
        return Char32(((word0 & 0x3FF) << 10) + (word1 & 0x3FF) + 0x10000);
    }
    else
        return Char32(word0);
}

template<typename ReadByteFunc>
Char32 getUnicodePointFromUTF16Reversed(const ReadByteFunc& readByte)
{
    uint32_t byte0 = Byte(readByte());
    uint32_t byte1 = Byte(readByte());
    uint32_t word0 = (byte0 << 8) + byte1;
    if (word0 >= 0xD800 && word0 <= 0xDFFF)
    {
        uint32_t byte2 = Byte(readByte());
        uint32_t byte3 = Byte(readByte());
        uint32_t word1 = (byte2 << 8) + byte3;
        return Char32(((word0 & 0x3FF) << 10) + (word1 & 0x3FF));
    }
    else
        return Char32(word0);
}

template<typename ReadByteFunc>
Char32 getUnicodePointFromUTF32(const ReadByteFunc& readByte)
{
    uint32_t byte0 = Byte(readByte());
    uint32_t byte1 = Byte(readByte());
    uint32_t byte2 = Byte(readByte());
    uint32_t byte3 = Byte(readByte());
    return Char32(byte0 + (byte1 << 8) + (byte2 << 16) + (byte3 << 24));
}

// Encode functions return the amount of elements output to the buffer
inline int encodeUnicodePointToUTF8(Char32 codePoint, char* outBuffer)
{
    char* const dst = outBuffer;
    // TODO(JS): This supports 4 + 6 * 3 = 22 bits.
    // The standard allows up to 0x10FFFF.
    if (codePoint <= 0x7F)
    {
        dst[0] = char(codePoint);
        return 1;
    }
    else if (codePoint <= 0x7FF)
    {
        dst[0] = char(0xC0 + (codePoint >> 6));
        dst[1] = char(0x80 + (codePoint & 0x3F));
        return 2;
    }
    else if (codePoint <= 0xFFFF)
    {
        dst[0] = char(0xE0 + (codePoint >> 12));
        dst[1] = char(0x80 + ((codePoint >> 6) & (0x3F)));
        dst[2] = char(0x80 + (codePoint & 0x3F));
        return 3;
    }
    else
    {
        dst[0] = char(0xF0 + (codePoint >> 18));
        dst[1] = char(0x80 + ((codePoint >> 12) & 0x3F));
        dst[2] = char(0x80 + ((codePoint >> 6) & 0x3F));
        dst[3] = char(0x80 + (codePoint & 0x3F));
        return 4;
    }
}

inline int encodeUnicodePointToUTF16(Char32 codePoint, Char16* outBuffer)
{
    Char16* const dst = outBuffer;
    if (codePoint <= 0xD7FF || (codePoint >= 0xE000 && codePoint <= 0xFFFF))
    {
        dst[0] = Char16(codePoint);
        return 1;
    }
    else
    {
        const uint32_t sub = codePoint - 0x10000;
        dst[0] = Char16((sub >> 10) + 0xD800);
        dst[1] = Char16((sub & 0x3FF) + 0xDC00);
        return 2;
    }
}

SLANG_FORCE_INLINE Char16 reverseByteOrder(Char16 val)
{
    return (val >> 8) | (val << 8);
}

inline int encodeUnicodePointToUTF16Reversed(Char32 codePoint, Char16* outBuffer)
{
    Char16* const dst = outBuffer;
    if (codePoint <= 0xD7FF || (codePoint >= 0xE000 && codePoint <= 0xFFFF))
    {
        dst[0] = reverseByteOrder(Char16(codePoint));
        return 1;
    }
    else
    {
        const uint32_t sub = codePoint - 0x10000;
        const uint32_t high = (sub >> 10) + 0xD800;
        const uint32_t low = (sub & 0x3FF) + 0xDC00;
        dst[0] = reverseByteOrder(Char16(high));
        dst[1] = reverseByteOrder(Char16(low));
        return 2;
    }
}

static const Char16 kUTF16Header = 0xFEFF;
static const Char16 kUTF16ReversedHeader = 0xFFFE;

class CharEncoding
{
public:
    static CharEncoding *UTF8, *UTF16, *UTF16Reversed, *UTF32;

    /// Encode Utf8 held in slice append into ioBuffer
    virtual void encode(const UnownedStringSlice& str, List<Byte>& ioBuffer) = 0;
    /// Decode buffer into Utf8 held in ioBuffer
    virtual void decode(const Byte* buffer, int length, List<char>& ioBuffer) = 0;

    virtual ~CharEncoding() {}

    /// Get the encoding type
    CharEncodeType getEncodingType() const { return m_encodingType; }

    /// Given some bytes determines a character encoding type, based on the initial bytes.
    /// If can't be determined will assume UTF8.
    /// Outputs the offset to the first non mark in outOffset
    static CharEncodeType determineEncoding(
        const Byte* bytes,
        size_t bytesCount,
        size_t& outOffset);

    /// Get the
    static CharEncoding* getEncoding(CharEncodeType type) { return g_encoding[Index(type)]; }

    CharEncoding(CharEncodeType encodingType)
        : m_encodingType(encodingType)
    {
    }

protected:
    CharEncodeType m_encodingType;

    static CharEncoding* const g_encoding[Index(CharEncodeType::CountOf)];
};

struct UTF8Util
{
    /// Given a slice calculate the number of code points (unicode chars)
    ///
    /// NOTE! This doesn't check the *validity* of code points/encoding.
    /// Non valid utf8 input or ending starting in partial characters, will produce
    /// undefined results without error.
    static Index calcCodePointCount(const UnownedStringSlice& in);


    /// Given a slice in UTF8, calculate the number of UTF16 characters needed to represent the
    /// string.
    static Index calcUTF16CharCount(const UnownedStringSlice& in);
};

} // namespace Slang

#endif
