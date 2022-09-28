
#include "slang-name-convention-util.h"

#include "../core/slang-char-util.h"
#include "../core/slang-string-util.h"

namespace Slang
{

/* static */NameConvention NameConventionUtil::getConvention(const UnownedStringSlice& slice)
{
    // If no chars, or first char isn't alpha we don't know what it is
    if (slice.getLength() <= 0 || !CharUtil::isAlpha(slice[0]))
    {
        return NameConvention::makeInvalid();
    }

    typedef int Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            Underscore  = 0x1,
            Dash        = 0x2,
            Upper       = 0x4,
            Lower       = 0x8,
        };
    };

    Flags flags = 0;
    
    for (const char c : slice)
    {
        switch (c)
        {
            case '-':   flags |= Flag::Dash; break;
            case '_':   flags |= Flag::Underscore; break;
            default:
            {
                if (CharUtil::isLower(c))
                {
                    flags |= Flag::Lower;
                }
                else if (CharUtil::isUpper(c))
                {
                    flags |= Flag::Upper;
                }
                else if (CharUtil::isDigit(c))
                {
                    // Is okay as long as not first char (which we already tested is alpha)
                }
                else
                {
                    // Don't know what this style is
                    return NameConvention::makeInvalid();
                }
            }
        }
    }

    // Use flags to determine what convention is used

    switch (flags)
    {
        // We'll assume it's lower camel. 
        case Flag::Lower:               return NameConvention::makeLower(NameStyle::Camel);
        // We'll assume it's upper snake. It almost certainly isn't camel, and snake is more usual
        // than kabab.
        case Flag::Upper:               return NameConvention::makeUpper(NameStyle::Snake);
        case Flag::Upper | Flag::Lower:
        {
            // Looks like camel, choose the right case based on first char
            if (CharUtil::isUpper(slice[0]))
            {
                return NameConvention::makeUpper(NameStyle::Camel);
            }
            else
            {
                return NameConvention::makeLower(NameStyle::Camel);
            }
        }
        case Flag::Lower | Flag::Dash:          return NameConvention::makeLower(NameStyle::Kabab);
        case Flag::Upper | Flag::Dash:          return NameConvention::makeUpper(NameStyle::Kabab);
        case Flag::Lower | Flag::Underscore:    return NameConvention::makeLower(NameStyle::Snake);
        case Flag::Upper | Flag::Underscore:    return NameConvention::makeUpper(NameStyle::Snake);
        default: break;
    }

    // Don't know what this style is
    return NameConvention::makeInvalid();
}


/* static */NameStyle NameConventionUtil::getStyle(const UnownedStringSlice& slice)
{
    for (const char c : slice)
    {
        switch (c)
        {
            case '-':   return NameStyle::Kabab;
            case '_':   return NameStyle::Snake;
            default: break;
        }
    }
    return NameStyle::Camel;
}

/* static */void NameConventionUtil::split(NameStyle style, const UnownedStringSlice& slice, List<UnownedStringSlice>& out)
{
    switch (style)
    {
        case NameStyle::Kabab:
        {
            StringUtil::split(slice, '-', out);
            break;
        }
        case NameStyle::Snake:
        {
            StringUtil::split(slice, '_', out);
            break;
        }
        case NameStyle::Camel:
        {
            typedef CharUtil::Flags CharFlags;
            typedef CharUtil::Flag CharFlag;

            CharFlags prevFlags = 0;
            const char*const end = slice.end();

            const char* start = slice.begin();
            for (const char* cur = start; cur < end; ++cur)
            {
                const char c = *cur;
                const CharUtil::Flags flags = CharUtil::getFlags(c);

                if (flags & CharFlag::Upper)
                {
                    if (prevFlags & CharFlag::Lower)
                    {
                        // If we go from lower to upper, we have a transition
                        out.add(UnownedStringSlice(start, cur));
                        start = cur;
                    }
                    else if ((prevFlags & CharFlag::Upper) && cur + 1 < end)
                    {
                        // This works with capital or uncapitalized acronyms, but if we have two capitalized acronyms following each other - it can't split.
                        // 
                        // For example 
                        // "IAABBSystem" -> "IAABB", "System"
                        // 
                        // If it only accepted lower case acronyms the logic could be changed such that the following could be produced
                        // "IAabbSystem" -> "I", "Aabb", "System" 
                        //
                        // Since Slang source largely goes with upper case acronyms, we work with the heuristic here..

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
        case NameStyle::Unknown:
        {
            out.add(slice);
            break;
        }
    }
}

void NameConventionUtil::split(const UnownedStringSlice& slice, List<UnownedStringSlice>& out)
{
    split(getStyle(slice), slice, out);
}

/* static */void NameConventionUtil::join(const UnownedStringSlice* slices, Index slicesCount, CharCase charCase, char joinChar, StringBuilder& out)
{
    if (slicesCount <= 0)
    {
        return;
    }

    Index totalSize = slicesCount - 1;
    for (Index i = 0; i < slicesCount; ++i)
    {
        totalSize += slices[i].getLength();
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
        }

        dst += count;
    }

    SLANG_ASSERT(dstStart + totalSize == dst);
    out.appendInPlace(dstStart, totalSize);
}

/* static */void NameConventionUtil::join(const UnownedStringSlice* slices, Index slicesCount, NameConvention convention, StringBuilder& out)
{
    switch (convention.style)
    {
        case NameStyle::Kabab:        return join(slices, slicesCount, convention.charCase, '-', out);
        case NameStyle::Snake:        return join(slices, slicesCount, convention.charCase, '_', out);
        case NameStyle::Camel:
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

                if (count > 0 && !(i == 0 && convention.charCase == CharCase::Lower))
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

            SLANG_ASSERT(dstStart + totalSize == dst);
            out.appendInPlace(dstStart, totalSize);
            break;
        }
    }
}

/* static */void NameConventionUtil::convert(NameStyle fromStyle, const UnownedStringSlice& slice, NameConvention toConvention, StringBuilder& out)
{
    // Split into slices
    List<UnownedStringSlice> slices;
    split(fromStyle, slice, slices);

    // Join the slices in the toConvention
    join(slices.getBuffer(), slices.getCount(), toConvention, out);
}

/* static */void NameConventionUtil::convert(const UnownedStringSlice& slice, NameConvention toConvention, StringBuilder& out)
{
    convert(getStyle(slice), slice, toConvention, out);
}

}

