#ifndef SLANG_CORE_NAME_CONVENTION_UTIL_H
#define SLANG_CORE_NAME_CONVENTION_UTIL_H

#include "slang-string.h"
#include "slang-list.h"

namespace Slang
{

struct NameConventionUtil
{
       /// Calculate as lower case dash delimited. For example
        /// So MyString -> my-string
    static void camelCaseToLowerDashed(const UnownedStringSlice& in, StringBuilder& out);

        /// Calculate a snake cased name as as lower dashed.
        /// So MY_STRING -> my-string
    static void snakeCaseToLowerDashed(const UnownedStringSlice& in, StringBuilder& out);

        /// Calculate equivalent snake case input to upper camel output
        /// So MY_STRING -> MyString
    static void snakeCaseToUpperCamel(const UnownedStringSlice& in, StringBuilder& out);

        /// Convert dashed to upper snake
        /// So my-string -> MY_STRING
    static void dashedToUpperSnake(const UnownedStringSlice& in, StringBuilder& out);
};

}

#endif // SLANG_CORE_NAME_CONVENTION_UTIL_H
