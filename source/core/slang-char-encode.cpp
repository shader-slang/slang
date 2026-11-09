#include "slang-char-encode.h"

namespace Slang
{

// Return the number of terminal columns occupied by a single Unicode code
// point. This is a coarse approximation of POSIX `wcwidth`: zero-width
// combining marks count as 0, the common East-Asian "wide" and "fullwidth"
// ranges count as 2, and everything else counts as 1. It exists so caret/
// underline rows line up under multi-byte and wide characters in source
// snippets (e.g. CJK text), which a naive byte- or code-point count gets wrong.
Int64 UTF8Util::getCodePointDisplayWidth(Char32 c)
{
    // A tab advances by an indeterminate amount, but since the renderer emits
    // the source line verbatim (tabs are not expanded), counting it as a single
    // column keeps the caret line consistent with the rest of this code, which
    // also treats one tab as one column (e.g. indent detection). This matches
    // the pre-existing behavior before display-width handling was introduced.
    if (c == '\t')
        return 1;

    // Other control characters (NUL, the C0 range, and the C1 range) have no
    // sensible width; treat as zero so they don't shift the underline.
    if (c == 0 || c < 0x20 || (c >= 0x7f && c < 0xa0))
        return 0;

    // Zero-width combining marks and joiners.
    if ((c >= 0x0300 && c <= 0x036f) || // combining diacritical marks
        (c >= 0x1ab0 && c <= 0x1aff) || // combining diacritical marks extended
        (c >= 0x1dc0 && c <= 0x1dff) || // combining diacritical marks supplement
        (c >= 0x20d0 && c <= 0x20ff) || // combining diacritical marks for symbols
        (c >= 0xfe20 && c <= 0xfe2f) || // combining half marks
        c == 0x200b ||                  // zero-width space
        c == 0x200d)                    // zero-width joiner
        return 0;

    // East-Asian wide and fullwidth ranges (subset covering the common cases).
    if ((c >= 0x1100 && c <= 0x115f) || // Hangul Jamo
        (c >= 0x2e80 && c <= 0x303e) || // CJK radicals, Kangxi, symbols
        (c >= 0x3041 && c <= 0x33ff) || // Hiragana, Katakana, CJK symbols
        (c >= 0x3400 && c <= 0x4dbf) || // CJK Unified Ideographs Extension A
        (c >= 0x4e00 && c <= 0x9fff) || // CJK Unified Ideographs
        (c >= 0xa000 && c <= 0xa4cf) || // Yi
        (c >= 0xac00 && c <= 0xd7a3) || // Hangul Syllables
        (c >= 0xf900 && c <= 0xfaff) || // CJK Compatibility Ideographs
        (c >= 0xfe30 && c <= 0xfe4f) || // CJK Compatibility Forms
        (c >= 0xff00 && c <= 0xff60) || // Fullwidth Forms
        (c >= 0xffe0 && c <= 0xffe6) || // Fullwidth signs
        (c >= 0x20000 && c <= 0x3fffd)) // CJK Unified Ideographs Extension B+
        return 2;

    return 1;
}

class Utf8CharEncoding : public CharEncoding
{
public:
    typedef CharEncoding Super;

    virtual void encode(const UnownedStringSlice& slice, List<Byte>& ioBuffer) override
    {
        ioBuffer.addRange((const Byte*)slice.begin(), slice.getLength());
    }
    virtual void decode(const Byte* bytes, int length, List<char>& ioChars) override
    {
        ioChars.addRange((const char*)bytes, length);
    }
    Utf8CharEncoding()
        : Super(CharEncodeType::UTF8)
    {
    }
};

class Utf32CharEncoding : public CharEncoding
{
public:
    typedef CharEncoding Super;

    virtual void encode(const UnownedStringSlice& slice, List<Byte>& ioBuffer) override
    {
        Index ptr = 0;
        while (ptr < slice.getLength())
        {
            const Char32 codePoint = getUnicodePointFromUTF8(
                [&]() -> Byte
                {
                    if (ptr < slice.getLength())
                        return slice[ptr++];
                    else
                        return '\0';
                });
            // Note: Assumes byte order is same as arch byte order
            ioBuffer.addRange((const Byte*)&codePoint, 4);
        }
    }
    virtual void decode(const Byte* bytes, int length, List<char>& ioBuffer) override
    {
        // Note: Assumes bytes is Char32 aligned
        SLANG_ASSERT((size_t(bytes) & 3) == 0);
        const Char32* content = (const Char32*)bytes;
        for (int i = 0; i < (length >> 2); i++)
        {
            char buf[5];
            int count = encodeUnicodePointToUTF8(content[i], buf);
            for (int j = 0; j < count; j++)
                ioBuffer.addRange(buf, count);
        }
    }

    Utf32CharEncoding()
        : Super(CharEncodeType::UTF32)
    {
    }
};

class Utf16CharEncoding : public CharEncoding // UTF16
{
public:
    typedef CharEncoding Super;
    Utf16CharEncoding(bool reverseOrder)
        : Super(reverseOrder ? CharEncodeType::UTF16Reversed : CharEncodeType::UTF16)
        , m_reverseOrder(reverseOrder)
    {
    }
    virtual void encode(const UnownedStringSlice& slice, List<Byte>& ioBuffer) override
    {
        Index index = 0;
        while (index < slice.getLength())
        {
            const Char32 codePoint = getUnicodePointFromUTF8(
                [&]() -> Byte
                {
                    if (index < slice.getLength())
                        return slice[index++];
                    else
                        return '\0';
                });

            Char16 buffer[2];
            int count;
            if (!m_reverseOrder)
                count = encodeUnicodePointToUTF16(codePoint, buffer);
            else
                count = encodeUnicodePointToUTF16Reversed(codePoint, buffer);
            ioBuffer.addRange((const Byte*)buffer, count * 2);
        }
    }
    virtual void decode(const Byte* bytes, int length, List<char>& ioBuffer) override
    {
        Index index = 0;
        while (index < length)
        {
            auto readByte = [&]() -> Byte { return (index < length) ? bytes[index++] : Byte(0); };
            const Char32 codePoint = m_reverseOrder ? getUnicodePointFromUTF16Reversed(readByte)
                                                    : getUnicodePointFromUTF16(readByte);

            char buf[5];
            int count = encodeUnicodePointToUTF8(codePoint, buf);
            ioBuffer.addRange((const char*)buf, count);
        }
    }

private:
    bool m_reverseOrder = false;
};

/* static */ CharEncodeType CharEncoding::determineEncoding(
    const Byte* bytes,
    size_t bytesCount,
    size_t& outOffset)
{
    // TODO(JS): Assumes the bytes are suitably aligned

    if (bytesCount >= 3 && bytes[0] == 0xef && bytes[1] == 0xbb && bytes[2] == 0xbf)
    {
        outOffset = 3;
        return CharEncodeType::UTF8;
    }
    else if (bytesCount >= 2 && ((bytesCount % 2) == 0))
    {
        Char16 c;
        ::memcpy(&c, bytes, 2);

        if (c == kUTF16Header)
        {
            outOffset = 2;
            return CharEncodeType::UTF16;
        }
        else if (c == kUTF16ReversedHeader)
        {
            outOffset = 2;
            return CharEncodeType::UTF16Reversed;
        }

        // If we don't have a 'mark' byte then we are bit stumped. We'll look for
        // null (non-terminator) bytes and assume they mean we have a 16-bit encoding
        for (size_t i = 0; i < (bytesCount - 1); i += 2)
        {
#if SLANG_LITTLE_ENDIAN
            const auto low = bytes[i];
            const auto high = bytes[i + 1];
#else
            const auto low = bytes[i + 1];
            const auto high = bytes[i];
#endif
            if ((low == 0) ^ (high == 0))
            {
                outOffset = 0;
                return (high == 0) ? CharEncodeType::UTF16 : CharEncodeType::UTF16Reversed;
            }
        }
    }

    // Assume it's UTF8 or 7 bit ascii which UTF8 is a superset of
    outOffset = 0;
    return CharEncodeType::UTF8;
}

static Utf8CharEncoding _utf8Encoding;
static Utf16CharEncoding _utf16Encoding(false);
static Utf16CharEncoding _utf16EncodingReversed(true);
static Utf32CharEncoding _utf32Encoding;

/* static */ CharEncoding* const CharEncoding::g_encoding[Index(CharEncodeType::CountOf)]{
    &_utf8Encoding,          // UTF8,
    &_utf16Encoding,         // UTF16,
    &_utf16EncodingReversed, // UTF16Reversed,
    &_utf32Encoding,         // UTF32,
};

CharEncoding* CharEncoding::UTF8 = &_utf8Encoding;
CharEncoding* CharEncoding::UTF16 = &_utf16Encoding;
CharEncoding* CharEncoding::UTF16Reversed = &_utf16EncodingReversed;
CharEncoding* CharEncoding::UTF32 = &_utf32Encoding;

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! UTF8Util !!!!!!!!!!!!!!!!!!!!!!!!! */

/* static */ Index UTF8Util::calcCodePointCount(const UnownedStringSlice& in)
{
    Index count = 0;

    // Analyse with bytes...
    const int8_t* cur = (const int8_t*)in.begin();
    const int8_t* const end = (const int8_t*)in.end();

    while (cur < end)
    {
        const auto c = *cur++;

        count++;

        // If c < 0 it means the top bit is set... which means we have multiple bytes
        if (c < 0)
        {
            // https://en.wikipedia.org/wiki/UTF-8
            // All continuation bytes contain exactly six bits from the code point.So the next six
            // bits of the code point
            /// are stored in the low order six bits of the next byte, and 10 is stored in the high
            /// order two bits to
            // mark it as a continuation byte(so 10000010).

            while (cur < end && (*cur & 0xc0) == 0x80)
            {
                cur++;
            }
        }
    }

    return count;
}

Index UTF8Util::calcUTF16CharCount(const UnownedStringSlice& in)
{
    Index count = 0;
    Index readPtr = 0;
    for (;;)
    {
        int c = getUnicodePointFromUTF8(
            [&]() -> Byte
            {
                if (readPtr < in.getLength())
                    return in[readPtr++];
                else
                    return 0;
            });
        if (c == 0)
            break;
        Char16 buffer[2];
        count += encodeUnicodePointToUTF16(c, buffer);
        if (readPtr >= in.getLength())
            break;
    }
    return count;
}

} // namespace Slang
