
#include "slang-type-text-util.h"


namespace Slang
{

#define SLANG_SCALAR_TYPES(x) \
    x(None, none) \
    x(Void, void) \
    x(Bool, bool) \
    x(Float16, half) \
    x(UInt32, uint32_t) \
    x(Int32, int32_t) \
    x(Int64, int64_t) \
    x(UInt64, uint64_t) \
    x(Float32, float) \
    x(Float64, double) 

namespace { // anonymous

struct ScalarTypeInfo
{
    slang::TypeReflection::ScalarType type;
    UnownedStringSlice text;
};

static const ScalarTypeInfo s_scalarTypeInfo[] =
{
    #define SLANG_SCALAR_TYPE_INFO(value, text) \
            { slang::TypeReflection::ScalarType::value, UnownedStringSlice::fromLiteral(#text) },
    SLANG_SCALAR_TYPES(SLANG_SCALAR_TYPE_INFO)
};

} // anonymous

/* static */UnownedStringSlice TypeTextUtil::asText(slang::TypeReflection::ScalarType scalarType)
{    
    typedef slang::TypeReflection::ScalarType ScalarType;
    switch (scalarType)
    {
#define SLANG_SCALAR_TYPE_TO_TEXT(value, text) case ScalarType::value:             return UnownedStringSlice::fromLiteral(#text);       
        SLANG_SCALAR_TYPES(SLANG_SCALAR_TYPE_TO_TEXT)
        default: break;
    }

    return UnownedStringSlice();
}

/* static */slang::TypeReflection::ScalarType TypeTextUtil::asScalarType(const UnownedStringSlice& inText)
{
    for (Index i = 0; i < SLANG_COUNT_OF(s_scalarTypeInfo); ++i)
    {
        const auto& info = s_scalarTypeInfo[i];
        if (info.text == inText)
        {
            return info.type;
        }
    }
    return slang::TypeReflection::ScalarType::None;
}

}

