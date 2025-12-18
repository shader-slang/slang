// slang-base-type-info.cpp
#include "slang-base-type-info.h"

namespace Slang
{

/* static */ const BaseTypeInfo BaseTypeInfo::s_info[Index(BaseType::CountOfPrimitives)] = {
    {0, 0, uint8_t(BaseType::Void)},
    {uint8_t(sizeof(bool)), 0, uint8_t(BaseType::Bool)},
    {uint8_t(sizeof(int8_t)),
     BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer,
     uint8_t(BaseType::Int8)},
    {uint8_t(sizeof(int16_t)),
     BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer,
     uint8_t(BaseType::Int16)},
    {uint8_t(sizeof(int32_t)),
     BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer,
     uint8_t(BaseType::Int)},
    {uint8_t(sizeof(int64_t)),
     BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer,
     uint8_t(BaseType::Int64)},
    {uint8_t(sizeof(uint8_t)), BaseTypeInfo::Flag::Integer, uint8_t(BaseType::UInt8)},
    {uint8_t(sizeof(uint16_t)), BaseTypeInfo::Flag::Integer, uint8_t(BaseType::UInt16)},
    {uint8_t(sizeof(uint32_t)), BaseTypeInfo::Flag::Integer, uint8_t(BaseType::UInt)},
    {uint8_t(sizeof(uint64_t)), BaseTypeInfo::Flag::Integer, uint8_t(BaseType::UInt64)},
    {uint8_t(sizeof(uint16_t)), BaseTypeInfo::Flag::FloatingPoint, uint8_t(BaseType::Half)},
    {uint8_t(sizeof(float)), BaseTypeInfo::Flag::FloatingPoint, uint8_t(BaseType::Float)},
    {uint8_t(sizeof(double)), BaseTypeInfo::Flag::FloatingPoint, uint8_t(BaseType::Double)},
    {uint8_t(sizeof(char)),
     BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer,
     uint8_t(BaseType::Char)},
    {uint8_t(sizeof(intptr_t)),
     BaseTypeInfo::Flag::Signed | BaseTypeInfo::Flag::Integer,
     uint8_t(BaseType::IntPtr)},
    {uint8_t(sizeof(uintptr_t)), BaseTypeInfo::Flag::Integer, uint8_t(BaseType::UIntPtr)},
};

/* static */ bool BaseTypeInfo::check()
{
    for (Index i = 0; i < SLANG_COUNT_OF(s_info); ++i)
    {
        if (s_info[i].baseType != i)
        {
            SLANG_ASSERT(!"Inconsistency between the s_info table and BaseInfo");
            return false;
        }
    }
    return true;
}

/* static */ UnownedStringSlice BaseTypeInfo::asText(BaseType baseType)
{
    switch (baseType)
    {
    case BaseType::Void:
        return UnownedStringSlice::fromLiteral("void");
    case BaseType::Bool:
        return UnownedStringSlice::fromLiteral("bool");
    case BaseType::Int8:
        return UnownedStringSlice::fromLiteral("int8_t");
    case BaseType::Int16:
        return UnownedStringSlice::fromLiteral("int16_t");
    case BaseType::Int:
        return UnownedStringSlice::fromLiteral("int");
    case BaseType::Int64:
        return UnownedStringSlice::fromLiteral("int64_t");
    case BaseType::UInt8:
        return UnownedStringSlice::fromLiteral("uint8_t");
    case BaseType::UInt16:
        return UnownedStringSlice::fromLiteral("uint16_t");
    case BaseType::UInt:
        return UnownedStringSlice::fromLiteral("uint");
    case BaseType::UInt64:
        return UnownedStringSlice::fromLiteral("uint64_t");
    case BaseType::Half:
        return UnownedStringSlice::fromLiteral("half");
    case BaseType::Float:
        return UnownedStringSlice::fromLiteral("float");
    case BaseType::Double:
        return UnownedStringSlice::fromLiteral("double");
    case BaseType::Char:
        return UnownedStringSlice::fromLiteral("char");
    case BaseType::IntPtr:
        return UnownedStringSlice::fromLiteral("intptr_t");
    case BaseType::UIntPtr:
        return UnownedStringSlice::fromLiteral("uintptr_t");
    default:
        {
            SLANG_ASSERT(!"Unknown basic type");
            return UnownedStringSlice();
        }
    }
}

} // namespace Slang
