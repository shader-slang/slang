#include "slang-string-escape-util.h"

#include "slang-char-util.h"
#include "slang-text-io.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!! StringSpaceEscapeUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


/* static */bool StringSpaceEscapeUtil::isQuotingNeeded(const UnownedStringSlice& slice)
{
    return slice.indexOf(' ') >= 0;
}

/* static */SlangResult StringSpaceEscapeUtil::appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    if (slice.indexOf('"') >= 0)
    {
        return SLANG_FAIL;
    }

    out.append(slice);
    return SLANG_OK;
}

/* static */SlangResult StringSpaceEscapeUtil::appendEscaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    if (slice.indexOf('"') >= 0)
    {
        return SLANG_FAIL;
    }
    out.append(slice);
    return SLANG_OK;
}

void StringSpaceEscapeUtil::appendQuoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    out.appendChar('"');
    appendEscaped(slice, out);
    out.appendChar('"');
}

/* static */void StringSpaceEscapeUtil::appendMaybeQuoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    if (isQuotingNeeded(slice))
    {
        appendQuoted(slice, out);
    }
    else
    {
        out.append(slice);
    }
}

/* static */SlangResult StringSpaceEscapeUtil::lexQuoted(const char* cursor, char quoteChar, const char** outCursor)
{
    *outCursor = cursor;

    if (*cursor != quoteChar)
    {
        return SLANG_FAIL;
    }
    cursor++;

    for (;;)
    {
        const char c = *cursor;
        if (c == quoteChar)
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

/* static */SlangResult StringSpaceEscapeUtil::appendMaybeUnquoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    const Index len = slice.getLength();

    if (len >= 2 && slice[0] == '"' && slice[len - 1] == '"')
    {
        return appendUnquoted(slice, out);
    }
    else
    {
        out.append(slice);
        return SLANG_OK;
    }
}

/* static */SlangResult StringSpaceEscapeUtil::appendUnquoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    const Index len = slice.getLength();

    // Must have quote characters around if
    SLANG_ASSERT(len >= 2 && slice[0] == '"' && slice[len - 1] == '"');

    return appendUnescaped(slice.subString(1, len - 2), out);
}

// !!!!!!!!!!!!!!!!!!!!!!!!!! StringCppEscapeUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

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

/* static */bool StringCppEscapeUtil::isQuotingNeeded(const UnownedStringSlice& slice)
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

/* static */void StringCppEscapeUtil::appendQuoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    out.appendChar('"');
    appendEscaped(slice, out);
    out.appendChar('"');
}

/* static */SlangResult StringCppEscapeUtil::appendUnquoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    const Index len = slice.getLength();

    // Must have quote characters around if
    SLANG_ASSERT(len >= 2 && slice[0] == '"' && slice[len - 1] == '"');

    return appendUnescaped(slice.subString(1, len - 2), out);
}

/* static */void StringCppEscapeUtil::appendMaybeQuoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    if (isQuotingNeeded(slice))
    {
        appendQuoted(slice, out);
    }
    else
    {
        out.append(slice);
    }
}

/* static */SlangResult StringCppEscapeUtil::appendMaybeUnquoted(const UnownedStringSlice& slice, StringBuilder& out)
{
    const Index len = slice.getLength();

    if (len >= 2 && slice[0] == '"' && slice[len - 1] == '"')
    {
        return appendUnquoted(slice, out);
    }
    else
    {
        out.append(slice);
        return SLANG_OK;
    }
}

/* static */void StringCppEscapeUtil::appendEscaped(const UnownedStringSlice& slice, StringBuilder& out)
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
}

/* static */SlangResult StringCppEscapeUtil::appendUnescaped(const UnownedStringSlice& slice, StringBuilder& out)
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

/* static */SlangResult StringCppEscapeUtil::lexQuoted(const char* cursor, char quote, const char** outCursor)
{
    *outCursor = cursor;

    if (*cursor != quote)
    {
        return SLANG_FAIL;
    }
    cursor++;

    for (;;)
    {
        const char c = *cursor;
        if (c == quote)
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

} // namespace Slang
