#ifndef SLANG_CORE_STRING_ESCAPE_UTIL_H
#define SLANG_CORE_STRING_ESCAPE_UTIL_H

#include "slang-string.h"

namespace Slang {

/* A set of function that can be used for escaping/unescaping quoting/unquoting strings.

The distinction between 'escaping' and 'quoting' here, is just that escaping is the 'payload' of quotes. 
In *principal* the Style can determine different styles of escaping that can be used.

TODO(JS): NOTE! Currently style is largely ignored. 

Use CommandLine::kQuoteStyle for the quoting style that the command line for the current platform uses. 
*/
struct StringEscapeUtil
{
    enum class Style
    {
        Cpp,        ///< Escape/quoting in the style on C/C++
        UnixCmd,    ///< Unix command line style escaping
        WinCmd,     ///< Windows command line style 
    };

    static bool isWinQuotingNeeded(const UnownedStringSlice& slice);
    static bool isCppQuotingNeeded(const UnownedStringSlice& slice);
    static bool isUnixQuotingNeeded(const UnownedStringSlice& slice);

    static SlangResult appendCppUnescaped(const UnownedStringSlice& slice, StringBuilder& out);
    static SlangResult appendUnixUnescaped(const UnownedStringSlice& slice, StringBuilder& out);
    static SlangResult appendWinUnescaped(const UnownedStringSlice& slice, StringBuilder& out);

    static void appendWinEscaped(const UnownedStringSlice& slice, StringBuilder& out);
    static void appendCppEscaped(const UnownedStringSlice& slice, StringBuilder& out);
    static void appendUnixEscaped(const UnownedStringSlice& slice, StringBuilder& out);

        /// True if quoting is needed
    static bool isQuotingNeeded(Style style, const UnownedStringSlice& slice);

        /// If slice needs quoting for the specified style, append quoted to out
    static void appendMaybeQuoted(Style style, const UnownedStringSlice& slice, StringBuilder& out);

        /// If the slice appears to be quoted for the style, unquote it, else just append to out
    static SlangResult appendMaybeUnquoted(Style style, const UnownedStringSlice& slice, StringBuilder& out);

        /// Append with quotes (even if not needed)
    static void appendQuoted(Style style, const UnownedStringSlice& slice, StringBuilder& out);

        /// Append unquoted to out. 
    static SlangResult appendUnquoted(Style style, const UnownedStringSlice& slice, StringBuilder& out);

        /// Takes slice and adds C++/C type escaping for special characters (like '\', '"' and if not ascii will write out as hex sequence)
        /// Does not append double quotes around the output
    static void appendEscaped(Style style, const UnownedStringSlice& slice, StringBuilder& out);

    
        /// Given a slice append it unescaped
        /// Does not consume surrounding quotes
    static SlangResult appendUnescaped(Style style, const UnownedStringSlice& slice, StringBuilder& out);

        /// Lex quoted text.
        /// The first character of cursor should be the quoteCharacter. 
        /// cursor points to the string to be lexed - must typically be 0 terminated.
        /// outCursor on successful lex will be at the next character after was processed.
    static SlangResult lexQuoted(Style style, const char* cursor, char quoteChar, const char** outCursor);
};

} // namespace Slang

#endif // SLANG_CORE_STRING_ESCAPE_UTIL_H
