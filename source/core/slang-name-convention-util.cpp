
#include "slang-name-convention-util.h"

#include "slang-char-util.h"
#include "slang-string-util.h"

namespace Slang
{

/* static */void NameConventionUtil::split(NameConvention convention, const UnownedStringSlice& slice, List<UnownedStringSlice>& out)
{
    switch (convention)
    {
        case NameConvention::Kabab:
        {
            StringUtil::split(slice, '-', out);
            break;
        }
        case NameConvention::Snake:
        {
            StringUtil::split(slice, '_', out);
            break;
        }
        case NameConvention::Camel:
        {
            typedef CharUtil::Flags CharFlags;
            typedef CharUtil::Flag CharFlag;

            CharFlags prevFlags = 0;
            const char*const end = slice.end();

            const char* start = slice.begin();
            for (const char* cur = start; cur < end; ++cur)
            {
                char c = *cur;
                const CharUtil::Flags flags = CharUtil::getFlags(c);

                if (flags & CharFlag::Upper)
                {
                    if (prevFlags & CharFlag::Lower)
                    {
                        // If we go from lower to upper, we have a transition
                        out.add(UnownedStringSlice(start, cur - 1));
                        start = cur;
                    }
                    else if ((prevFlags & CharFlag::Upper) && cur + 1 < end)
                    {
                        // TODO(JS): This doesn't catch situations where there is a single letter like
                        // ABox, because of this rule will be split to AB, ox
                        // If we have acronyms not being capitalized this would not be an issue

                        // Could be an acronym, if the next character is lower
                        if (CharUtil::isLower(cur[1]))
                        {
                            out.add(UnownedStringSlice(start, cur));
                            start = cur;
                        }
                    }
                }
                
                prevFlags = flags;
            }

            // Add any end section
            if (start < end)
            {
                out.add(UnownedStringSlice(start, end));
            }
            break;
        }
    }

}

/* static */void NameConventionUtil::join(const UnownedStringSlice* slices, Index slicesCount, CharCase charCase, char joinChar, StringBuilder& out)
{
    if (slicesCount <= 0)
    {
        return;
    }

    Index totalSize = slicesCount + 1;
    for (Index i = 0; i < slicesCount; ++i)
    {
        totalSize = slices[i].getLength();
    }

    char*const dstStart = out.prepareForAppend(totalSize);
    char* dst = dstStart;

    for (Index i = 0; i < slicesCount; ++i)
    {
        const UnownedStringSlice& slice = slices[i];
        const Index count = slice.getLength();
        const char*const src = slice.begin();

        if (i > 0)
        {
            *dst++ = joinChar;
        }

        switch (charCase)
        {
            case CharCase::Upper:
            {
                for (Index j = 0; j < count; ++j)
                {
                    dst[j] = CharUtil::toUpper(src[j]);
                }
                break;
            }
            case CharCase::Lower:
            {
                for (Index j = 0; j < count; ++j)
                {
                    dst[j] = CharUtil::toLower(src[j]);
                }
                break;
            }
            default:
            case CharCase::None:
            {
                for (Index j = 0; j < count; ++j)
                {
                    dst[j] = src[j];
                }
                break;
            }
        }

        dst += count;
    }

    SLANG_ASSERT(dstStart + totalSize == dst);
    out.appendInPlace(dstStart, totalSize);
}

/* static */void NameConventionUtil::join(const UnownedStringSlice* slices, Index slicesCount, CharCase charCase, NameConvention convention, StringBuilder& out)
{
    switch (convention)
    {
        case NameConvention::Kabab:        return join(slices, slicesCount, charCase, '-', out);
        case NameConvention::Snake:        return join(slices, slicesCount, charCase, '_', out);
        case NameConvention::Camel:
        {
            Index totalSize = 0;

            for (Index i = 0; i < slicesCount; ++i)
            {
                totalSize += slices[i].getLength();
            }

            char*const dstStart = out.prepareForAppend(totalSize);
            char* dst = dstStart;

            for (Index i = 0; i < slicesCount; ++i)
            {
                const UnownedStringSlice& slice = slices[i];
                Index count = slice.getLength();
                const char* src = slice.begin();

                Int j = 0;

                if (count > 0 && !(i == 0 && charCase == CharCase::Lower))
                {
                    // Capitalize first letter of each word, unless on first word and 'lower'
                    dst[j] = CharUtil::toUpper(src[j]);
                    j++;
                }

                for (; j < count; ++j)
                {
                    dst[j] = CharUtil::toLower(src[j]);
                }

                dst += count;
            }
            break;
        }
    }
}

/* static */void NameConventionUtil::convert(NameConvention fromConvention, const UnownedStringSlice& slice, CharCase charCase, NameConvention toConvention, StringBuilder& out)
{
    // Split into slices
    List<UnownedStringSlice> slices;
    split(fromConvention, slice, slices);

    // Join the slices in the toConvention
    join(slices.getBuffer(), slices.getCount(), charCase, toConvention, out);
}

}

