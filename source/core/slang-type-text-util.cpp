
#include "slang-type-text-util.h"

#include "slang-string-util.h"

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
        x(nvrtc, NVRTC) \
        x(llvm, LLVM)

namespace { // anonymous

struct ScalarTypeInfo
{
    slang::TypeReflection::ScalarType type;
    UnownedStringSlice text;
};

static const ScalarTypeInfo s_scalarTypeInfos[] =
{
    #define SLANG_SCALAR_TYPE_INFO(value, text) \
            { slang::TypeReflection::ScalarType::value, UnownedStringSlice::fromLiteral(#text) },
    SLANG_SCALAR_TYPES(SLANG_SCALAR_TYPE_INFO)
};

struct CompileTargetInfo
{
    SlangCompileTarget target;          ///< The target
    const char* extensions;             ///< Comma delimited list of extensions associated with the target
    const char* names;                  ///< Comma delimited list of names associated with the target. NOTE! First name is taken as the normal display name.
};

static const CompileTargetInfo s_compileTargetInfos[] = 
{
    { SLANG_TARGET_UNKNOWN, "",                                                 "unknown"},
    { SLANG_TARGET_NONE,    "",                                                 "none"},
    { SLANG_HLSL,           "hlsl,fx",                                          "hlsl"},
    { SLANG_DXBC,           "dxbc",                                             "dxbc"},
    { SLANG_DXBC_ASM,       "dxbc.asm",                                         "dxbc-asm,dxbc-assembly" },
    { SLANG_DXIL,           "dxil",                                             "dxil" },
    { SLANG_DXIL_ASM,       "dxil.asm",                                         "dxil-asm,dxil-assembly" },
    { SLANG_GLSL,           "glsl,vert,frag,geom,tesc,tese,comp",               "glsl" },
    { SLANG_GLSL_VULKAN,    "",                                                 "glsl-vulkan" },
    { SLANG_GLSL_VULKAN_ONE_DESC, "",                                           "glsl-vulkan-one-desc" },
    { SLANG_SPIRV,          "spv",                                              "spirv"},
    { SLANG_SPIRV_ASM,      "spv.asm",                                          "spirv-asm,spirv-assembly" },
    { SLANG_C_SOURCE,       "c",                                                "c" },
    { SLANG_CPP_SOURCE,     "cpp,c++,cxx",                                      "cpp,c++,cxx" },
    { SLANG_HOST_CPP_SOURCE, "cpp,c++,cxx",                                     "host-cpp,host-c++,host-cxx"},
    { SLANG_HOST_EXECUTABLE,"exe",                                              "exe,executable" },
    { SLANG_SHADER_SHARED_LIBRARY, "dll,so",                                    "sharedlib,sharedlibrary,dll" },
    { SLANG_CUDA_SOURCE,    "cu",                                               "cuda,cu"  },
    { SLANG_PTX,            "ptx",                                              "ptx" },
    { SLANG_SHADER_HOST_CALLABLE,  "",                                          "host-callable,callable" },
    { SLANG_OBJECT_CODE,    "obj,o",                                            "object-code" },
    { SLANG_HOST_HOST_CALLABLE, "",                                             "host-host-callable" },


};

struct ArchiveTypeInfo
{
    SlangArchiveType type;
    UnownedStringSlice text;
};

static const ArchiveTypeInfo s_archiveTypeInfos[] =
{
    { SLANG_ARCHIVE_TYPE_RIFF_DEFLATE, UnownedStringSlice::fromLiteral("riff-deflate")},
    { SLANG_ARCHIVE_TYPE_RIFF_LZ4, UnownedStringSlice::fromLiteral("riff-lz4")},
    { SLANG_ARCHIVE_TYPE_ZIP, UnownedStringSlice::fromLiteral("zip")},
    { SLANG_ARCHIVE_TYPE_RIFF, UnownedStringSlice::fromLiteral("riff")},
};

} // anonymous

/* static */SlangArchiveType TypeTextUtil::findArchiveType(const UnownedStringSlice& slice)
{
    for (const auto& entry : s_archiveTypeInfos)
    {
        if (slice == entry.text)
        {
            return entry.type;
        }
    }
    return SLANG_ARCHIVE_TYPE_UNDEFINED;
}

/* static */UnownedStringSlice TypeTextUtil::getScalarTypeName(slang::TypeReflection::ScalarType scalarType)
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

/* static */slang::TypeReflection::ScalarType TypeTextUtil::findScalarType(const UnownedStringSlice& inText)
{
    for (Index i = 0; i < SLANG_COUNT_OF(s_scalarTypeInfos); ++i)
    {
        const auto& info = s_scalarTypeInfos[i];
        if (info.text == inText)
        {
            return info.type;
        }
    }
    return slang::TypeReflection::ScalarType::None;
}

#define SLANG_PASS_THROUGH_HUMAN_TEXT(x) \
    x(NONE,             "Unknown") \
    x(VISUAL_STUDIO,    "Visual Studio") \
    x(GCC,              "GCC") \
    x(CLANG,            "Clang") \
    x(NVRTC,            "NVRTC") \
    x(FXC,              "fxc") \
    x(DXC,              "dxc") \
    x(GLSLANG,          "glslang") \
    x(LLVM,             "LLVM/Clang")

/* static */UnownedStringSlice TypeTextUtil::getPassThroughAsHumanText(SlangPassThrough type)
{
#define SLANG_PASS_THROUGH_HUMAN_CASE(value, text)  case SLANG_PASS_THROUGH_##value: return UnownedStringSlice::fromLiteral(text); 

    switch (type)
    {
        default:    /* fall-through to none */
        SLANG_PASS_THROUGH_HUMAN_TEXT(SLANG_PASS_THROUGH_HUMAN_CASE)
    }
}

/* static */SlangResult TypeTextUtil::findPassThroughFromHumanText(const UnownedStringSlice& inText, SlangPassThrough& outPassThrough)
{
    #define SLANG_PASS_THROUGH_HUMAN_IF(value, text)  if (inText == UnownedStringSlice::fromLiteral(text)) { outPassThrough = SLANG_PASS_THROUGH_##value; return SLANG_OK; } else
    SLANG_PASS_THROUGH_HUMAN_TEXT(SLANG_PASS_THROUGH_HUMAN_IF)
    return SLANG_FAIL;
}

/* static */SlangSourceLanguage TypeTextUtil::findSourceLanguage(const UnownedStringSlice& text)
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

/* static */SlangPassThrough TypeTextUtil::findPassThrough(const UnownedStringSlice& slice)
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

/* static */SlangResult TypeTextUtil::findPassThrough(const UnownedStringSlice& slice, SlangPassThrough& outPassThrough)
{
    outPassThrough = findPassThrough(slice);
    // It could be none on error - if it's not equal to "none" then it must be an error
    if (outPassThrough == SLANG_PASS_THROUGH_NONE && slice != UnownedStringSlice::fromLiteral("none"))
    {
        return SLANG_FAIL;
    }
    return SLANG_OK;
}

/* static */UnownedStringSlice TypeTextUtil::getPassThroughName(SlangPassThrough passThru)
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

/* static */SlangCompileTarget TypeTextUtil::findCompileTargetFromExtension(const UnownedStringSlice& slice)
{
    if (slice.getLength())
    {
        for (const auto& info : s_compileTargetInfos)
        {
            if (StringUtil::indexOfInSplit(UnownedStringSlice(info.extensions), ',', slice) >= 0)
            {
                return info.target;
            }
        }
    }
    return SLANG_TARGET_UNKNOWN;
}

/* static */ SlangCompileTarget TypeTextUtil::findCompileTargetFromName(const UnownedStringSlice& slice)
{
    if (slice.getLength())
    {
        for (const auto& info : s_compileTargetInfos)
        {
            if (StringUtil::indexOfInSplit(UnownedStringSlice(info.names), ',', slice) >= 0)
            {
                return info.target;
            }
        }
    }
    return SLANG_TARGET_UNKNOWN;
}

static Index _getTargetInfoIndex(SlangCompileTarget target)
{
    for (Index i = 0; i < SLANG_COUNT_OF(s_compileTargetInfos); ++i)
    {
        if (s_compileTargetInfos[i].target == target)
        {
            return i;
        }
    }
    return -1;
}

UnownedStringSlice TypeTextUtil::getCompileTargetName(SlangCompileTarget target)
{
    const Index index = _getTargetInfoIndex(target);
    // Return the first name
    return index >= 0 ? StringUtil::getAtInSplit(UnownedStringSlice(s_compileTargetInfos[index].names), ',', 0) : UnownedStringSlice();
}

}

