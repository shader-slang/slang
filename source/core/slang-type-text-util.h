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
};

}

#endif // SLANG_CORE_TYPE_TEXT_UTIL_H
