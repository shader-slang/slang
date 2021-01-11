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
    static Slang::UnownedStringSlice getScalarTypeName(slang::TypeReflection::ScalarType scalarType);

        // Converts text to scalar type. Returns 'none' if not determined
    static slang::TypeReflection::ScalarType findScalarType(const Slang::UnownedStringSlice& text);

        /// As human readable text
    static UnownedStringSlice getPassThroughAsHumanText(SlangPassThrough type);

        /// Given a source language name returns a source language. Name here is distinct from extension
    static SlangSourceLanguage findSourceLanguage(const UnownedStringSlice& text);

        /// Given a name returns the pass through
    static SlangPassThrough findPassThrough(const UnownedStringSlice& slice);
    static SlangResult findPassThrough(const UnownedStringSlice& slice, SlangPassThrough& outPassThrough);

        /// Get the compilers name
    static UnownedStringSlice getPassThroughName(SlangPassThrough passThru);

        /// Given a file extension determines suitable target
        /// If doesn't match any target will return SLANG_TARGET_UNKNOWN
    static SlangCompileTarget findCompileTargetFromExtension(const UnownedStringSlice& slice);

        /// Given a name suitable target
        /// If doesn't match any target will return SLANG_TARGET_UNKNOWN
    static SlangCompileTarget findCompileTargetFromName(const UnownedStringSlice& slice);

        /// Given a target returns the associated name.
    static UnownedStringSlice getCompileTargetName(SlangCompileTarget target);

        /// Returns SLANG_ARCHIVE_TYPE_UNKNOWN if a match is not found
    static SlangArchiveType findArchiveType(const UnownedStringSlice& slice);
};

}

#endif // SLANG_CORE_TYPE_TEXT_UTIL_H
