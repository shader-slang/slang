// slang-base-type-info.cpp
#include "slang-base-type-info.h"

#include "slang-ast-support-types.h"

namespace Slang
{
namespace
{
// The core module generator emits scalar constructors for every builtin base
// type. These classes are the single source of truth for ranking those
// constructor conversions and for semantic-checking fast paths that bypass
// constructor lookup.
enum BaseTypeConversionKind : uint8_t
{
    kBaseTypeConversionKind_Signed,
    kBaseTypeConversionKind_Unsigned,
    kBaseTypeConversionKind_Float,
    kBaseTypeConversionKind_Error,
};
enum BaseTypeConversionRank : uint8_t
{
    kBaseTypeConversionRank_Bool,
    kBaseTypeConversionRank_Int8,
    kBaseTypeConversionRank_Int16,
    kBaseTypeConversionRank_Int32,
    kBaseTypeConversionRank_IntPtr,
    kBaseTypeConversionRank_Int64,
    kBaseTypeConversionRank_Error,
};

struct BaseTypeConversionInfo
{
    BaseType tag;
    BaseTypeConversionKind conversionKind;
    BaseTypeConversionRank conversionRank;
};

static BaseTypeConversionInfo _getBaseTypeConversionInfo(BaseType baseType)
{
    switch (baseType)
    {
    case BaseType::Bool:
        return {BaseType::Bool, kBaseTypeConversionKind_Unsigned, kBaseTypeConversionRank_Bool};

    case BaseType::Int8:
        return {BaseType::Int8, kBaseTypeConversionKind_Signed, kBaseTypeConversionRank_Int8};
    case BaseType::Int16:
        return {BaseType::Int16, kBaseTypeConversionKind_Signed, kBaseTypeConversionRank_Int16};
    case BaseType::Int:
        return {BaseType::Int, kBaseTypeConversionKind_Signed, kBaseTypeConversionRank_Int32};
    case BaseType::Int64:
        return {BaseType::Int64, kBaseTypeConversionKind_Signed, kBaseTypeConversionRank_Int64};
    case BaseType::IntPtr:
        return {BaseType::IntPtr, kBaseTypeConversionKind_Signed, kBaseTypeConversionRank_IntPtr};

    case BaseType::UInt8:
        return {BaseType::UInt8, kBaseTypeConversionKind_Unsigned, kBaseTypeConversionRank_Int8};
    case BaseType::UInt16:
        return {BaseType::UInt16, kBaseTypeConversionKind_Unsigned, kBaseTypeConversionRank_Int16};
    case BaseType::UInt:
        return {BaseType::UInt, kBaseTypeConversionKind_Unsigned, kBaseTypeConversionRank_Int32};
    case BaseType::UInt64:
        return {BaseType::UInt64, kBaseTypeConversionKind_Unsigned, kBaseTypeConversionRank_Int64};
    case BaseType::UIntPtr:
        return {
            BaseType::UIntPtr,
            kBaseTypeConversionKind_Unsigned,
            kBaseTypeConversionRank_IntPtr};

    case BaseType::Half:
        return {BaseType::Half, kBaseTypeConversionKind_Float, kBaseTypeConversionRank_Int16};
    case BaseType::Float:
        return {BaseType::Float, kBaseTypeConversionKind_Float, kBaseTypeConversionRank_Int32};
    case BaseType::Double:
        return {BaseType::Double, kBaseTypeConversionKind_Float, kBaseTypeConversionRank_Int64};

    default:
        return {baseType, kBaseTypeConversionKind_Error, kBaseTypeConversionRank_Error};
    }
}
} // namespace

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

ConversionCost getBaseTypeConversionCost(BaseType toType, BaseType fromType)
{
    auto toInfo = _getBaseTypeConversionInfo(toType);
    auto fromInfo = _getBaseTypeConversionInfo(fromType);
    if (toInfo.conversionKind == kBaseTypeConversionKind_Error ||
        fromInfo.conversionKind == kBaseTypeConversionKind_Error)
    {
        return kConversionCost_Impossible;
    }

    if (toInfo.conversionKind == fromInfo.conversionKind &&
        toInfo.conversionRank == fromInfo.conversionRank)
    {
        // These should represent the exact same type.
        return kConversionCost_None;
    }

    // Conversions within the same kind are easiest to handle: widening is a
    // promotion, while narrowing is the general conversion fallback.
    if (toInfo.conversionKind == fromInfo.conversionKind)
    {
        if (toInfo.conversionRank > fromInfo.conversionRank)
            return kConversionCost_RankPromotion;
        else
            return kConversionCost_GeneralConversion;
    }
    else if (fromInfo.tag == BaseType::Bool && toInfo.tag == BaseType::Int)
    {
        return kConversionCost_BoolToInt;
    }

    // An unsigned integer can promote to a signed integer only when the signed
    // rank is strictly larger and neither side is pointer-sized, because
    // pointer-sized integer width is target-dependent.
    else if (
        toInfo.conversionKind == kBaseTypeConversionKind_Signed &&
        fromInfo.conversionKind == kBaseTypeConversionKind_Unsigned &&
        toInfo.conversionRank > fromInfo.conversionRank &&
        toInfo.conversionRank != kBaseTypeConversionRank_IntPtr &&
        fromInfo.conversionRank != kBaseTypeConversionRank_IntPtr)
    {
        return kConversionCost_UnsignedToSignedPromotion;
    }
    else if (
        toInfo.conversionKind == kBaseTypeConversionKind_Signed &&
        fromInfo.conversionKind == kBaseTypeConversionKind_Unsigned &&
        toInfo.conversionRank == fromInfo.conversionRank &&
        toInfo.conversionRank != kBaseTypeConversionRank_IntPtr &&
        fromInfo.conversionRank != kBaseTypeConversionRank_IntPtr)
    {
        return kConversionCost_SameSizeUnsignedToSignedConversion;
    }

    // Signed-to-unsigned conversions are lossy, but they are preferred over
    // same-size unsigned-to-signed conversions.
    else if (
        toInfo.conversionKind == kBaseTypeConversionKind_Unsigned &&
        fromInfo.conversionKind == kBaseTypeConversionKind_Signed &&
        toInfo.conversionRank >= fromInfo.conversionRank)
    {
        return kConversionCost_SignedToUnsignedConversion;
    }

    // Integer-to-floating conversions are not promotions. We still rank float
    // and double above half so normal overload resolution prefers `float`.
    else if (
        toInfo.conversionKind == kBaseTypeConversionKind_Float &&
        toInfo.conversionRank >= kBaseTypeConversionRank_Int32 &&
        fromInfo.conversionRank >= kBaseTypeConversionRank_Int8)
    {
        return kConversionCost_IntegerToFloatConversion;
    }
    else if (
        toInfo.conversionKind == kBaseTypeConversionKind_Float &&
        toInfo.conversionRank >= kBaseTypeConversionRank_Int16 &&
        fromInfo.conversionRank >= kBaseTypeConversionRank_Int8)
    {
        return kConversionCost_IntegerToHalfConversion;
    }
    else
    {
        return kConversionCost_GeneralConversion;
    }
}

} // namespace Slang
