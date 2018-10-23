#include "slang-string-util.h"

namespace Slang {

/* static */void StringUtil::split(const UnownedStringSlice& in, char splitChar, List<UnownedStringSlice>& slicesOut)
{
    slicesOut.Clear();

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
        slicesOut.Add(UnownedStringSlice(start, cur));

        // Skip the split character, if at end we are okay anyway
        start = cur + 1;
    }
}


/* static */void StringUtil::append(const char* format, va_list args, StringBuilder& buf)
{
    int numChars = 0;

#if SLANG_WINDOWS_FAMILY
    numChars = _vscprintf(format, args);
#else
    {
        va_list argsCopy;
        va_copy(argsCopy, args);
        numChars = vsnprintf(nullptr, 0, format, argsCopy);
        va_end(argsCopy);
    }
#endif

    List<char> chars;
    chars.SetSize(numChars + 1);

#if SLANG_WINDOWS_FAMILY
    vsnprintf_s(chars.Buffer(), numChars + 1, _TRUNCATE, format, args);
#else
    vsnprintf(chars.Buffer(), numChars + 1, format, args);
#endif

    buf.Append(chars.Buffer(), numChars);
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

} // namespace Slang
