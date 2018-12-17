#include "slang-string-util.h"

namespace Slang {

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringBlob !!!!!!!!!!!!!!!!!!!!!!!!!!!

// Allocate static const storage for the various interface IDs that the Slang API needs to expose
static const Guid IID_ISlangUnknown = SLANG_UUID_ISlangUnknown;
static const Guid IID_ISlangBlob = SLANG_UUID_ISlangBlob;

/* static */ISlangUnknown* StringBlob::getInterface(const Guid& guid)
{
    return (guid == IID_ISlangUnknown || guid == IID_ISlangBlob) ? static_cast<ISlangBlob*>(this) : nullptr;
}

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! StringUtil !!!!!!!!!!!!!!!!!!!!!!!!!!!

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
    // Calculate the size 
    size_t numChars;
    {
        // Create a copy of args, as will be consumed by calcFormattedSize
        va_list argsCopy;
        va_copy(argsCopy, args);
        numChars = calcFormattedSize(format, argsCopy);
        va_end(argsCopy);
    }

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

} // namespace Slang
