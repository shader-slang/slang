// slang-base-type-info.h
#pragma once

//
// This file defines the `BaseTypeInfo` type, which encodes
// information (such as size in bits) about the base types
// supported by the Slang language. That information is used
// for things like checking if a literal is in the representible
// range of a given type, and for determining the relative
// cost of implicit conversions between the base types.
//

#include "../core/slang-basic.h"
#include "slang-type-system-shared.h"

namespace Slang
{

// Information about BaseType that's useful for checking literals
struct BaseTypeInfo
{
    typedef uint8_t Flags;
    struct Flag
    {
        enum Enum : Flags
        {
            Signed = 0x1,
            FloatingPoint = 0x2,
            Integer = 0x4,
        };
    };

    SLANG_FORCE_INLINE static const BaseTypeInfo& getInfo(BaseType baseType)
    {
        return s_info[Index(baseType)];
    }

    static UnownedStringSlice asText(BaseType baseType);

    uint8_t sizeInBytes; ///< Size of type in bytes
    Flags flags;
    uint8_t baseType;

    static bool check();

private:
    static const BaseTypeInfo s_info[Index(BaseType::CountOfPrimitives)];
};

} // namespace Slang
