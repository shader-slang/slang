
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

#define SLANG_PASS_THROUGH_TYPES(x) \
        x(none, NONE) \
        x(fxc, FXC) \
        x(dxc, DXC) \
        x(glslang, GLSLANG) \
        x(visualstudio, VISUAL_STUDIO) \
        x(clang, CLANG) \
        x(gcc, GCC) \
        x(genericcpp, GENERIC_C_CPP) \
        x(nvrtc, NVRTC)

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

/* static */UnownedStringSlice TypeTextUtil::asHumanText(SlangPassThrough type)
{
    switch (type)
    {
        default:
        case SLANG_PASS_THROUGH_NONE:           return UnownedStringSlice::fromLiteral("Unknown");
        case SLANG_PASS_THROUGH_VISUAL_STUDIO:  return UnownedStringSlice::fromLiteral("Visual Studio");
        case SLANG_PASS_THROUGH_GCC:            return UnownedStringSlice::fromLiteral("GCC");
        case SLANG_PASS_THROUGH_CLANG:          return UnownedStringSlice::fromLiteral("Clang");
        case SLANG_PASS_THROUGH_NVRTC:          return UnownedStringSlice::fromLiteral("NVRTC");
        case SLANG_PASS_THROUGH_FXC:            return UnownedStringSlice::fromLiteral("fxc");
        case SLANG_PASS_THROUGH_DXC:            return UnownedStringSlice::fromLiteral("dxc");
        case SLANG_PASS_THROUGH_GLSLANG:        return UnownedStringSlice::fromLiteral("glslang");
    }
}

/* static */SlangSourceLanguage TypeTextUtil::asSourceLanguage(const UnownedStringSlice& text)
{
    if (text == "c" || text == "C")
    {
        return SLANG_SOURCE_LANGUAGE_C;
    }
    else if (text == "cpp" || text == "c++" || text == "C++" || text == "cxx")
    {
        return SLANG_SOURCE_LANGUAGE_CPP;
    }
    else if (text == "slang")
    {
        return SLANG_SOURCE_LANGUAGE_SLANG;
    }
    else if (text == "glsl")
    {
        return SLANG_SOURCE_LANGUAGE_GLSL;
    }
    else if (text == "hlsl")
    {
        return SLANG_SOURCE_LANGUAGE_HLSL;
    }
    else if (text == "cu" || text == "cuda")
    {
        return SLANG_SOURCE_LANGUAGE_CUDA;
    }
    return SLANG_SOURCE_LANGUAGE_UNKNOWN;
}

/* static */SlangPassThrough TypeTextUtil::asPassThrough(const UnownedStringSlice& slice)
{
#define SLANG_PASS_THROUGH_NAME_TO_TYPE(x, y) \
    if (slice == UnownedStringSlice::fromLiteral(#x)) return SLANG_PASS_THROUGH_##y;

    SLANG_PASS_THROUGH_TYPES(SLANG_PASS_THROUGH_NAME_TO_TYPE)

        // Other options
        if (slice == "c" || slice == "cpp")
        {
            return SLANG_PASS_THROUGH_GENERIC_C_CPP;
        }
        else if (slice == "vs")
        {
            return SLANG_PASS_THROUGH_VISUAL_STUDIO;
        }

    return SLANG_PASS_THROUGH_NONE;
}

/* static */SlangResult TypeTextUtil::asPassThrough(const UnownedStringSlice& slice, SlangPassThrough& outPassThrough)
{
    outPassThrough = asPassThrough(slice);
    // It could be none on error - if it's not equal to "none" then it must be an error
    if (outPassThrough == SLANG_PASS_THROUGH_NONE && slice != UnownedStringSlice::fromLiteral("none"))
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

/* static */UnownedStringSlice TypeTextUtil::asText(SlangPassThrough passThru)
{
#define SLANG_PASS_THROUGH_TYPE_TO_NAME(x, y) \
    case SLANG_PASS_THROUGH_##y: return UnownedStringSlice::fromLiteral(#x);

    switch (passThru)
    {
        SLANG_PASS_THROUGH_TYPES(SLANG_PASS_THROUGH_TYPE_TO_NAME)
        default: break;
    }
    return UnownedStringSlice::fromLiteral("unknown");
}


}

