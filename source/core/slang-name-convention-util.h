#ifndef SLANG_CORE_NAME_CONVENTION_UTIL_H
#define SLANG_CORE_NAME_CONVENTION_UTIL_H

#include "slang-string.h"
#include "slang-list.h"

namespace Slang
{


enum class NameConvention
{
    UpperKababCase,     /// Words are separated with -. WORDS-ARE-SEPARATED
    LowerKababCase,     /// Words are separated with -. words-are-separated
    UpperSnakeCase,     /// Words are separated with _. WORDS_ARE_SEPARATED
    LowerSnakeCase,     /// Words are separated with _. words_are_separated
    UpperCamelCase,     /// Words start with a capital, first word starts with upper. WordsStartWithACapital (aka PascalCase)
    LowerCamelCase,     /// Words start with a capital, first word starts with lower. wordsStartWithACapital (aka camelCase)
};

struct NameConventionUtil
{
    enum class CharCase
    {
        None,
        Upper,
        Lower,
    };

        /// Given a slice and a naming convention, split into it's constituent parts. 
    static void split(NameConvention convention, const UnownedStringSlice& slice, List<UnownedStringSlice>& out);

        /// Given slices, join together with the specified convention into out
    static void join(const UnownedStringSlice* slices, Index slicesCount, NameConvention convention, StringBuilder& out);

        /// Join with a join char, and potentially changing case of input slices
    static void join(const UnownedStringSlice* slices, Index slicesCount, CharCase charCase, char joinChar, StringBuilder& out);

        /// Convert from one convention to another
    static void convert(NameConvention fromConvention, const UnownedStringSlice& slice, NameConvention toConvention, StringBuilder& out);
};

}

#endif // SLANG_CORE_NAME_CONVENTION_UTIL_H
