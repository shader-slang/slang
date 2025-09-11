#include "slang-type-system-shared.h"

#include "../core/slang-common.h"

#include <bit>
#include <type_traits>

namespace Slang
{
template<typename T>
static constexpr BaseType GetBaseType()
{
    if constexpr (std::is_same_v<T, void>)
    {
        return BaseType::Void;
    }
    else if constexpr (std::is_same_v<T, bool>)
    {
        return BaseType::Bool;
    }
    else if constexpr (std::is_same_v<T, char>)
    {
        return BaseType::Char;
    }
    else if constexpr (std::is_integral_v<T>)
    {
        constexpr UInt rank = std::bit_width(sizeof(T)) - 1;
        constexpr BaseType base = std::is_signed_v<T> ? BaseType::Int8 : BaseType::UInt8;
        return BaseType((UInt)base + rank);
    }
    else if constexpr (std::is_floating_point_v<T>)
    {
        constexpr UInt rank = std::bit_width(sizeof(T)) - 1;
        return BaseType((UInt)BaseType::Half + rank);
    }
    else
    {
        static_assert(false, "Cannot deduce BaseType");
    }
    return {};
}

static constexpr const BaseType BaseEnumUnderlyingType[] = {
#define FILL_BASE_ENUM(Enum) GetBaseType<typename std::underlying_type_t<Enum>>(),
    FOREACH_BASE_ENUM(FILL_BASE_ENUM)
#undef FILL_BASE_ENUM
};

BaseType getBaseEnumUnderlyingType(BaseType enumBaseType)
{
    SLANG_ASSERT((UInt)enumBaseType > (UInt)BaseType::CountOfPrimitives);
    SLANG_ASSERT((UInt)enumBaseType < (UInt)BaseType::CountOf);
    return BaseEnumUnderlyingType[(UInt)enumBaseType - ((UInt)BaseType::CountOfPrimitives + 1)];
}
} // namespace Slang
