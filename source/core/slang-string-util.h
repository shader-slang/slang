#ifndef SLANG_STRING_UTIL_H
#define SLANG_STRING_UTIL_H

#include "slang-string.h"
#include "list.h"

#include <stdarg.h>

namespace Slang {

struct StringUtil
{
        /// Split in, by specified splitChar into slices out
        /// Slices contents will directly address into in, so contents will only stay valid as long as in does.
    static void split(const UnownedStringSlice& in, char splitChar, List<UnownedStringSlice>& slicesOut);

        /// Appends formatted string with args into buf
    static void append(const char* format, va_list args, StringBuilder& buf);

        /// Appends the formatted string with specified trailing args
    static void appendFormat(StringBuilder& buf, const char* format, ...);

        /// Create a string from the format string applying args (like sprintf)
    static String makeStringWithFormat(const char* format, ...);
};

} // namespace Slang

#endif // SLANG_STRING_UTIL_H
