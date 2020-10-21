
#include "slang-name-convention-util.h"

#include "slang-char-util.h"
#include "slang-string-util.h"

namespace Slang
{

/* static */void NameConventionUtil::camelCaseToLowerDashed(const UnownedStringSlice& in, StringBuilder& out)
{
    typedef CharUtil::Flags CharFlags;
    typedef CharUtil::Flag CharFlag;

    CharFlags prevFlags = 0;
    const char*const end = in.end();
    for (const char* cur = in.begin(); cur < end; ++cur)
    {
        char c = *cur;
        const CharUtil::Flags flags = CharUtil::getFlags(c);

        if (flags & CharFlag::Upper)
        {
            if (prevFlags & CharFlag::Lower)
            {
                // If we go from lower to upper, insert a dash. aA -> a-a
                out << '-';
            }
            else if ((prevFlags & CharFlag::Upper) && cur + 1 < end)
            {
                // Could be an acronym, if the next character is lower, we need to insert a - here
                if (CharUtil::isLower(cur[1]))
                {
                    out << '-';
                }
            }
            // Make it lower
            c = c - 'A' + 'a';
        }
        out << c;

        prevFlags = flags;
    }
}


/* static */void NameConventionUtil::snakeCaseToLowerDashed(const UnownedStringSlice& inSlice, StringBuilder& out)
{
    Index count = inSlice.getLength();
    const char* const src = inSlice.begin();

    char* dst = out.prepareForAppend(count);

    for (Index i = 0; i < count; ++i)
    {
        const char c = src[i];
        dst[i] = (c == '_') ? '-' : CharUtil::toLower(c);
    }

    out.appendInPlace(dst, count);
}


/* static */void NameConventionUtil::snakeCaseToUpperCamel(const UnownedStringSlice& in, StringBuilder& out)
{
    const char*const end = in.end();

    bool capitalizeNextAlpha = true;

    for (const char* cur = in.begin(); cur < end; ++cur)
    {
        char c = *cur;

        if (c == '_')
        {
            capitalizeNextAlpha = true;
        }
        else
        {
            if (CharUtil::isAlpha(c))
            {
                if (capitalizeNextAlpha)
                {
                    c = CharUtil::toUpper(c);
                }
                else
                {
                    c = CharUtil::toLower(c);
                }
            }
            // First character after _, should be capitalized. If the character isn't alpha we just ignore capitalization
            capitalizeNextAlpha = false;

            out.appendChar(c);
        }
    }
}

/* static */void NameConventionUtil::dashedToUpperSnake(const UnownedStringSlice& in, StringBuilder& out)
{
    Index count = in.getLength();
    const char* const src = in.begin();

    char* dst = out.prepareForAppend(count);

    for (Index i = 0; i < count; ++i)
    {
        const char c = src[i];
        dst[i] = (c == '-') ? '_' : CharUtil::toUpper(c);
    }

    out.appendInPlace(dst, count);
}


}

