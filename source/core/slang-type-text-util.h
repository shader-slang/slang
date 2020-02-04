#ifndef SLANG_CORE_TYPE_TEXT_UTIL_H
#define SLANG_CORE_TYPE_TEXT_UTIL_H

#include "../../slang.h"

#include "slang-string.h"


namespace Slang
{

/// Utility class to allow conversion of types (such as enums) to and from text types
struct TypeTextUtil
{

        /// Get the scalar type as text.
    static Slang::UnownedStringSlice asText(slang::TypeReflection::ScalarType scalarType);

        // Converts text to scalar type. Returns 'none' if not determined
    static slang::TypeReflection::ScalarType asScalarType(const Slang::UnownedStringSlice& text);

        /// As human readable text
    static UnownedStringSlice asHumanText(SlangPassThrough type);

        /// Given a source language name returns a source language. Name here is distinct from extension
    static SlangSourceLanguage asSourceLanguage(const UnownedStringSlice& text);

        /// Given a name returns the pass through
    static SlangPassThrough asPassThrough(const UnownedStringSlice& slice);
    static SlangResult asPassThrough(const UnownedStringSlice& slice, SlangPassThrough& outPassThrough);

        /// Get the compilers name
    static UnownedStringSlice asText(SlangPassThrough passThru);
};

}

#endif // SLANG_CORE_TYPE_TEXT_UTIL_H
