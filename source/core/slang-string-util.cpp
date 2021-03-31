#include "slang-string-util.h"

#include "slang-blob.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!

/* static */bool StringUtil::areAllEqual(const List<UnownedStringSlice>& a, const List<UnownedStringSlice>& b, EqualFn equalFn)
{
    if (a.getCount() != b.getCount())
    {
        return false;
    }

    const Index count = a.getCount();
    for (Index i = 0; i < count; ++i)
    {
        if (!equalFn(a[i], b[i]))
        {
            return false;
        }
    }
    return true;
}

/* static */bool StringUtil::areAllEqualWithSplit(const UnownedStringSlice& a, const UnownedStringSlice& b, char splitChar, EqualFn equalFn)
{
    List<UnownedStringSlice> slicesA, slicesB;
    StringUtil::split(a, splitChar, slicesA);
    StringUtil::split(b, splitChar, slicesB);
    return areAllEqual(slicesA, slicesB, equalFn);
}

/* static */void StringUtil::split(const UnownedStringSlice& in, char splitChar, List<UnownedStringSlice>& outSlices)
{
    outSlices.clear();

    const char* start = in.begin();
    const char* end = in.end();

    while (start < end)
    {
        // Move cur so it's either at the end or at next split character
        const char* cur = start;
        while (cur < end && *cur != splitChar)
        {
            cur++;
        }

        // Add to output
        outSlices.add(UnownedStringSlice(start, cur));

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }
}

/* static */Index StringUtil::split(const UnownedStringSlice& in, char splitChar, Index maxSlices, UnownedStringSlice* outSlices)
{
    Index index = 0;

    const char* start = in.begin();
    const char* end = in.end();

    while (start < end && index < maxSlices)
    {
        // Move cur so it's either at the end or at next split character
        const char* cur = start;
        while (cur < end && *cur != splitChar)
        {
            cur++;
        }

        // Add to output
        outSlices[index++] = UnownedStringSlice(start, cur);

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }

    return index;
}

/* static */SlangResult StringUtil::split(const UnownedStringSlice& in, char splitChar, Index maxSlices, UnownedStringSlice* outSlices, Index& outSlicesCount)
{
    const Index sliceCount = split(in, splitChar, maxSlices, outSlices);
    if (sliceCount == maxSlices && sliceCount > 0)
    {
        // To succeed must have parsed all of the input
        if (in.end() != outSlices[sliceCount - 1].end())
        {
            return SLANG_FAIL;
        }
    }
    outSlicesCount = sliceCount;
    return SLANG_OK;
}

/* static */void StringUtil::join(const List<String>& values, char separator, StringBuilder& out)
{
    join(values, UnownedStringSlice(&separator, 1), out);
}

/* static */void StringUtil::join(const List<String>& values, const UnownedStringSlice& separator, StringBuilder& out)
{
    const Index count = values.getCount();
    if (count <= 0)
    {
        return;
    }
    out.append(values[0]);
    for (Index i = 1; i < count; i++)
    {
        out.append(separator);
        out.append(values[i]);
    }
}

/* static */void StringUtil::join(const UnownedStringSlice* values, Index valueCount, char separator, StringBuilder& out)
{
    join(values, valueCount, UnownedStringSlice(&separator, 1), out);
}

/* static */void StringUtil::join(const UnownedStringSlice* values, Index valueCount, const UnownedStringSlice& separator, StringBuilder& out)
{
    if (valueCount <= 0)
    {
        return;
    }
    out.append(values[0]);
    for (Index i = 1; i < valueCount; i++)
    {
        out.append(separator);
        out.append(values[i]);
    }
}

/* static */Index StringUtil::indexOfInSplit(const UnownedStringSlice& in, char splitChar, const UnownedStringSlice& find)
{
    const char* start = in.begin();
    const char* end = in.end();

    for (Index i = 0; start < end; ++i)
    {
        // Move cur so it's either at the end or at next split character
        const char* cur = start;
        while (cur < end && *cur != splitChar)
        {
            cur++;
        }

        // See if we have a match
        if (UnownedStringSlice(start, cur) == find)
        {
            return i;
        }

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }
    return -1;
}

UnownedStringSlice StringUtil::getAtInSplit(const UnownedStringSlice& in, char splitChar, Index index)
{
    const char* start = in.begin();
    const char* end = in.end();

    for (Index i = 0; start < end; ++i)
    {
        // Move cur so it's either at the end or at next split character
        const char* cur = start;
        while (cur < end && *cur != splitChar)
        {
            cur++;
        }

        if (i == index)
        {
            return UnownedStringSlice(start, cur);
        }

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }

    return UnownedStringSlice();
}

/* static */size_t StringUtil::calcFormattedSize(const char* format, va_list args)
{
#if SLANG_WINDOWS_FAMILY
    return _vscprintf(format, args);
#else
     return vsnprintf(nullptr, 0, format, args);
#endif
}

/* static */void StringUtil::calcFormatted(const char* format, va_list args, size_t numChars, char* dst)
{
#if SLANG_WINDOWS_FAMILY
    vsnprintf_s(dst, numChars + 1, _TRUNCATE, format, args);
#else
    vsnprintf(dst, numChars + 1, format, args);
#endif
}

/* static */void StringUtil::append(const char* format, va_list args, StringBuilder& buf)
{
    // Calculate the size required (not including terminating 0)
    size_t numChars;
    {
        // Create a copy of args, as will be consumed by calcFormattedSize
        va_list argsCopy;
        va_copy(argsCopy, args);
        numChars = calcFormattedSize(format, argsCopy);
        va_end(argsCopy);
    }

    // Requires + 1 , because calcFormatted appends a terminating 0
    char* dst = buf.prepareForAppend(numChars + 1);
    calcFormatted(format, args, numChars, dst);
    buf.appendInPlace(dst, numChars);
}

/* static */void StringUtil::appendFormat(StringBuilder& buf, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    append(format, args, buf);
    va_end(args);
}

/* static */String StringUtil::makeStringWithFormat(const char* format, ...)
{
    StringBuilder builder;

    va_list args;
    va_start(args, format);
    append(format, args, builder);
    va_end(args);

    return builder;
}

/* static */String StringUtil::getString(ISlangBlob* blob)
{
    if (blob)
    {
        size_t size = blob->getBufferSize();
        if (size > 0)
        {
            const char* contents = (const char*)blob->getBufferPointer();
            // Check it has terminating 0, if not we must construct as if it does
            if (contents[size - 1] == 0)
            {
                size--;
            }
            return String(contents, contents + size);
        }
    }
    return String();
}

ComPtr<ISlangBlob> StringUtil::createStringBlob(const String& string)
{
    return ComPtr<ISlangBlob>(new StringBlob(string));
}

/* static */String StringUtil::calcCharReplaced(const UnownedStringSlice& slice, char fromChar, char toChar)
{
    if (fromChar == toChar)
    {
        return slice;
    }

    const Index numChars = slice.getLength();
    const char* srcChars = slice.begin();

    StringBuilder builder;
    char* dstChars = builder.prepareForAppend(numChars);

    for (Index i = 0; i < numChars; ++i)
    {
        char c = srcChars[i];
        dstChars[i] = (c == fromChar) ? toChar : c;
    }

    builder.appendInPlace(dstChars, numChars);
    return builder;
}

/* static */String StringUtil::calcCharReplaced(const String& string, char fromChar, char toChar)
{
    return (fromChar == toChar || string.indexOf(fromChar) == Index(-1)) ? string : calcCharReplaced(string.getUnownedSlice(), fromChar, toChar);
}

/* static */bool StringUtil::extractLine(UnownedStringSlice& ioText, UnownedStringSlice& outLine)
{
    char const*const begin = ioText.begin();
    char const*const end = ioText.end();

    // If we have hit the end then return the 'special' terminator
    if (begin == nullptr)
    {
        outLine = UnownedStringSlice(nullptr, nullptr);
        return false;
    }

    char const* cursor = begin;
    while (cursor < end)
    {
        int c = *cursor++;
        switch (c)
        {
            case '\r': case '\n':
            {
                // Remember the end of the line
                const char*const lineEnd = cursor - 1;

                // When we see a line-break character we need
                // to record the line break, but we also need
                // to deal with the annoying issue of encodings,
                // where a multi-byte sequence might encode
                // the line break.
                if (cursor < end)
                {
                    int d = *cursor;
                    if ((c ^ d) == ('\r' ^ '\n'))
                        cursor++;
                }

                ioText = UnownedStringSlice(cursor, end);
                outLine = UnownedStringSlice(begin, lineEnd);
                return true;
            }
            default:
                break;
        }
    }

    // There is nothing remaining
    ioText = UnownedStringSlice(nullptr, nullptr);

    // Could be empty, or the remaining line (without line end terminators of)
    SLANG_ASSERT(begin <= cursor);

    outLine = UnownedStringSlice(begin, cursor);
    return true;
}

/* static */void StringUtil::calcLines(const UnownedStringSlice& textIn, List<UnownedStringSlice>& outLines)
{
    outLines.clear();
    UnownedStringSlice text(textIn), line;
    while (extractLine(text, line))
    {
        outLines.add(line);
    }
}

/* static */UnownedStringSlice StringUtil::trimEndOfLine(const UnownedStringSlice& line)
{
    // Strip CR/LF from end of line if present

    const char* begin = line.begin();
    const char* end = line.end();

    if (end > begin)
    {
        const char c = end[-1];
        // If last char is CR/LF move back a char
        if (c == '\n' || c == '\r')
        {
            --end;
            // If next char is a match for the CR/LF pair move back an extra char.
            end -= Index((end > begin) && (c ^ end[-1]) == ('\r' ^ '\n'));
        }
    }

    return line.head(Index(end - begin));
}

/* static */bool StringUtil::areLinesEqual(const UnownedStringSlice& inA, const UnownedStringSlice& inB)
{
    UnownedStringSlice a(inA), b(inB), lineA, lineB;
    
    while (true)
    {
        const auto hasLineA = extractLine(a, lineA);
        const auto hasLineB = extractLine(b, lineB);

        if (!(hasLineA && hasLineB))
        {
            return hasLineA == hasLineB;
        }

        // The lines must be equal
        if (lineA != lineB)
        {
            return false;
        }
    }
}

SLANG_FORCE_INLINE static bool _isDigit(char c)
{
    return (c >= '0' && c <= '9');
}

/* static */SlangResult StringUtil::parseInt(const UnownedStringSlice& in, Int& outValue)
{
    const char* cur = in.begin();
    const char* end = in.end();

    bool negate = false;
    if (cur < end && *cur == '-')
    {
        negate = true;
        cur++;
    }

    // We need at least one digit
    if (cur >= end || !_isDigit(*cur))
    {
        return SLANG_FAIL;
    }
    
    Int value = *cur++ - '0';
    // Do the remaining digits
    for (; cur < end; ++cur)
    {
        const char c = *cur;
        if (!_isDigit(c))
        {
            return SLANG_FAIL;
        }
        value = value * 10 + (c - '0');
    }

    value = negate ? -value : value;

    outValue = value;
    return SLANG_OK;
}

static char _getHexChar(int v)
{
    return (v <= 9) ? char(v + '0') : char(v - 10 + 'A');
}

static char _getEscapedChar(char c)
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

/* static */void StringUtil::appendEscaped(const UnownedStringSlice& slice, StringBuilder& out)
{
    const char* start = slice.begin();
    const char* cur = start;
    const char*const end = slice.end();

    for (; cur < end; ++cur)
    {
        const char c = *cur;
        const char escapedChar = _getEscapedChar(c);

        if (escapedChar)
        {
            // Flush
            if (start < cur)
            {
                out.append(start, end);
            }
            out.appendChar('\\');
            out.appendChar(escapedChar);

            start = cur + 1;
        }
        else if ( c < ' ' || c > 126)
        {
            // Flush
            if (start < cur)
            {
                out.append(start, end);
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


} // namespace Slang
