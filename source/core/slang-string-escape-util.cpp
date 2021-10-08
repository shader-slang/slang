#include "slang-string-escape-util.h"

#include "slang-char-util.h"
#include "slang-text-io.h"
#include "slang-memory-arena.h"

#include "../../slang-com-helper.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!! SpaceStringEscapeHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class SpaceStringEscapeHandler : public StringEscapeHandler
{
public:
    typedef StringEscapeHandler Super;

    virtual bool isQuotingNeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE { return isEscapingNeeded(slice); }
 
    virtual bool isEscapingNeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE;
    virtual bool isUnescapingNeeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE;

    virtual SlangResult appendEscaped(const UnownedStringSlice& slice, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult lexQuoted(const char* cursor, const char** outCursor) SLANG_OVERRIDE;

    SpaceStringEscapeHandler() : Super('"') {}
};

bool SpaceStringEscapeHandler::isEscapingNeeded(const UnownedStringSlice& slice)
{
    return slice.indexOf(' ') >= 0;
}

bool SpaceStringEscapeHandler::isUnescapingNeeeded(const UnownedStringSlice& slice)
{
    SLANG_UNUSED(slice);
    // As it stands we never have to unescape
    return false;
}

SlangResult SpaceStringEscapeHandler::appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    if (slice.indexOf('"') >= 0)
    {
        return SLANG_FAIL;
    }

    out.append(slice);
    return SLANG_OK;
}

SlangResult SpaceStringEscapeHandler::appendEscaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    if (slice.indexOf('"') >= 0)
    {
        return SLANG_FAIL;
    }
    out.append(slice);
    return SLANG_OK;
}

/* static */SlangResult SpaceStringEscapeHandler::lexQuoted(const char* cursor, const char** outCursor)
{
    *outCursor = cursor;

    if (*cursor != m_quoteChar)
    {
        return SLANG_FAIL;
    }
    cursor++;

    for (;;)
    {
        const char c = *cursor;
        if (c == m_quoteChar)
        {
            *outCursor = cursor + 1;
            return SLANG_OK;
        }
        switch (c)
        {
            case 0:
            case '\n':
            case '\r':
            {
                // Didn't hit closing quote!
                return SLANG_FAIL;
            }
            default:
            {
                ++cursor;
                break;
            }
        }
    }
}



// !!!!!!!!!!!!!!!!!!!!!!!!!! CppStringEscapeHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class CppStringEscapeHandler : public StringEscapeHandler
{
public:
    typedef StringEscapeHandler Super;

    virtual bool isQuotingNeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE { SLANG_UNUSED(slice); return true; }
    virtual bool isEscapingNeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE;
    virtual bool isUnescapingNeeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE;
    virtual SlangResult appendEscaped(const UnownedStringSlice& slice, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult lexQuoted(const char* cursor, const char** outCursor) SLANG_OVERRIDE;

    CppStringEscapeHandler() : Super('"') {}
};

static char _getHexChar(int v)
{
    return (v <= 9) ? char(v + '0') : char(v - 10 + 'A');
}

static int _getHexDigit(char c)
{
    if (c >= '0' && c <= '9')
    {
        return c - '0';
    }
    else if (c >= 'a' && c <= 'f')
    {
        return c - 'a' + 10;
    }
    else if (c >= 'A' && c <= 'F')
    {
        return c - 'A' + 10;
    }
    else
    {
        SLANG_ASSERT(!"Not a hex digit");
        return 0;
    }
}

static char _getCppEscapedChar(char c)
{
    switch (c)
    {
        case '\b':      return 'b';
        case '\f':      return 'f';
        case '\n':      return 'n';
        case '\r':      return 'r';
        case '\a':      return 'a';
        case '\t':      return 't';
        case '\v':      return 'v';
        case '\'':      return '\'';
        case '\"':      return '"';
        case '\\':      return '\\';
        default:        return 0;
    }
}

static char _getCppUnescapedChar(char c)
{
    switch (c)
    {
        case 'b':      return '\b';
        case 'f':      return '\f';
        case 'n':      return '\n';
        case 'r':      return '\r';
        case 'a':      return '\a';
        case 't':      return '\t';
        case 'v':      return '\v';
        case '\'':      return '\'';
        case '\"':      return '"';
        case '\\':      return '\\';
        default:        return 0;
    }
}


bool CppStringEscapeHandler::isUnescapingNeeeded(const UnownedStringSlice& slice)
{
    return slice.indexOf('\\') >= 0;
}

/* static */bool CppStringEscapeHandler::isEscapingNeeded(const UnownedStringSlice& slice)
{
    const char* cur = slice.begin();
    const char*const end = slice.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;

        switch (c)
        {
            case '\'':
            case '\"':
            case '\\':
            {
                // Strictly speaking ' shouldn't need a quote if in a C style string. 
                return true;
            }
            default:
            {
                if (c < ' ' || c >= 0x7e)
                {
                    return true;
                }
                break;
            }
        }
    }
    return false;
}

SlangResult CppStringEscapeHandler::appendEscaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    const char* start = slice.begin();
    const char* cur = start;
    const char*const end = slice.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;
        const char escapedChar = _getCppEscapedChar(c);

        if (escapedChar)
        {
            // Flush
            if (start < cur)
            {
                out.append(start, cur);
            }
            out.appendChar('\\');
            out.appendChar(escapedChar);

            start = cur + 1;
        }
        else if (c < ' ' || c > 126)
        {
            // Flush
            if (start < cur)
            {
                out.append(start, cur);
            }

            char buf[5] = "\\0x0";

            buf[3] = _getHexChar((int(c) >> 4) & 0xf);
            buf[4] = _getHexChar(c & 0xf);

            out.append(buf, buf + 4);

            start = cur + 1;
        }
    }

    if (start < end)
    {
        out.append(start, end);
    }
    return SLANG_OK;
}

SlangResult CppStringEscapeHandler::appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    const char* start = slice.begin();
    const char* cur = start;
    const char*const end = slice.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;

        if (c == '\\')
        {
            // Flush
            if (start < end)
            {
                out.append(start, end);
            }

            /// Next 
            cur++;

            if (cur >= end)
            {
                return SLANG_FAIL;
            }

            // Need to handle various escape sequence cases
            switch (*cur)
            {
                case '\'':
                case '\"':
                case '\\':
                case '?':
                case 'a':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                case 'v':
                {
                    const char unescapedChar = _getCppUnescapedChar(*cur);
                    if (unescapedChar == 0)
                    {
                        // Don't know how to unescape that char
                        return SLANG_FAIL;
                    }
                    out.appendChar(unescapedChar);

                    start = cur + 1;
                    break;
                }
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7':
                {
                    // octal escape: up to 3 characters
                    ++cur;
                    int value = 0;

                    const char* octEnd = cur + 3;
                    octEnd = (octEnd > end) ? end : octEnd;

                    for (; cur < octEnd; ++cur)
                    {
                        const char d = *cur;
                        if (d >= '0' && d <= '7')
                        {
                            value = (value << 3) | (d - '0');
                        }
                    }
                    out.appendChar(char(value));

                    start = cur;
                    break;
                }
                case 'x':
                {
                    uint32_t value = 0;
                    for (++cur; cur < end && CharUtil::isHexDigit(*cur); ++cur)
                    {
                        value = value << 4 | _getHexDigit(*cur);
                    }

                    // It's arguable what is appropriate. We only decode/encode 4, which the current spec has,
                    // but 6 are possible, so lets go large.
                    const Index maxUtf8EncodeCount = 6;

                    char* chars = out.prepareForAppend(maxUtf8EncodeCount);

                    int numChars = EncodeUnicodePointToUTF8(chars, int(value));
                    out.appendInPlace(chars, numChars);

                    start = cur;
                    break;
                }
                default:
                {
                    return SLANG_FAIL;
                }
            }
        }
    }

    if (start < end)
    {
        out.append(start, end);
    }

    return SLANG_OK;
}

SlangResult CppStringEscapeHandler::lexQuoted(const char* cursor, const char** outCursor)
{
    *outCursor = cursor;

    if (*cursor != m_quoteChar)
    {
        return SLANG_FAIL;
    }
    cursor++;

    for (;;)
    {
        const char c = *cursor;
        if (c == m_quoteChar)
        {
            *outCursor = cursor + 1;
            return SLANG_OK;
        }
        switch (c)
        {
            case 0:
            case '\n':
            case '\r':
            {
                // Didn't hit closing quote!
                return SLANG_FAIL;
            }
            case '\\':
            {
                ++cursor;
                // Need to handle various escape sequence cases
                switch (*cursor)
                {
                    case '\'':
                    case '\"':
                    case '\\':
                    case '?':
                    case 'a':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    case 'v':
                    {
                        ++cursor;
                        break;
                    }
                    case '0': case '1': case '2': case '3': case '4':
                    case '5': case '6': case '7':
                    {
                        // octal escape: up to 3 characters
                        ++cursor;
                        for (int ii = 0; ii < 3; ++ii)
                        {
                            const char d = *cursor;
                            if (('0' <= d) && (d <= '7'))
                            {
                                ++cursor;
                                continue;
                            }
                            else
                            {
                                break;
                            }
                        }
                        break;
                    }
                    case 'x':
                    {
                        // hexadecimal escape: any number of characters
                        ++cursor;
                        for (; CharUtil::isHexDigit(*cursor); ++cursor);

                        // TODO: Unicode escape sequences
                        break;
                    }
                }
                break;
            }
            default:
            {
                ++cursor;
                break;
            }
        }
    }
}

// !!!!!!!!!!!!!!!!!!!!!!!!!! JSONStringEscapeHandler !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

class JSONStringEscapeHandler : public StringEscapeHandler
{
public:
    typedef StringEscapeHandler Super;

    virtual bool isQuotingNeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE { SLANG_UNUSED(slice); return true; }
    virtual bool isEscapingNeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE;
    virtual bool isUnescapingNeeeded(const UnownedStringSlice& slice) SLANG_OVERRIDE;
    virtual SlangResult appendEscaped(const UnownedStringSlice& slice, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out) SLANG_OVERRIDE;
    virtual SlangResult lexQuoted(const char* cursor, const char** outCursor) SLANG_OVERRIDE;

    JSONStringEscapeHandler() : Super('"') {}
};

bool JSONStringEscapeHandler::isUnescapingNeeeded(const UnownedStringSlice& slice)
{
    return slice.indexOf('\\') >= 0;
}

bool JSONStringEscapeHandler::isEscapingNeeded(const UnownedStringSlice& slice)
{
    const char* cur = slice.begin();
    const char*const end = slice.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;

        switch (c)
        {
            case '\"':
            case '\\':
            case '/':
            {
                return true;
            }
            default:
            {
                if (c < ' ' || c >= 0x7e)
                {
                    return true;
                }
                break;
            }
        }
    }
    return false;
}

SlangResult JSONStringEscapeHandler::lexQuoted(const char* cursor, const char** outCursor)
{
    // We've skipped the first "
    while (true)
    {
        const char c = *cursor++;

        switch (c)
        {
            case 0:     return SLANG_FAIL;
            case '"':
            {
                *outCursor = cursor;
                return SLANG_OK;
            }
            case '\\':
            {
                const char nextC = *cursor;
                switch (nextC)
                {
                    case '"':
                    case '\\':
                    case '/':
                    case 'b':
                    case 'f':
                    case 'n':
                    case 'r':
                    case 't':
                    {
                        ++cursor;
                        break;
                    }
                    case 'u':
                    {
                        cursor++;
                        for (Index i = 0; i < 4; ++i)
                        {
                            if (!CharUtil::isHexDigit(cursor[i]))
                            {
                                return SLANG_FAIL;
                            }
                        }
                        cursor += 4;
                        break;
                    }
                }
            }
            // Somewhat surprisingly it appears it's valid to have \r\n inside of quotes.
            default: break;
        }
    }
}

static char _getJSONEscapedChar(char c)
{
    switch (c)
    {
        case '\b':      return 'b';
        case '\f':      return 'f';
        case '\n':      return 'n';
        case '\r':      return 'r';
        case '\t':      return 't';
        case '\\':      return '\\';
        case '/':       return '/';
        case '"':       return '"';
        default:        return 0;
    }
}

static char _getJSONUnescapedChar(char c)
{
    switch (c)
    {
        case 'b':      return '\b';
        case 'f':      return '\f';
        case 'n':      return '\n';
        case 'r':      return '\r';
        case 't':      return '\t';
        case '\\':      return '\\';
        case '/':       return '/';
        case '"':       return '"';
        default:        return 0;
    }
}

static const char s_hex[] = "0123456789abcdef";

// Outputs ioSlice with the chars remaining after utf8 encoded value
// Returns ~uint32_t(0) if can't decode
static uint32_t _getUnicodePointFromUTF8(UnownedStringSlice& ioSlice)
{
    const Index length = ioSlice.getLength();
    SLANG_ASSERT(length > 0);
    const char* cur = ioSlice.begin();

    uint32_t codePoint = 0;
    unsigned int leading = cur[0];
    unsigned int mask = 0x80;

    Index count = 0;
    while (leading & mask)
    {
        count++;
        mask >>= 1;
    }

    if (count > length)
    {
        SLANG_ASSERT(!"Can't decode");
        ioSlice = UnownedStringSlice(ioSlice.end(), ioSlice.end());
        return ~uint32_t(0);
    }

    codePoint = (leading & (mask - 1));
    for (Index i = 1; i <= count - 1; i++)
    {
        codePoint <<= 6;
        codePoint += (cur[i] & 0x3F);
    }

    ioSlice = UnownedStringSlice(cur + count, ioSlice.end());
    return codePoint;
}

static void _appendHex16(uint32_t value, StringBuilder& out)
{
    // Let's go with hex
    char buf[] = "\\u0000";

    buf[2] = s_hex[(value >> 12) & 0xf];
    buf[3] = s_hex[(value >> 8) & 0xf];
    buf[4] = s_hex[(value >> 4) & 0xf];
    buf[5] = s_hex[(value >> 0) & 0xf];

    out.append(UnownedStringSlice(buf, 6));
}

SlangResult JSONStringEscapeHandler::appendEscaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    const char* start = slice.begin();
    const char* cur = start;
    const char*const end = slice.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;
        
        const char escapedChar = _getJSONEscapedChar(c);

        if (escapedChar)
        {
            // Flush
            if (start < cur)
            {
                out.append(start, cur);
            }
            out.appendChar('\\');
            out.appendChar(escapedChar);

            start = cur + 1;
        }
        else if (uint8_t(c) & 0x80)
        {
            // Flush
            if (start < cur)
            {
                out.append(start, cur);
            }

            // UTF8
            UnownedStringSlice remainingSlice(cur, end);
            uint32_t codePoint = _getUnicodePointFromUTF8(remainingSlice);

            // We only support up to 16 bit unicode values for now...
            SLANG_ASSERT(codePoint < 0x10000);

            _appendHex16(codePoint, out);

            cur = remainingSlice.begin() - 1;
            start = cur + 1;
        }
        else if (uint8_t(c) < ' ' || (c >= 0x7e))
        {
            if (start < cur)
            {
                out.append(start, cur);
            }

            _appendHex16(uint32_t(c), out);

            start = cur + 1;
        }
        else
        {
            // Can go out as it is
        }
    }

    // Flush at the end
    if (start < end)
    {
        out.append(start, end);
    }
    return SLANG_OK;
}

SlangResult JSONStringEscapeHandler::appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    const char* start = slice.begin();
    const char* cur = start;
    const char*const end = slice.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;

        if (c == '\\')
        {
            // Flush
            if (start < cur)
            {
                out.append(start, cur);
            }

            /// Next 
            cur++;

            if (cur >= end)
            {
                return SLANG_FAIL;
            }

            // Need to handle various escape sequence cases
            switch (*cur)
            {
                case '\"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                {
                    const char unescapedChar = _getJSONUnescapedChar(*cur);
                    if (unescapedChar == 0)
                    {
                        // Don't know how to unescape that char
                        return SLANG_FAIL;
                    }
                    out.appendChar(unescapedChar);

                    start = cur + 1;
                    break;
                }
                case 'u':
                {
                    uint32_t value = 0;
                    cur++;

                    if (cur + 4 > end)
                    {
                        return SLANG_FAIL;
                    }

                    for (Index i = 0; i < 4; ++i)
                    {
                        const char digitC = cur[i];

                        uint32_t digitValue;
                        if (digitC >= '0' && digitC <= '9')
                        {
                            digitValue = digitC - '0';
                        }
                        else if (digitC >= 'a' && digitC <= 'f')
                        {
                            digitValue = digitC -'a' + 10;
                        }
                        else if(digitC >= 'A' && digitC <= 'F')
                        {
                            digitValue = digitC - 'A' + 10;
                        }
                        else
                        {
                            return SLANG_FAIL;
                        }
                        SLANG_ASSERT(digitValue < 0x10);
                        value = (value << 4) | digitValue;
                    }
                    cur += 4;

                    // NOTE! Strictly speaking we may want to combine 2 UTF16 surrogates to make a single
                    // UTF8 encoded char.
                    
                    // Need to encode in UTF8 to concat

                    char buf[8];
                    int len = EncodeUnicodePointToUTF8(buf, value);

                    out.append(buf, buf + len);

                    start = cur;
                    cur--;
                    break;
                }
                default:
                {
                    // Can't decode
                    return SLANG_FAIL;
                }
            }
        }
    }

    // Flush
    if (start < end)
    {
        out.append(start, end);
    }

    return SLANG_OK;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!! StringEscapeUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

static CppStringEscapeHandler g_cppHandler;
static SpaceStringEscapeHandler g_spaceHandler;
static JSONStringEscapeHandler g_jsonHandler;

StringEscapeUtil::Handler* StringEscapeUtil::getHandler(Style style)
{
    switch (style)
    {
        case Style::Cpp:    return &g_cppHandler;
        case Style::Space:  return &g_spaceHandler;
        case Style::JSON:   return &g_jsonHandler;
        default:            return nullptr;
    }
}

/* static */SlangResult StringEscapeUtil::appendQuoted(Handler* handler, const UnownedStringSlice& slice, StringBuilder& out)
{
    const char quoteChar = handler->getQuoteChar();
    out.appendChar(quoteChar);
    SlangResult res = handler->appendEscaped(slice, out);
    out.appendChar(quoteChar);
    return res;
}

/* static */SlangResult StringEscapeUtil::appendUnquoted(Handler* handler, const UnownedStringSlice& slice, StringBuilder& out)
{
    const Index len = slice.getLength();

    const char quoteChar = handler->getQuoteChar();
    SLANG_UNUSED(quoteChar);

    // Must have quote characters around if
    SLANG_ASSERT(len >= 2 && slice[0] == quoteChar && slice[len - 1] == quoteChar);

    return handler->appendUnescaped(slice.subString(1, len - 2), out);
}

/* static */SlangResult StringEscapeUtil::appendMaybeQuoted(Handler* handler, const UnownedStringSlice& slice, StringBuilder& out)
{
    if (handler->isQuotingNeeded(slice))
    {
        return appendQuoted(handler, slice, out);
    }
    else
    {
        out.append(slice);
        return SLANG_OK;
    }
}

/* static */bool StringEscapeUtil::isQuoted(char quoteChar, UnownedStringSlice& slice)
{
    const Index len = slice.getLength();
    return len >= 2 && slice[0] == quoteChar && slice[len - 1] == quoteChar;
}

/* static */UnownedStringSlice StringEscapeUtil::unquote(char quoteChar, const UnownedStringSlice& slice)
{
    const Index len = slice.getLength();
    if (len >= 2 && slice[0] == quoteChar && slice[len - 1] == quoteChar)
    {
        return UnownedStringSlice(slice.begin() + 1, len - 2);
    }
    SLANG_ASSERT(!"Not quoted!");
    return UnownedStringSlice();
}

/* static */SlangResult StringEscapeUtil::appendMaybeUnquoted(Handler* handler, const UnownedStringSlice& slice, StringBuilder& out)
{
    const char quoteChar = handler->getQuoteChar();

    const Index len = slice.getLength();

    if (len >= 2 && slice[0] == quoteChar && slice[len - 1] == quoteChar)
    {
        return appendUnquoted(handler, slice, out);
    }
    else
    {
        out.append(slice);
        return SLANG_OK;
    }
}

/* static */SlangResult StringEscapeUtil::isUnescapeShellLikeNeeded(Handler* handler, const UnownedStringSlice& slice)
{
    return slice.indexOf(handler->getQuoteChar()) >= 0;
}

/* static */SlangResult StringEscapeUtil::unescapeShellLike(Handler* handler, const UnownedStringSlice& slice, StringBuilder& out)
{
    StringBuilder buf;
    const char quoteChar = handler->getQuoteChar();

    UnownedStringSlice remaining(slice);

    while (remaining.getLength())
    {
        const Index index = remaining.indexOf(quoteChar);

        if (index < 0)
        {
            out.append(remaining);
            return SLANG_OK;
        }

        // Append the bit before
        out.append(remaining.head(index));

        // Okay we need to lex to the end

        const char* quotedEnd = nullptr;
        SLANG_RETURN_ON_FAIL(handler->lexQuoted(remaining.begin() + index, &quotedEnd));

        // Unescape it
        SLANG_RETURN_ON_FAIL(appendUnquoted(handler, UnownedStringSlice(remaining.begin() + index, quotedEnd), out));

        // Fix up remaining
        remaining = UnownedStringSlice(quotedEnd, remaining.end());
    }

    return SLANG_OK;
}

} // namespace Slang
