#include "slang-stdlib-textures.h"
#include <spirv/unified1/spirv.h>

#define EMIT_LINE_DIRECTIVE() sb << "#line " << (__LINE__+1) << " \"slang-stdlib-textures.cpp\"\n"

namespace Slang
{

// Concatenate anything which can be passed to a StringBuilder
template<typename... Ts>
String cat(const Ts&... xs)
{
    return (StringBuilder{} << ... << xs);
};

//
// Utilities
//

const auto indentWidth = 4;
static const char spaces[] = "                    ";
static_assert(SLANG_COUNT_OF(spaces) % indentWidth == 1);

struct BraceScope
{
    BraceScope(const char*& i, StringBuilder& sb, const char* end = "\n")
    :i(i), sb(sb), end(end)
    {
        // If we hit this assert, it means that we are indenting too deep and
        // need more spaces in 'spaces' above.
        SLANG_ASSERT(i != spaces);
        sb << i << "{\n";
        i -= indentWidth;
    }
    ~BraceScope()
    {
        // If we hit this assert, it means that we've got a bug unindenting
        // more than we've indented.
        SLANG_ASSERT(*i != '\0');
        i += indentWidth;
        sb << i << "}" << end;
    }
    const char*& i;
    StringBuilder& sb;
    const char* end;
};

TextureTypeInfo::TextureTypeInfo(
    BaseTextureShapeInfo const& base,
    bool isArray,
    bool isMultisample,
    bool isShadow,
    StringBuilder& inSB,
    String const& inPath)
    : base(base)
    , isArray(isArray)
    , isMultisample(isMultisample)
    , isShadow(isShadow)
    , sb(inSB)
    , path(inPath)
{
    i = spaces + SLANG_COUNT_OF(spaces) - 1;
}

void TextureTypeInfo::writeFuncBody(
    const char* funcName,
    const String& glsl,
    const String& cuda,
    const String& spirvDefault,
    const String& spirvRWDefault,
    const String& spirvCombined)
{
    BraceScope funcScope{i, sb};
    {
        sb << i << "__target_switch\n";
        BraceScope switchScope{i, sb};
        sb << i << "case cpp:\n";
        sb << i << "case hlsl:\n";
        sb << i << "__intrinsic_asm \"." << funcName << "\";\n";
        if(glsl.getLength())
        {
            sb << i << "case glsl:\n";
            if (glsl.startsWith("if"))
                sb << glsl;
            else
                sb << i << "__intrinsic_asm \"" << glsl << "\";\n";
        }
        if(cuda.getLength())
        {
            sb << i << "case cuda:\n";
            sb << i << "__intrinsic_asm \"" << cuda << "\";\n";
        }
        if(spirvDefault.getLength() && spirvCombined.getLength())
        {
            sb << i << "case spirv:\n";
            sb << i << "if (access == " << kStdlibResourceAccessReadWrite << ")\n";
            sb << i << "return spirv_asm\n";
            {
                BraceScope spirvRWScope{ i, sb, ";\n" };
                sb << spirvRWDefault << "\n";
            }
            sb << i << "else if (isCombined != 0)\n";
            sb << i << "{\n";
            {
                sb << i << "return spirv_asm\n";
                BraceScope spirvCombinedScope{i, sb, ";\n"};
                sb << spirvCombined << "\n";
            }
            sb << i << "}\n";
            sb << i << "else\n";
            sb << i << "{\n";
            {
                sb << i << "return spirv_asm\n";
                BraceScope spirvDefaultScope{i, sb, ";\n"};
                sb << spirvDefault << "\n";
            }
            sb << i << "}\n";
        }
    }
}

void TextureTypeInfo::writeFuncWithSig(
    const char* funcName,
    const String& sig,
    const String& glsl,
    const String& spirvDefault,
    const String& spirvRWDefault,
    const String& spirvCombined,
    const String& cuda,
    const ReadNoneMode readNoneMode)
{
    if (readNoneMode == ReadNoneMode::Always)
        sb << i << "[__readNone]\n";
    sb << i << "[__readNone]\n";
    sb << i << "[ForceInline]\n";
    sb << i << sig << "\n";
    writeFuncBody(funcName, glsl, cuda, spirvDefault, spirvRWDefault, spirvCombined);
    sb << "\n";
}

void TextureTypeInfo::writeFunc(
    const char* returnType,
    const char* funcName,
    const String& params,
    const String& glsl,
    const String& spirvDefault,
    const String& spirvRWDefault,
    const String& spirvCombined,
    const String& cuda,
    const ReadNoneMode readNoneMode)
{
    writeFuncWithSig(
        funcName,
        cat(returnType, " ", funcName, "(", params, ")"),
        glsl,
        spirvDefault,
        spirvRWDefault,
        spirvCombined,
        cuda,
        readNoneMode
    );
}

void TextureTypeInfo::writeGetDimensionFunctions()
{
    static const char* kComponentNames[]{ "x", "y", "z", "w" };

    SlangResourceShape baseShape = base.baseShape;

    // `GetDimensions`
    const char* dimParamTypes[] = { "out float ", "out int ", "out uint " };
    const char* dimParamTypesInner[] = { "float", "int", "uint" };
    for (int tid = 0; tid < 3; tid++)
    {
        auto t = dimParamTypes[tid];
        auto rawT = dimParamTypesInner[tid];

        for (int includeMipInfo = 0; includeMipInfo < 2; ++includeMipInfo)
        {
            if (includeMipInfo && isMultisample)
            {
                continue;
            }

            int sizeDimCount = 0;
            StringBuilder params;
            if (includeMipInfo)
                params << "uint mipLevel, ";

            switch (baseShape)
            {
            case SLANG_TEXTURE_1D:
                params << t << "width";
                sizeDimCount = 1;
                break;

            case SLANG_TEXTURE_2D:
            case SLANG_TEXTURE_CUBE:
                params << t << "width,";
                params << t << "height";
                sizeDimCount = 2;
                break;

            case SLANG_TEXTURE_3D:
                params << t << "width,";
                params << t << "height,";
                params << t << "depth";
                sizeDimCount = 3;
                break;

            default:
                assert(!"unexpected");
                break;
            }

            if (isArray)
            {
                params << ", " << t << "elements";
                sizeDimCount++;
            }

            if (isMultisample)
            {
                params << ", " << t << "sampleCount";
            }

            if (includeMipInfo)
                params << ", " << t << "numberOfLevels";


            StringBuilder glsl;
            {
                auto emitIntrinsic = [&](UnownedStringSlice funcName, bool useLodStr)
                    {
                        int aa = 1;
                        StringBuilder opStrSB;
                        opStrSB << " = " << funcName << "($0";
                        if (useLodStr)
                        {
                            String lodStr = ", 0";
                            if (includeMipInfo)
                            {
                                int mipLevelArg = aa++;
                                lodStr = ", int($";
                                lodStr.append(mipLevelArg);
                                lodStr.append(")");
                            }
                            opStrSB << lodStr;
                        }
                        auto opStr = opStrSB.produceString();
                        int cc = 0;
                        switch (baseShape)
                        {
                        case SLANG_TEXTURE_1D:
                            glsl << "($" << aa++ << opStr << ")";
                            if (isArray)
                            {
                                glsl << ".x";
                            }
                            glsl << ")";
                            cc = 1;
                            break;

                        case SLANG_TEXTURE_2D:
                        case SLANG_TEXTURE_CUBE:
                            glsl << "($" << aa++ << opStr << ").x)";
                            glsl << ", ($" << aa++ << opStr << ").y)";
                            cc = 2;
                            break;

                        case SLANG_TEXTURE_3D:
                            glsl << "($" << aa++ << opStr << ").x)";
                            glsl << ", ($" << aa++ << opStr << ").y)";
                            glsl << ", ($" << aa++ << opStr << ").z)";
                            cc = 3;
                            break;

                        default:
                            SLANG_UNEXPECTED("unhandled resource shape");
                            break;
                        }

                        if (isArray)
                        {
                            glsl << ", ($" << aa++ << opStr << ")." << kComponentNames[cc] << ")";
                        }

                        if (isMultisample)
                        {
                            glsl << ", ($" << aa++ << " = textureSamples($0))";
                        }

                        if (includeMipInfo)
                        {
                            glsl << ", ($" << aa++ << " = textureQueryLevels($0))";
                        }
                    };
                glsl << "if (access == " << kStdlibResourceAccessReadOnly << ") __intrinsic_asm \"";
                emitIntrinsic(toSlice("textureSize"), !isMultisample);
                glsl << "\";\n";
                glsl << "__intrinsic_asm \"";
                emitIntrinsic(toSlice("imageSize"), false);
                glsl << "\";\n";
            }

            // SPIRV ASM generation
            auto generateSpirvAsm = [&](StringBuilder& spirv, bool isRW, UnownedStringSlice imageVar) 
            {
                spirv << "%vecSize:$$uint";
                if (sizeDimCount > 1) spirv << sizeDimCount;
                spirv << " = ";
                if (isMultisample || isRW)
                    spirv << "OpImageQuerySize " << imageVar << ";";
                else
                    spirv << "OpImageQuerySizeLod " << imageVar <<" $0;";

                auto convertAndStore = [&](UnownedStringSlice uintSourceVal, const char* destParam)
                    {
                        if (UnownedStringSlice(rawT) == "uint")
                        {
                            spirv << "OpStore &" << destParam << " %" << uintSourceVal << ";";
                        }
                        else
                        {
                            if (UnownedStringSlice(rawT) == "int")
                            {
                                spirv << "%c_" << uintSourceVal << " : $$" << rawT << " = OpBitcast %" << uintSourceVal << "; ";
                            }
                            else
                            {
                                spirv << "%c_" << uintSourceVal << " : $$" << rawT << " = OpConvertUToF %" << uintSourceVal << "; ";
                            }
                            spirv << "OpStore &" << destParam << "%c_" << uintSourceVal << ";";
                        }
                    };
                auto extractSizeComponent = [&](int componentId, const char* destParam)
                    {
                        String elementVal = String("_") + destParam;
                        if (sizeDimCount == 1)
                        {
                            spirv << "%" << elementVal << " : $$uint = OpCopyObject %vecSize; ";
                        }
                        else
                        {
                            spirv << "%" << elementVal << " : $$uint = OpCompositeExtract %vecSize " << componentId << "; ";
                        }
                        convertAndStore(elementVal.getUnownedSlice(), destParam);
                    };
                switch (baseShape)
                {
                case SLANG_TEXTURE_1D:
                    extractSizeComponent(0, "width");
                    break;

                case SLANG_TEXTURE_2D:
                case SLANG_TEXTURE_CUBE:
                    extractSizeComponent(0, "width");
                    extractSizeComponent(1, "height");
                    break;

                case SLANG_TEXTURE_3D:
                    extractSizeComponent(0, "width");
                    extractSizeComponent(1, "height");
                    extractSizeComponent(2, "depth");
                    break;

                default:
                    assert(!"unexpected");
                    break;
                }

                if (isArray)
                {
                    extractSizeComponent(sizeDimCount - 1, "elements");
                }

                if (isMultisample)
                {
                    spirv << "%_sampleCount : $$uint = OpImageQuerySamples" << imageVar << ";";
                    convertAndStore(UnownedStringSlice("_sampleCount"), "sampleCount");
                }

                if (includeMipInfo)
                {
                    spirv << "%_levelCount : $$uint = OpImageQueryLevels" << imageVar << ";";
                    convertAndStore(UnownedStringSlice("_levelCount"), "numberOfLevels");
                }
            };
            StringBuilder spirvCombined;
            {
                spirvCombined << "OpCapability ImageQuery; ";
                spirvCombined << "%image:__imageType(this) = OpImage $this; ";
                generateSpirvAsm(spirvCombined, false, toSlice("%image"));
            }

            StringBuilder spirvDefault;
            {
                spirvDefault << "OpCapability ImageQuery; ";
                generateSpirvAsm(spirvDefault, false, toSlice("$this"));
            }

            StringBuilder spirvRWDefault;
            {
                spirvRWDefault << "OpCapability ImageQuery; ";
                generateSpirvAsm(spirvRWDefault, true, toSlice("$this"));
            }

            sb << "    __glsl_version(450)\n";
            sb << "    __glsl_extension(GL_EXT_samplerless_texture_functions)\n";
            writeFunc(
                "void",
                "GetDimensions",
                params,
                glsl,
                spirvDefault,
                spirvRWDefault,
                spirvCombined,
                "",
                ReadNoneMode::Always);
        }
    }
}

}
