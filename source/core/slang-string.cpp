#include "slang-string.h"
#include "slang-text-io.h"

#include "slang-char-util.h"

namespace Slang
{
    // TODO: this belongs in a different file:

    SLANG_RETURN_NEVER void signalUnexpectedError(char const* message)
    {
        // Can be useful to uncomment during debug when problem is on CI
        // printf("Unexpected: %s\n", message);
        throw InternalError(message);
    }

    // OSString

    OSString::OSString()
        : m_begin(nullptr)
        , m_end(nullptr)
    {}

    OSString::OSString(wchar_t* begin, wchar_t* end)
        : m_begin(begin)
        , m_end(end)
    {}

    void OSString::_releaseBuffer()
    {
        if (m_begin)
        {
            delete[] m_begin;
        }
    }

    void OSString::set(const wchar_t* begin, const wchar_t* end)
    {
        if (m_begin)
        {
            delete[] m_begin;
            m_begin = nullptr;
            m_end = nullptr;
        }
        const size_t len = end - begin;
        if (len > 0)
        {
            // TODO(JS): The allocation is only done this way to be compatible with the buffer being detached from an array
            // This is unfortunate, because it means that the allocation stores the size (and alignment fix), which is a shame because we know the size
            m_begin = new wchar_t[len + 1];
            memcpy(m_begin, begin, len * sizeof(wchar_t));
            // Zero terminate
            m_begin[len] = 0;
            m_end = m_begin + len;
        }
    }

    static const wchar_t kEmptyOSString[] = { 0 };

    wchar_t const* OSString::begin() const
    {
        return m_begin ? m_begin : kEmptyOSString;
    }

    wchar_t const* OSString::end() const
    {
        return m_end ? m_end : kEmptyOSString;
    }

    // UnownedStringSlice

    bool UnownedStringSlice::startsWith(UnownedStringSlice const& other) const
    {
        UInt thisSize = getLength();
        UInt otherSize = other.getLength();

        if (otherSize > thisSize)
            return false;

        return UnownedStringSlice(begin(), begin() + otherSize) == other;
    }

    bool UnownedStringSlice::startsWith(char const* str) const
    {
        return startsWith(UnownedTerminatedStringSlice(str));
    }


    bool UnownedStringSlice::endsWith(UnownedStringSlice const& other) const
    {
        UInt thisSize = getLength();
        UInt otherSize = other.getLength();

        if (otherSize > thisSize)
            return false;

        return UnownedStringSlice(
            end() - otherSize, end()) == other;
    }

    bool UnownedStringSlice::endsWith(char const* str) const
    {
        return endsWith(UnownedTerminatedStringSlice(str));
    }

    
    UnownedStringSlice UnownedStringSlice::trim() const
    {
        const char* start = m_begin;
        const char* end = m_end;

        while (start < end && CharUtil::isHorizontalWhitespace(*start)) start++;
        while (end > start && CharUtil::isHorizontalWhitespace(end[-1])) end--;
        return UnownedStringSlice(start, end);
    }

    UnownedStringSlice UnownedStringSlice::trim(char c) const
    {
        const char* start = m_begin;
        const char* end = m_end;

        while (start < end && *start == c) start++;
        while (end > start && end[-1] == c) end--;
        return UnownedStringSlice(start, end);
    }

    // StringSlice

    StringSlice::StringSlice()
        : representation(0)
        , beginIndex(0)
        , endIndex(0)
    {}

    StringSlice::StringSlice(String const& str)
        : representation(str.m_buffer)
        , beginIndex(0)
        , endIndex(str.getLength())
    {}

    StringSlice::StringSlice(String const& str, UInt beginIndex, UInt endIndex)
        : representation(str.m_buffer)
        , beginIndex(beginIndex)
        , endIndex(endIndex)
    {}


    //

    _EndLine EndLine;

    String operator+(const char * op1, const String & op2)
    {
        String result(op1);
        result.append(op2);
        return result;
    }

    String operator+(const String & op1, const char * op2)
    {
        String result(op1);
        result.append(op2);
        return result;
    }

    String operator+(const String & op1, const String & op2)
    {
        String result(op1);
        result.append(op2);
        return result;
    }

    int StringToInt(const String & str, int radix)
    {
        if (str.startsWith("0x"))
            return (int)strtoll(str.getBuffer(), NULL, 16);
        else
            return (int)strtoll(str.getBuffer(), NULL, radix);
    }
    unsigned int StringToUInt(const String & str, int radix)
    {
        if (str.startsWith("0x"))
            return (unsigned int)strtoull(str.getBuffer(), NULL, 16);
        else
            return (unsigned int)strtoull(str.getBuffer(), NULL, radix);
    }
    double StringToDouble(const String & str)
    {
        return (double)strtod(str.getBuffer(), NULL);
    }
    float StringToFloat(const String & str)
    {
        return strtof(str.getBuffer(), NULL);
    }

#if 0
    String String::ReplaceAll(String src, String dst) const
    {
        String rs = *this;
        int index = 0;
        int srcLen = src.length;
        int len = rs.length;
        while ((index = rs.IndexOf(src, index)) != -1)
        {
            rs = rs.SubString(0, index) + dst + rs.SubString(index + srcLen, len - index - srcLen);
            len = rs.length;
        }
        return rs;
    }
#endif

    String String::fromWString(const wchar_t * wstr)
    {
#ifdef _WIN32
        return Slang::Encoding::UTF16->ToString((const char*)wstr, (int)(wcslen(wstr) * sizeof(wchar_t)));
#else
        return Slang::Encoding::UTF32->ToString((const char*)wstr, (int)(wcslen(wstr) * sizeof(wchar_t)));
#endif
    }

    String String::fromWString(const wchar_t * wstr, const wchar_t * wend)
    {
#ifdef _WIN32
        return Slang::Encoding::UTF16->ToString((const char*)wstr, (int)((wend - wstr) * sizeof(wchar_t)));
#else
        return Slang::Encoding::UTF32->ToString((const char*)wstr, (int)((wend - wstr) * sizeof(wchar_t)));
#endif
    }

    String String::fromWChar(const wchar_t ch)
    {
#ifdef _WIN32
        return Slang::Encoding::UTF16->ToString((const char*)&ch, (int)(sizeof(wchar_t)));
#else
        return Slang::Encoding::UTF32->ToString((const char*)&ch, (int)(sizeof(wchar_t)));
#endif
    }

    String String::fromUnicodePoint(unsigned int codePoint)
    {
        char buf[6];
        int len = Slang::EncodeUnicodePointToUTF8(buf, (int)codePoint);
        buf[len] = 0;
        return String(buf);
    }

    OSString String::toWString(Index* outLength) const
    {
        if (!m_buffer)
        {
            return OSString();
        }
        else
        {
            List<char> buf;
            switch(sizeof(wchar_t))
            {
            case 2:
                Slang::Encoding::UTF16->GetBytes(buf, *this);                
                break;

            case 4:
                Slang::Encoding::UTF32->GetBytes(buf, *this);                
                break;

            default:
                break;
            }

            auto length = Index(buf.getCount() / sizeof(wchar_t));
            if (outLength)
                *outLength = length;

            for(int ii = 0; ii < sizeof(wchar_t); ++ii)
                buf.add(0);

            wchar_t* beginData = (wchar_t*)buf.getBuffer();
            wchar_t* endData = beginData + length;

            buf.detachBuffer();

            return OSString(beginData, endData);
        }
    }

    //

    void String::ensureUniqueStorageWithCapacity(Index requiredCapacity)
    {
        if (m_buffer && m_buffer->isUniquelyReferenced() && m_buffer->capacity >= requiredCapacity)
            return;

        Index newCapacity = m_buffer ? 2 * m_buffer->capacity : 16;
        if (newCapacity < requiredCapacity)
        {
            newCapacity = requiredCapacity;
        }

        Index length = getLength();
        StringRepresentation* newRepresentation = StringRepresentation::createWithCapacityAndLength(newCapacity, length);

        if (m_buffer)
        {
            memcpy(newRepresentation->getData(), m_buffer->getData(), length + 1);
        }

        m_buffer = newRepresentation;
    }

    char* String::prepareForAppend(Index count)
    {
        auto oldLength = getLength();
        auto newLength = oldLength + count;
        ensureUniqueStorageWithCapacity(newLength);
        return getData() + oldLength;
    }
    void String::appendInPlace(const char* chars, Index count)
    {
        SLANG_UNUSED(chars);

        if (count > 0)
        {
            SLANG_ASSERT(m_buffer && m_buffer->isUniquelyReferenced());

            auto oldLength = getLength();
            auto newLength = oldLength + count;

            char* dst = m_buffer->getData();

            // Make sure the input buffer is the same one returned from prepareForAppend
            SLANG_ASSERT(chars == dst + oldLength);
            // It has to fit within the capacity
            SLANG_ASSERT(newLength <= m_buffer->capacity);

            // We just need to modify the length
            m_buffer->length = newLength;

            // And mark with a terminating 0
            dst[newLength] = 0;
        }
    }

    void String::append(const char* textBegin, char const* textEnd)
    {
        auto oldLength = getLength();
        auto textLength = textEnd - textBegin;

        auto newLength = oldLength + textLength;

        ensureUniqueStorageWithCapacity(newLength);

        memcpy(getData() + oldLength, textBegin, textLength);
        getData()[newLength] = 0;
        m_buffer->length = newLength;
    }

    void String::append(char const* str)
    {
        if (str)
        {
            append(str, str + strlen(str));
        }
    }

    void String::appendRepeatedChar(char chr, Index count)
    {
        SLANG_ASSERT(count >= 0);
        if (count > 0)
        { 
            char* chars = prepareForAppend(count);
            // Set all space to repeated chr. 
            ::memset(chars, chr, sizeof(char) * count);
            appendInPlace(chars, count);
        }
    }

    void String::appendChar(char c)
    {
        const auto oldLength = getLength();
        const auto newLength = oldLength + 1;

        ensureUniqueStorageWithCapacity(newLength);

        // Since there must be space for at least one character, m_buffer cannot be nullptr
        SLANG_ASSERT(m_buffer);
        char* data = m_buffer->getData();
        data[oldLength] = c;
        data[newLength] = 0;

        m_buffer->length = newLength;
    }

    void String::append(char chr)
    {
        appendChar(chr);
    }

    void String::append(String const& str)
    {
        if (!m_buffer)
        {
            m_buffer = str.m_buffer;
            return;
        }

        append(str.begin(), str.end());
    }

    void String::append(StringSlice const& slice)
    {
        append(slice.begin(), slice.end());
    }

    void String::append(UnownedStringSlice const& slice)
    {
        append(slice.begin(), slice.end());
    }

    void String::append(int32_t value, int radix)
    {
        enum { kCount = 33 };
        char* data = prepareForAppend(kCount);
        auto count = IntToAscii(data, value, radix);
        ReverseInternalAscii(data, count);
        m_buffer->length += count;
    }

    void String::append(uint32_t value, int radix)
    {
        enum { kCount = 33 };
        char* data = prepareForAppend(kCount);
        auto count = IntToAscii(data, value, radix);
        ReverseInternalAscii(data, count);
        m_buffer->length += count;
    }

    void String::append(int64_t value, int radix)
    {
        enum { kCount = 65 };
        char* data = prepareForAppend(kCount);
        auto count = IntToAscii(data, value, radix);
        ReverseInternalAscii(data, count);
        m_buffer->length += count;
    }

    void String::append(uint64_t value, int radix)
    {
        enum { kCount = 65 };
        char* data = prepareForAppend(kCount);
        auto count = IntToAscii(data, value, radix);
        ReverseInternalAscii(data, count);
        m_buffer->length += count;
    }

    void String::append(float val, const char * format)
    {
        enum { kCount = 128 };
        char* data = prepareForAppend(kCount);
        sprintf_s(data, kCount, format, val);
        m_buffer->length += strnlen_s(data, kCount);
    }

    void String::append(double val, const char * format)
    {
        enum { kCount = 128 };
        char* data = prepareForAppend(kCount);
        sprintf_s(data, kCount, format, val);
        m_buffer->length += strnlen_s(data, kCount);
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! UnownedStringSlice !!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    Index UnownedStringSlice::indexOf(char c) const
    {
        const Index size = Index(m_end - m_begin);
        for (Index i = 0; i < size; ++i)
        {
            if (m_begin[i] == c)
            {
                return i;
            }
        }
        return -1;
    }

    Index UnownedStringSlice::indexOf(const UnownedStringSlice& in) const
    {
        const Index len = getLength();
        const Index inLen = in.getLength();
        if (inLen > len)
        {
            return -1;
        }

        const char* inChars = in.m_begin;
        switch (inLen)
        {
            case 0:         return 0;
            case 1:         return indexOf(inChars[0]);
            default: break;
        }

        const char* chars = m_begin;
        const char firstChar = inChars[0];

        for (Int i = 0; i < len - inLen; ++i)
        {
            if (chars[i] == firstChar && in == UnownedStringSlice(chars + i, inLen))
            {
                return i;
            }
        }

        return -1;
    }

    UnownedStringSlice UnownedStringSlice::subString(Index idx, Index len) const
    {
        const Index totalLen = getLength();
        SLANG_ASSERT(idx >= 0 && len >= 0 && idx <= totalLen);

        // If too large, we truncate
        len = (idx + len > totalLen) ? (totalLen - idx) : len;

        // Return the substring
        return UnownedStringSlice(m_begin + idx, m_begin + idx + len);
    }

    bool UnownedStringSlice::operator==(ThisType const& other) const
    {
        // Note that memcmp is undefined when passed in null ptrs, so if we want to handle
        // we need to cover that case.
        // Can only be nullptr if size is 0.
        auto thisSize = getLength();
        auto otherSize = other.getLength();

        if (thisSize != otherSize)
        {
            return false;
        }

        const char*const thisChars = begin();
        const char*const otherChars = other.begin();
        if (thisChars == otherChars || thisSize == 0)
        {
            return true;
        }
        SLANG_ASSERT(thisChars && otherChars);
        return memcmp(thisChars, otherChars, thisSize) == 0;
    }

    bool UnownedStringSlice::caseInsensitiveEquals(const ThisType& rhs) const
    {
        const auto length = getLength();
        if (length != rhs.getLength())
        {
            return false;
        }

        const char* a = m_begin;
        const char* b = rhs.m_begin;

        // Assuming this is a faster test
        if (memcmp(a, b, length) != 0)
        {
            // They aren't identical so compare character by character
            for (Index i = 0; i < length; ++i)
            {
                if (CharUtil::toLower(a[i]) != CharUtil::toLower(b[i]))
                {
                    return false;
                }
            }
        }

        return true;
    }

}
