
#include "slang-type-text-util.h"
#include "slang-array-view.h"

#include "slang-string-util.h"

namespace Slang
{




namespace { // anonymous

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

// Make sure to keep this table in sync with that in slang/slang-options.cpp getHelpText
static const TypeTextUtil::CompileTargetInfo s_compileTargetInfos[] =
{
    { SLANG_TARGET_UNKNOWN, "",                                                 "unknown"},
    { SLANG_TARGET_NONE,    "",                                                 "none"},
    { SLANG_HLSL,           "hlsl,fx",                                          "hlsl"},
    { SLANG_DXBC,           "dxbc",                                             "dxbc"},
    { SLANG_DXBC_ASM,       "dxbc-asm",                                         "dxbc-asm,dxbc-assembly" },
    { SLANG_DXIL,           "dxil",                                             "dxil" },
    { SLANG_DXIL_ASM,       "dxil-asm",                                         "dxil-asm,dxil-assembly" },
    { SLANG_GLSL,           "glsl,vert,frag,geom,tesc,tese,comp",               "glsl" },
    { SLANG_GLSL_VULKAN,    "",                                                 "glsl-vulkan" },
    { SLANG_GLSL_VULKAN_ONE_DESC, "",                                           "glsl-vulkan-one-desc" },
    { SLANG_SPIRV,          "spv",                                              "spirv"},
    { SLANG_SPIRV_ASM,      "spv-asm",                                          "spirv-asm,spirv-assembly" },
    { SLANG_C_SOURCE,       "c",                                                "c" },
    { SLANG_CPP_SOURCE,     "cpp,c++,cxx",                                      "cpp,c++,cxx" },
    { SLANG_CPP_PYTORCH_BINDING, "cpp,c++,cxx",                                 "torch,torch-binding,torch-cpp,torch-cpp-binding" },
    { SLANG_HOST_CPP_SOURCE, "cpp,c++,cxx",                                     "host-cpp,host-c++,host-cxx"},
    { SLANG_HOST_EXECUTABLE,"exe",                                              "exe,executable" },
    { SLANG_SHADER_SHARED_LIBRARY, "dll,so",                                    "sharedlib,sharedlibrary,dll" },
    { SLANG_CUDA_SOURCE,    "cu",                                               "cuda,cu"  },
    { SLANG_PTX,            "ptx",                                              "ptx" },
    { SLANG_CUDA_OBJECT_CODE, "obj,o",                                          "cuobj,cubin" },
    { SLANG_SHADER_HOST_CALLABLE,  "",                                          "host-callable,callable" },
    { SLANG_OBJECT_CODE,    "obj,o",                                            "object-code" },
    { SLANG_HOST_HOST_CALLABLE, "",                                             "host-host-callable" },
};

static const NamesValue s_languageInfos[] = 
{
    { SLANG_SOURCE_LANGUAGE_C, "c,C" },
    { SLANG_SOURCE_LANGUAGE_CPP, "cpp,c++,C++,cxx" },
    { SLANG_SOURCE_LANGUAGE_SLANG, "slang" },
    { SLANG_SOURCE_LANGUAGE_GLSL, "glsl" },
    { SLANG_SOURCE_LANGUAGE_HLSL, "hlsl" },
    { SLANG_SOURCE_LANGUAGE_CUDA, "cu,cuda" },
};

#define SLANG_PASS_THROUGH_TYPES(x) \
        x("none", NONE, "Unknown") \
        x("fxc", FXC, "fxc") \
        x("dxc", DXC, "dxc") \
        x("glslang", GLSLANG, "glslang") \
        x("visualstudio,vs", VISUAL_STUDIO, "Visual Studio") \
        x("clang", CLANG, "Clang") \
        x("gcc", GCC, "GCC") \
        x("genericcpp,c,cpp", GENERIC_C_CPP, "Generic C/C++ compiler") \
        x("nvrtc", NVRTC, "NVRTC (Cuda compiler)") \
        x("llvm", LLVM, "LLVM/Clang")

static const NamesDescriptionValue s_compilerInfos[] = 
{
    #define SLANG_PASS_THROUGH_INFO(x, y, human) { SLANG_PASS_THROUGH_##y, x, human },
    SLANG_PASS_THROUGH_TYPES(SLANG_PASS_THROUGH_INFO)
};

static const NameValue s_archiveTypeInfos[] =
{
    { SLANG_ARCHIVE_TYPE_RIFF_DEFLATE, "riff-deflate"},
    { SLANG_ARCHIVE_TYPE_RIFF_LZ4, "riff-lz4"},
    { SLANG_ARCHIVE_TYPE_ZIP, "zip"},
    { SLANG_ARCHIVE_TYPE_RIFF, "riff"},
};

static const NameValue s_debugInfoFormatInfos[] = 
{
    { SLANG_DEBUG_INFO_FORMAT_DEFAULT, "default-format" },
    { SLANG_DEBUG_INFO_FORMAT_C7, "c7" },
    { SLANG_DEBUG_INFO_FORMAT_PDB, "pdb" },
    { SLANG_DEBUG_INFO_FORMAT_STABS, "stabs" },
    { SLANG_DEBUG_INFO_FORMAT_COFF, "coff" },
    { SLANG_DEBUG_INFO_FORMAT_DWARF, "dwarf" },
};

static const NamesDescriptionValue s_lineDirectiveInfos[] = 
{
    { SLANG_LINE_DIRECTIVE_MODE_NONE, "none", "Don't emit `#line` directives at all"}, 
    { SLANG_LINE_DIRECTIVE_MODE_SOURCE_MAP, "source-map", "Use source map to track line associations (doen't emit #line)"},
    { SLANG_LINE_DIRECTIVE_MODE_DEFAULT, "default", "Default behavior"},
    { SLANG_LINE_DIRECTIVE_MODE_STANDARD, "standard", "Emit standard C-style `#line` directives." },
    { SLANG_LINE_DIRECTIVE_MODE_GLSL, "glsl", "Emit GLSL-style directives with file *number* instead of name." },
};

static const NamesDescriptionValue s_floatingPointModes[] =
{
    { SLANG_FLOATING_POINT_MODE_PRECISE, "precise",
    "Disable optimization that could change the output of floating-"
    "point computations, including around infinities, NaNs, denormalized "
    "values, and negative zero. Prefer the most precise versions of special "
    "functions supported by the target."},
    { SLANG_FLOATING_POINT_MODE_FAST,"fast",
    "Allow optimizations that may change results of floating-point "
    "computations. Prefer the fastest version of special functions supported "
    "by the target."},
    { SLANG_FLOATING_POINT_MODE_DEFAULT, "default", 
    "Default floating point mode" }
};

} // anonymous

/* static */ConstArrayView<TypeTextUtil::CompileTargetInfo> TypeTextUtil::getCompileTargetInfos()
{
    return makeConstArrayView(s_compileTargetInfos);
}

/* static */ConstArrayView<NamesValue> TypeTextUtil::getLanguageInfos()
{
    return makeConstArrayView(s_languageInfos);
}

/* static */ConstArrayView<NamesDescriptionValue> TypeTextUtil::getCompilerInfos()
{
    return makeConstArrayView(s_compilerInfos);
}

/* static */ConstArrayView<NameValue> TypeTextUtil::getArchiveTypeInfos()
{
    return makeConstArrayView(s_archiveTypeInfos);
}

/* static */ConstArrayView<NameValue> TypeTextUtil::getDebugInfoFormatInfos()
{
    return makeConstArrayView(s_debugInfoFormatInfos);
}

/* static */ConstArrayView<NamesDescriptionValue> TypeTextUtil::getLineDirectiveInfos()
{
    return makeConstArrayView(s_lineDirectiveInfos);
}

/* static */ConstArrayView<NamesDescriptionValue> TypeTextUtil::getFloatingPointModeInfos()
{
    return makeConstArrayView(s_floatingPointModes);
}

/* static */SlangArchiveType TypeTextUtil::findArchiveType(const UnownedStringSlice& slice)
{
    return NameValueUtil::findValue(getArchiveTypeInfos(), slice, SLANG_ARCHIVE_TYPE_UNDEFINED);
}

/* static */SlangResult TypeTextUtil::findDebugInfoFormat(const Slang::UnownedStringSlice& text, SlangDebugInfoFormat& out)
{
    const ValueInt value = NameValueUtil::findValue(getDebugInfoFormatInfos(), text, -1);
    if (value >= 0)
    {
        out = SlangDebugInfoFormat(value);
        return SLANG_OK;
    }
    return SLANG_FAIL;
}

/* static */UnownedStringSlice TypeTextUtil::getDebugInfoFormatName(SlangDebugInfoFormat format) 
{
    return NameValueUtil::findName(getDebugInfoFormatInfos(), format, toSlice("unknown"));
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


/* static */UnownedStringSlice TypeTextUtil::getPassThroughAsHumanText(SlangPassThrough type)
{
    return NameValueUtil::findDescription(getCompilerInfos(), type, toSlice("unknown"));
}

/* static */SlangSourceLanguage TypeTextUtil::findSourceLanguage(const UnownedStringSlice& text)
{
    return NameValueUtil::findValue(getLanguageInfos(), text, SLANG_SOURCE_LANGUAGE_UNKNOWN);
}

/* static */SlangPassThrough TypeTextUtil::findPassThrough(const UnownedStringSlice& slice)
{
    return NameValueUtil::findValue(getCompilerInfos(), slice, SLANG_PASS_THROUGH_NONE);
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
    return NameValueUtil::findName(getCompilerInfos(), passThru, toSlice("unknown"));
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

