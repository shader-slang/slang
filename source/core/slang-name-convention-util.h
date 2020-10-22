#ifndef SLANG_CORE_NAME_CONVENTION_UTIL_H
#define SLANG_CORE_NAME_CONVENTION_UTIL_H

#include "slang-string.h"
#include "slang-list.h"

namespace Slang
{

enum class NameConvention
{
    Kabab,     /// Words are separated with -. WORDS-ARE-SEPARATED
    Snake,     /// Words are separated with _. WORDS_ARE_SEPARATED
    Camel,     /// Words start with a capital. (Upper will make first words character capitalized, aka PascalCase)
};

enum class CharCase
{
    Upper,
    Lower,
};

/* This utility is to enable easy conversion and interpretation of names that use standard conventions, typically in programming
languages. The conventions are largely how to represent multiple words together.

Split is used to split up a name into it's constituent 'words' based on a convention.
Join is used to combine words based on a convention/character case

Convert uses split and join to allow easy conversion between conventions. 
*/
struct NameConventionUtil
{
        /// Given a slice tries to determine the convention used.
        /// If no separators are found, will assume Camel
    static NameConvention getConvention(const UnownedStringSlice& slice);

        /// Given a slice and a naming convention, split into it's constituent parts. If convention isn't specified, will infer from slice using getConvention.
    static void split(NameConvention convention, const UnownedStringSlice& slice, List<UnownedStringSlice>& out);
    static void split(const UnownedStringSlice& slice, List<UnownedStringSlice>& out);

        /// Given slices, join together with the specified convention into out
    static void join(const UnownedStringSlice* slices, Index slicesCount, CharCase charCase, NameConvention convention, StringBuilder& out);

        /// Join with a join char, and potentially changing case of input slices
    static void join(const UnownedStringSlice* slices, Index slicesCount, CharCase charCase, char joinChar, StringBuilder& out);

        /// Convert from one convention to another. If fromConvention isn't specified, will infer from slice using getConvention.
    static void convert(NameConvention fromConvention, const UnownedStringSlice& slice, CharCase charCase, NameConvention toConvention, StringBuilder& out);
    static void convert(const UnownedStringSlice& slice, CharCase charCase, NameConvention toConvention, StringBuilder& out);
};

}

#endif // SLANG_CORE_NAME_CONVENTION_UTIL_H
