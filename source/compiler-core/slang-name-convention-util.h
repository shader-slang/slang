#ifndef SLANG_COMPILER_CORE_NAME_CONVENTION_UTIL_H
#define SLANG_COMPILER_CORE_NAME_CONVENTION_UTIL_H

#include "../core/slang-string.h"
#include "../core/slang-list.h"

namespace Slang
{

enum class NameStyle : uint8_t
{
    Unknown,    /// Unknown style
    Kabab,      /// Words are separated with -. WORDS-ARE-SEPARATED, words-are-separted
    Snake,      /// Words are separated with _. WORDS_ARE_SEPARATED, words_are_separated
    Camel,      /// Words start with a capital. (Upper will make first words character capitalized, aka PascalCase)
};

enum class CharCase : uint8_t
{
    Upper,
    Lower,
};

struct NameConvention
{
    static NameConvention make(CharCase inCharCase, NameStyle inStyle) { return NameConvention{inStyle, inCharCase}; }
    static NameConvention makeLower(NameStyle inStyle) { return NameConvention{ inStyle, CharCase::Lower }; }
    static NameConvention makeUpper(NameStyle inStyle) { return NameConvention{ inStyle, CharCase::Upper }; }
    static NameConvention makeInvalid() { return NameConvention{NameStyle::Unknown, CharCase::Lower}; }

    bool isValid() const { return style != NameStyle::Unknown; }

    NameStyle style;
    CharCase charCase;
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
        /// Doesn't exhaustively test the string slice, or determine invalid scenarios
        /// Use 'getConvention' to get error checking
    static NameStyle getStyle(const UnownedStringSlice& slice);
        /// Gets the naming convention based on the slice.
        /// Will return invalid convention if cannot be determined.
    static NameConvention getConvention(const UnownedStringSlice& slice);

        /// Given a slice and a naming convention, split into it's constituent parts. If convention isn't specified, will infer from slice using getConvention.
    static void split(NameStyle nameStyle, const UnownedStringSlice& slice, List<UnownedStringSlice>& out);
    static void split(const UnownedStringSlice& slice, List<UnownedStringSlice>& out);

        /// Given slices, join together with the specified convention into out
    static void join(const UnownedStringSlice* slices, Index slicesCount, NameConvention convention, StringBuilder& out);

        /// Join with a join char, and potentially changing case of input slices
    static void join(const UnownedStringSlice* slices, Index slicesCount, CharCase charCase, char joinChar, StringBuilder& out);

        /// Convert from one convention to another. If fromConvention isn't specified, will infer from slice using getConvention.
    static void convert(NameStyle fromStyle, const UnownedStringSlice& slice, NameConvention toConvention, StringBuilder& out);
    static void convert(const UnownedStringSlice& slice, NameConvention toConvention, StringBuilder& out);
};

}

#endif // SLANG_COMPILER_CORE_NAME_CONVENTION_UTIL_H
