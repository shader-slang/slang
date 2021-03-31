#ifndef SLANG_CORE_CHAR_UTIL_H
#define SLANG_CORE_CHAR_UTIL_H

#include "slang-string.h"

namespace Slang {

struct CharUtil
{
    typedef uint8_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            Upper                       = 0x01,         ///< A-Z
            Lower                       = 0x02,         ///< a-z
            Digit                       = 0x04,         ///< 0-9
            HorizontalWhitespace        = 0x08,         ///< Whitespace that can appear horizontally (ie excluding CR/LF)
            HexDigit                    = 0x10,         ///< 0-9, a-f, A-F
            VerticalWhitespace          = 0x20,         ///< \n \r
        };
    };

    SLANG_FORCE_INLINE static bool isDigit(char c) { return c >= '0' && c <= '9'; }
    SLANG_FORCE_INLINE static bool isLower(char c) { return c >= 'a' && c <= 'z'; }
    SLANG_FORCE_INLINE static bool isUpper(char c) { return c >= 'A' && c <= 'Z'; }
    SLANG_FORCE_INLINE static bool isHorizontalWhitespace(char c) { return c == ' ' || c == '\t'; }
    SLANG_FORCE_INLINE static bool isVerticalWhitespace(char c) { return c == '\n' || c == '\r'; }
    SLANG_FORCE_INLINE static bool isWhitespace(char c) { return (getFlags(c) & (Flag::HorizontalWhitespace | Flag::VerticalWhitespace)) != 0; }

        /// True if it's alpha
    SLANG_FORCE_INLINE static bool isAlpha(char c) { return (getFlags(c) & (Flag::Upper | Flag::Lower)) != 0; }

    SLANG_FORCE_INLINE static bool isHexDigit(char c) { return (getFlags(c) & Flag::HexDigit) != 0; }

        /// For a given character get the associated flags
    SLANG_FORCE_INLINE static Flags getFlags(char c) { return g_charFlagMap.flags[size_t(c)]; }

        /// Given a character return the lower case equivalent 
    SLANG_FORCE_INLINE static char toLower(char c) { return (c >= 'A' && c <= 'Z') ? (c -'A' + 'a') : c; }
        /// Given a character return the upper case equivalent
    SLANG_FORCE_INLINE static char toUpper(char c) { return (c >= 'a' && c <= 'z') ? (c -'a' + 'A') : c; }

    
    struct CharFlagMap
    {
        Flags flags[0x100];
    };

    static const CharFlagMap g_charFlagMap;
};
    
} // namespace Slang

#endif // SLANG_CHAR_UTIL_H
