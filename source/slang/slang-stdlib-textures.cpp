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
    TextureTypePrefixInfo const& prefixInfo,
    BaseTextureShapeInfo const& base,
    bool isArray,
    bool isMultisample,
    BaseTextureAccessInfo const& accessInfo,
    StringBuilder& inSB,
    String const& inPath)
    : prefixInfo(prefixInfo)
    , base(base)
    , isArray(isArray)
    , isMultisample(isMultisample)
    , accessInfo(accessInfo)
    , sb(inSB)
    , path(inPath)
{
    i = spaces + SLANG_COUNT_OF(spaces) - 1;
}

void TextureTypeInfo::writeFuncBody(
    const char* funcName,
    const String& glsl,
    const String& cuda,
    const String& spirv)
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
            sb << i << "__intrinsic_asm \"" << glsl << "\";\n";
        }
        if(cuda.getLength())
        {
            sb << i << "case cuda:\n";
            sb << i << "__intrinsic_asm \"" << cuda << "\";\n";
        }
        if(spirv.getLength())
        {
            sb << i << "case spirv:\n";
            sb << i << "return spirv_asm\n";
            BraceScope spirvScope{i, sb, ";\n"};
            sb << spirv << "\n";
        }
    }
}

void TextureTypeInfo::writeFuncDecorations(
    const String& glsl,
    const String& cuda)
{
    if(glsl.getLength())
        sb << i << "__target_intrinsic(glsl, \"" << glsl << "\")\n";
    if(cuda.getLength())
        sb << i << "__target_intrinsic(cuda, \"" << cuda << "\")\n";
}

void TextureTypeInfo::writeFuncWithSig(
    const char* funcName,
    const String& sig,
    const String& glsl,
    const String& spirv,
    const String& cuda,
    const ReadNoneMode readNoneMode)
{
    const bool isReadOnly = (accessInfo.access == SLANG_RESOURCE_ACCESS_READ);
    const bool rn =
        readNoneMode == ReadNoneMode::Always
        || readNoneMode == ReadNoneMode::IfReadOnly && isReadOnly;
    if(spirv.getLength())
    {
        if(rn)
            sb << i << "[__readNone]\n";
        sb << i << sig << "\n";
        writeFuncBody(funcName, glsl, cuda, spirv);
    }
    else
    {
        writeFuncDecorations(glsl, cuda);
        if(rn)
            sb << i << "[__readNone]\n";
        sb << i << sig << ";\n";
    }
    sb << "\n";
}

void TextureTypeInfo::writeFunc(
    const char* returnType,
    const char* funcName,
    const String& params,
    const String& glsl,
    const String& spirv,
    const String& cuda,
    const ReadNoneMode readNoneMode)
{
    writeFuncWithSig(
        funcName,
        cat(returnType, " ", funcName, "(", params, ")"),
        glsl,
        spirv,
        cuda,
        readNoneMode
    );
}

void TextureTypeInfo::emitTypeDecl()
{
    char const* baseName = prefixInfo.name;
    char const* baseShapeName = base.shapeName;
    TextureFlavor::Shape baseShape = base.baseShape;

    // Arrays of 3D textures aren't allowed
    if (isArray && baseShape == TextureFlavor::Shape::Shape3D) return;

    auto access = accessInfo.access;

    // No such thing as RWTextureCube
    if (access == SLANG_RESOURCE_ACCESS_READ_WRITE && baseShape == TextureFlavor::Shape::ShapeCube)
    {
        return;
    }

    // TODO: any constraints to enforce on what gets to be multisampled?

    unsigned flavor = baseShape;
    if (isArray)		flavor |= TextureFlavor::ArrayFlag;
    if (isMultisample)	flavor |= TextureFlavor::MultisampleFlag;
    // if (isShadow)		flavor |= TextureFlavor::ShadowFlag;

    flavor |= (access << 8);

    // emit a generic signature
    sb << "__generic<T = float4";
    // Multi-sample rw texture types have an optional sampleCount parameter.
    if (isMultisample)
        sb << ", let sampleCount : int = 0";
    sb << ">";

    if(prefixInfo.combined)
    {
        sb << "__magic_type(TextureSamplerType," << int(flavor) << ")\n";
        sb << "__intrinsic_type(" << (kIROp_TextureSamplerType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
    }
    else
    {
        sb << "__magic_type(TextureType," << int(flavor) << ")\n";
        sb << "__intrinsic_type(" << (kIROp_TextureType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
    }
    sb << "struct ";
    sb << accessInfo.name;
    sb << baseName;
    sb << baseShapeName;
    if (isMultisample) sb << "MS";
    if (isArray) sb << "Array";
    // if (isShadow) sb << "Shadow";
    sb << "\n";

    // The struct body
    {
        BraceScope structBodyScope{i, sb, ";\n"};

        writeQueryFunctions();

        if(baseShape != TextureFlavor::Shape::ShapeCube)
            writeSubscriptFunctions();

        if( !isMultisample )
            writeSampleFunctions();
    }

    writeGatherExtensions();
} // TextureTypeInfo::emitTypeDecl

void TextureTypeInfo::writeQueryFunctions()
{
    static const char* kComponentNames[]{ "x", "y", "z", "w" };

    TextureFlavor::Shape baseShape = base.baseShape;

    char const* samplerStateParam = prefixInfo.combined ? "" : "SamplerState s, ";
    auto access = accessInfo.access;

    if( !isMultisample )
    {
        writeFunc(
            "float",
            "CalculateLevelOfDetail",
            cat(samplerStateParam, "float", base.coordCount, " location"),
            cat("textureQueryLod($p, $2).x"),
            "",
            "",
            ReadNoneMode::Never
        );

        writeFunc(
            "float",
            "CalculateLevelOfDetailUnclamped",
            cat(samplerStateParam, "float", base.coordCount, " location"),
            cat("textureQueryLod($p, $2).y"),
            "",
            "",
            ReadNoneMode::Never
        );
    }

    // `GetDimensions`
    const char* dimParamTypes[] = {"out float ", "out int ", "out uint "};
    const char* dimParamTypesInner[] = { "float", "int", "uint" };
    for (int tid = 0; tid < 3; tid++)
    {
        auto t = dimParamTypes[tid];
        auto rawT = dimParamTypesInner[tid];

        for (int includeMipInfo = 0; includeMipInfo < 2; ++includeMipInfo)
        {
            int sizeDimCount = 0;
            StringBuilder params;
            if (includeMipInfo)
                params << "uint mipLevel, ";

            switch (baseShape)
            {
            case TextureFlavor::Shape::Shape1D:
                params << t << "width";
                sizeDimCount = 1;
                break;

            case TextureFlavor::Shape::Shape2D:
            case TextureFlavor::Shape::ShapeCube:
                params << t << "width,";
                params << t << "height";
                sizeDimCount = 2;
                break;

            case TextureFlavor::Shape::Shape3D:
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
                glsl << "(";

                int aa = 1;
                String lodStr = ", 0";
                if (includeMipInfo)
                {
                    int mipLevelArg = aa++;
                    lodStr = ", int($";
                    lodStr.append(mipLevelArg);
                    lodStr.append(")");
                }

                String opStr = " = textureSize($0" + lodStr;
                switch (access)
                {
                case SLANG_RESOURCE_ACCESS_READ_WRITE:
                case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
                    opStr = " = imageSize($0";
                    break;

                default:
                    break;
                }

                int cc = 0;
                switch (baseShape)
                {
                case TextureFlavor::Shape::Shape1D:
                    glsl << "($" << aa++ << opStr << ")";
                    if (isArray)
                    {
                        glsl << ".x";
                    }
                    glsl << ")";
                    cc = 1;
                    break;

                case TextureFlavor::Shape::Shape2D:
                case TextureFlavor::Shape::ShapeCube:
                    glsl << "($" << aa++ << opStr << ").x)";
                    glsl << ", ($" << aa++ << opStr << ").y)";
                    cc = 2;
                    break;

                case TextureFlavor::Shape::Shape3D:
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


                glsl << ")";
            }

            StringBuilder spirv;
            {
                spirv << "OpCapability ImageQuery; ";
                spirv << "%vecSize:$$uint";
                if (sizeDimCount > 1) spirv << sizeDimCount;
                spirv << " = ";
                if (isMultisample)
                    spirv << "OpImageQuerySize $this;";
                else
                    spirv << "OpImageQuerySizeLod $this $0;";
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
                case TextureFlavor::Shape::Shape1D:
                    extractSizeComponent(0, "width");
                    break;

                case TextureFlavor::Shape::Shape2D:
                case TextureFlavor::Shape::ShapeCube:
                    extractSizeComponent(0, "width");
                    extractSizeComponent(1, "height");
                    break;

                case TextureFlavor::Shape::Shape3D:
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
                    spirv << "%_sampleCount : $$uint = OpImageQuerySamples $this;";
                    convertAndStore(UnownedStringSlice("_sampleCount"), "sampleCount");
                }

                if (includeMipInfo)
                {
                    spirv << "%_levelCount : $$uint = OpImageQueryLevels $this;";
                    convertAndStore(UnownedStringSlice("_levelCount"), "numberOfLevels");
                }
            }

            sb << "    __glsl_version(450)\n";
            sb << "    __glsl_extension(GL_EXT_samplerless_texture_functions)\n";
            writeFunc(
                "void",
                "GetDimensions",
                params,
                glsl,
                spirv,
                "",
                ReadNoneMode::Always);
        }
    }

    // `GetSamplePosition()`
    if( isMultisample )
    {
        writeFunc("float2", "GetSamplePosition", "int s", "", "", "", ReadNoneMode::Never);
    }

    // `Load()`

    if( base.coordCount + isArray < 4 )
    {
        // The `Load()` operation on an ordinary `Texture2D` takes
        // an `int3` for the location, where `.xy` holds the texel
        // coordinates, and `.z` holds the mip level to use.
        //
        // The third coordinate for mip level is absent in
        // `Texure2DMS.Load()` and `RWTexture2D.Load`. This pattern
        // is repreated for all the other texture shapes.
        //
        bool needsMipLevel = !isMultisample && (access == SLANG_RESOURCE_ACCESS_READ);

        int loadCoordCount = base.coordCount + isArray + (needsMipLevel?1:0);

        char const* glslFuncName = (access == SLANG_RESOURCE_ACCESS_READ) ? "texelFetch" : "imageLoad";

        // When translating to GLSL, we need to break apart the `location` argument.
        //
        // TODO: this should realy be handled by having this member actually get lowered!
        static const char* kGLSLLoadCoordsSwizzle[] = { "", "", "x", "xy", "xyz", "xyzw" };
        static const char* kGLSLLoadLODSwizzle[]    = { "", "", "y", "z", "w", "error" };

        // TODO: The GLSL translations here only handle the read-only texture
        // cases (stuff that lowers to `texture*` in GLSL) and not the stuff
        // that lowers to `image*`.
        //
        // At some point it may make sense to separate the read-only and
        // `RW`/`RasterizerOrdered` cases here rather than try to share code.

        // CUDA
        StringBuilder cudaBuilder;
        if(!isMultisample)
        {
            if (access == SLANG_RESOURCE_ACCESS_READ_WRITE)
            {
                const int coordCount = base.coordCount;
                const int vecCount = coordCount + int(isArray);

                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    cudaBuilder << "surf" << coordCount << "D";
                    if (isArray)
                    {
                        cudaBuilder << "Layered";
                    }
                    cudaBuilder << "read";
                    cudaBuilder << "<$T0>($0";
                    for (int j = 0; j < coordCount; ++j)
                    {
                        cudaBuilder << ", ($1)";
                        if (vecCount > 1)
                        {
                            cudaBuilder << '.' << char(j + 'x');
                        }

                        // Surface access is *byte* addressed in x in CUDA
                        if (j == 0)
                        {
                            cudaBuilder << " * $E";
                        }
                    }
                    if (isArray)
                    {
                        cudaBuilder << ", int(($1)." << char(coordCount + 'x') << ")";
                    }
                    cudaBuilder << ", SLANG_CUDA_BOUNDARY_MODE)";
                }
                else
                {
                    cudaBuilder << "__target_intrinsic(cuda, \"surfCubemap";
                    if (isArray)
                    {
                        cudaBuilder << "Layered";
                    }
                    cudaBuilder << "read";

                    // Surface access is *byte* addressed in x in CUDA
                    cudaBuilder << "<$T0>($0, ($1).x * $E, ($1).y, ($1).z";
                    if (isArray)
                    {
                        cudaBuilder << ", int(($1).w)";
                    }
                    cudaBuilder << ", SLANG_CUDA_BOUNDARY_MODE)";
                }
            }
            else if (access == SLANG_RESOURCE_ACCESS_READ)
            {
                // We can allow this on Texture1D
                if( baseShape == TextureFlavor::Shape::Shape1D && isArray == false)
                {
                    cudaBuilder << "tex1Dfetch<$T0>($0, ($1).x)";
                }
            }
        }

        // SPIRV
        auto getSpirvIntrinsic = [&](bool hasSampleIndex, bool hasOffset)
            {
                StringBuilder spirv;
                spirv << "%lod:$$int = OpCompositeExtract $location " << base.coordCount + isArray << "; ";
                spirv << "%coord:$$int" << base.coordCount + isArray << " = OpVectorShuffle $location $location ";
                for (int i = 0; i < base.coordCount + isArray; i++)
                    spirv << " " << i;
                spirv << "; ";
                spirv << "%sampled:__sampledType(T) = ";
                if (access == SLANG_RESOURCE_ACCESS_READ_WRITE)
                    spirv << "OpImageRead";
                else
                    spirv << "OpImageFetch";
                spirv << " $this %coord ";
                uint32_t operandMask = 0;
                if (!hasSampleIndex)
                    operandMask |= SpvImageOperandsLodMask;
                if (hasOffset)
                    operandMask |= SpvImageOperandsConstOffsetMask;
                if (hasSampleIndex)
                    operandMask |= SpvImageOperandsSampleMask;
                spirv << operandMask << " ";
                if (operandMask & SpvImageOperandsLodMask)
                    spirv << " %lod";
                if (operandMask & SpvImageOperandsConstOffsetMask)
                    spirv << " $offset";
                if (operandMask & SpvImageOperandsSampleMask)
                    spirv << " $sampleIndex";
                spirv << ";";
                spirv << i << "__truncate $$T result __sampledType(T) %sampled;";
                return spirv.produceString();
            };

        sb << i << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
        writeFunc(
            "T",
            "Load",
            cat("int", loadCoordCount, " location", isMultisample ? ", int sampleIndex" : ""),
            isMultisample ? cat("$c", glslFuncName, "($0, $1, $2)$z")
            : needsMipLevel ? cat(
                "$c",
                glslFuncName,
                "($0, ($1).",
                kGLSLLoadCoordsSwizzle[loadCoordCount],
                ", ($1).",
                kGLSLLoadLODSwizzle[loadCoordCount],
                ")$z")
            : cat("$c", glslFuncName, "($0, $1)$z"),
            getSpirvIntrinsic(isMultisample, false),
            cudaBuilder
        );

        glslFuncName = (access == SLANG_RESOURCE_ACCESS_READ) ? "texelFetchOffset" : "imageLoad";
        sb << i << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
        writeFunc(
            "T",
            "Load",
            cat(
                "int", loadCoordCount, " location",
                isMultisample ? ", int sampleIndex" : "",
                ", constexpr int", base.coordCount, " offset"
            ),
            isMultisample ? cat("$c", glslFuncName, "($0, $0, $1, $2)$z")
                : needsMipLevel ? cat(
                    "$c", glslFuncName, "($0, ($1).", kGLSLLoadCoordsSwizzle[loadCoordCount],
                    ", ($1).", kGLSLLoadLODSwizzle[loadCoordCount],
                    ", $2)$z")
                : cat("$c", glslFuncName, "($0, $1, 0, $2)$z"),
            getSpirvIntrinsic(isMultisample, true)
        );

        writeFunc(
            "T",
            "Load",
            cat(
                "int", loadCoordCount, " location",
                isMultisample ? ", int sampleIndex" : "",
                ", constexpr int", base.coordCount, " offset",
                ", out uint status"
            )
        );
    }
}

static String spirvReadIntrinsic(SlangResourceAccess access)
{
    StringBuilder spirvBuilder;
    const char* i = "                    ";
    switch (access)
    {
    case SLANG_RESOURCE_ACCESS_NONE:
    case SLANG_RESOURCE_ACCESS_READ:
        spirvBuilder << i << "%sampled : __sampledType(T) = OpImageFetch $this $location;\n";
        spirvBuilder << i << "__truncate $$T result __sampledType(T) %sampled;";
        break;

    default:
        spirvBuilder << i << "%sampled : __sampledType(T) = OpImageRead $this $location;\n";
        spirvBuilder << i << "__truncate $$T result __sampledType(T) %sampled;";
        break;
    }
    return spirvBuilder;
}

static String spirvWriteIntrinsic()
{
    StringBuilder spirvBuilder;
    const char* i = "                    ";
    spirvBuilder << i << "OpImageWrite $this $location $newValue;";
    return spirvBuilder;
}

void TextureTypeInfo::writeSubscriptFunctions()
{
    TextureFlavor::Shape baseShape = base.baseShape;
    auto access = accessInfo.access;

    int N = base.coordCount + isArray;

    char const* uintNs[] = { "", "uint", "uint2", "uint3", "uint4" };
    char const* ivecNs[] = {  "", "int", "ivec2", "ivec3", "ivec4" };

    auto uintN = uintNs[N];
    auto ivecN = ivecNs[N];

    // subscript operator
    sb << i << "__subscript(" << uintN << " location) -> T\n";
    BraceScope subscriptScope{i, sb};

    // !!!!!!!!!!!!!!!!!!!! get !!!!!!!!!!!!!!!!!!!!!!!

    // GLSL/SPIR-V distinguishes sampled vs. non-sampled images
    StringBuilder glslBuilder;
    {
        switch( access )
        {
        case SLANG_RESOURCE_ACCESS_NONE:
        case SLANG_RESOURCE_ACCESS_READ:
            sb << i << "__glsl_extension(GL_EXT_samplerless_texture_functions)\n";
            glslBuilder << "$ctexelFetch($0, " << ivecN << "($1)";
            if( !isMultisample )
            {
                glslBuilder << ", 0";
            }
            else
            {
                // TODO: how to handle passing through sample index?
                glslBuilder << ", 0";
            }
            break;

        default:
            glslBuilder << "$cimageLoad($0, " << ivecN << "($1)";
            if( isMultisample )
            {
                // TODO: how to handle passing through sample index?
                glslBuilder << ", 0";
            }
            break;
        }
        glslBuilder << ")$z";
    }

    // CUDA
    StringBuilder cudaBuilder;
    {
        if (access == SLANG_RESOURCE_ACCESS_READ_WRITE)
        {
            const int coordCount = base.coordCount;
            const int vecCount = coordCount + int(isArray);

            cudaBuilder << "surf";
            if( baseShape != TextureFlavor::Shape::ShapeCube )
            {
                cudaBuilder << coordCount << "D";
            }
            else
            {
                cudaBuilder << "Cubemap";
            }

            cudaBuilder << (isArray ? "Layered" : "");
            cudaBuilder << "read$C<$T0>($0";

            for (int j = 0; j < vecCount; ++j)
            {
                cudaBuilder << ", ($1)";
                if (vecCount > 1)
                {
                    cudaBuilder << '.' << char(j + 'x');
                }
                // Surface access is *byte* addressed in x in CUDA
                if (j == 0)
                {
                    cudaBuilder << " * $E";
                }
            }

            cudaBuilder << ", SLANG_CUDA_BOUNDARY_MODE)";
        }
        else if (access == SLANG_RESOURCE_ACCESS_READ)
        {
            // We can allow this on Texture1D
            if( baseShape == TextureFlavor::Shape::Shape1D && isArray == false)
            {
                cudaBuilder << "tex1Dfetch<$T0>($0, $1)";
            }
        }
    }

    // Output that has get
    writeFuncWithSig(
        "operator[]",
        "get",
        glslBuilder,
        spirvReadIntrinsic(access),
        cudaBuilder
    );

    // !!!!!!!!!!!!!!!!!!!! set !!!!!!!!!!!!!!!!!!!!!!!

    if (!(access == SLANG_RESOURCE_ACCESS_NONE || access == SLANG_RESOURCE_ACCESS_READ))
    {
        // CUDA
        cudaBuilder.clear();
        {
            const int coordCount = base.coordCount;
            const int vecCount = coordCount + int(isArray);

            cudaBuilder << "surf";
            if( baseShape != TextureFlavor::Shape::ShapeCube )
            {
                cudaBuilder << coordCount << "D";
            }
            else
            {
                cudaBuilder << "Cubemap";
            }

            cudaBuilder << (isArray ? "Layered" : "");
            cudaBuilder << "write$C<$T0>($2, $0";
            for (int j = 0; j < vecCount; ++j)
            {
                cudaBuilder << ", ($1)";
                if (vecCount > 1)
                {
                    cudaBuilder << '.' << char(j + 'x');
                }

                // Surface access is *byte* addressed in x in CUDA
                if (j == 0)
                {
                    cudaBuilder << " * $E";
                }
            }

            cudaBuilder << ", SLANG_CUDA_BOUNDARY_MODE)";
        }

        // Set
        sb << i << "[nonmutating]\n";
        writeFuncWithSig(
            "operator[]",
            "set(T newValue)",
            cat("imageStore($0, ", ivecN, "($1), $V2)"),
            spirvWriteIntrinsic(),
            cudaBuilder
        );
    }

    // !!!!!!!!!!!!!!!!!! ref !!!!!!!!!!!!!!!!!!!!!!!!!

    // Depending on the access level of the texture type,
    // we either have just a getter (the default), or both
    // a getter and setter.
    switch( access )
    {
    case SLANG_RESOURCE_ACCESS_NONE:
    case SLANG_RESOURCE_ACCESS_READ:
        break;
    default:
        sb << i << "__intrinsic_op(" << int(kIROp_ImageSubscript) << ") ref;\n";
        break;
    }
}

static String cudaSampleIntrinsic(const bool isArray, const BaseTextureShapeInfo& base, bool sampleLevel)
{
    StringBuilder cudaBuilder;

    TextureFlavor::Shape baseShape = base.baseShape;
    const int coordCount = base.coordCount;

    if( baseShape != TextureFlavor::Shape::ShapeCube )
    {
        cudaBuilder << "tex" << coordCount << "D";
        if (isArray)
            cudaBuilder << "Layered";
        if(sampleLevel)
            cudaBuilder << "Lod";
        cudaBuilder << "<$T0>($0";
        for (int i = 0; i < coordCount; ++i)
        {
            cudaBuilder << ", ($2)";
            cudaBuilder << '.' << "xyzw"[i];
        }
        if (isArray)
            cudaBuilder << ", int(($2)." << char(coordCount + 'x') << ")";
        if(sampleLevel)
            cudaBuilder << ", $3";
        cudaBuilder << ")";
    }
    else
    {
        cudaBuilder << "texCubemap";
        if (isArray)
            cudaBuilder << "Layered";
        if(sampleLevel)
            cudaBuilder << "Lod";
        cudaBuilder << "<$T0>($0, ($2).x, ($2).y, ($2).z";
        if (isArray)
            cudaBuilder << ", int(($2).w)";
        if(sampleLevel)
            cudaBuilder << ", $3";
        cudaBuilder << ")";
    }

    return cudaBuilder;
}

const char* noBias = nullptr;
const char* noLodLevel = nullptr;
const char* noGradX = nullptr;
const char* noGradY = nullptr;
const char* noConstOffset = nullptr;
const char* noMinLod = nullptr;

static String spirvSampleIntrinsic(
    const TextureTypePrefixInfo& prefixInfo,
    const char* bias = nullptr,
    const char* lodLevel = nullptr,
    const char* gradX = nullptr,
    const char* gradY = nullptr,
    const char* constOffset = nullptr,
    const char* minLod = nullptr)
{
    StringBuilder spirvBuilder;
    const char* i = "                ";

    SLANG_ASSERT(!(!gradX ^ !gradY));

    if(minLod)
        spirvBuilder << i << "OpCapability MinLod;\n";

    const char* sampledImage;
    if(prefixInfo.combined)
    {
        sampledImage = "$this";
    }
    else
    {
        const char* sampledImageType = "%sampledImageType";
        sampledImage = "%sampledImage";
        spirvBuilder << i << sampledImageType << " = OpTypeSampledImage $$This;\n";
        spirvBuilder << i << sampledImage << " : " << sampledImageType << " = OpSampledImage $this $s;\n";
    }

    const char* op = lodLevel || gradX ? "OpImageSampleExplicitLod" : "OpImageSampleImplicitLod";
    spirvBuilder << i << "%sampled : __sampledType(T) = " << op << " " << sampledImage << " $location";
    spirvBuilder << " None";
    if(bias)
        spirvBuilder << "|Bias";
    if(lodLevel)
        spirvBuilder << "|Lod";
    if(gradX)
        spirvBuilder << "|Grad";
    if(constOffset)
        spirvBuilder << "|ConstOffset";
    if(minLod)
        spirvBuilder << "|MinLod";

    if(bias)
        spirvBuilder << " $" << bias;
    if(lodLevel)
        spirvBuilder << " $" << lodLevel;
    if(gradX)
        spirvBuilder << " $" << gradX << " $" << gradY;
    if(constOffset)
        spirvBuilder << " $" << constOffset;
    if(minLod)
        spirvBuilder << " $" << minLod;
    spirvBuilder << ";\n";
    spirvBuilder << i << "__truncate $$T result __sampledType(T) %sampled;\n";
    return spirvBuilder;
}

void TextureTypeInfo::writeSampleFunctions()
{
    TextureFlavor::Shape baseShape = base.baseShape;
    char const* samplerStateParam = prefixInfo.combined ? "" : "SamplerState s, ";

    // `Sample()`

    writeFunc(
        "T",
        "Sample",
        cat(samplerStateParam, "float", base.coordCount + isArray, " location"),
        "$ctexture($p, $2)$z",
        spirvSampleIntrinsic(prefixInfo),
        cudaSampleIntrinsic(isArray, base, false)
    );

    if( baseShape != TextureFlavor::Shape::ShapeCube )
    {
        writeFunc(
            "T",
            "Sample",
            cat(samplerStateParam, "float", base.coordCount + isArray, " location, ", "constexpr int", base.coordCount, " offset"),
            "$ctextureOffset($p, $2, $3)$z",
            spirvSampleIntrinsic(prefixInfo, noBias, noLodLevel, noGradX, noGradY, "offset")
        );
    }

    writeFunc(
        "T",
        "Sample",
        cat(
            samplerStateParam,
            "float", base.coordCount + isArray, " location, ",
            baseShape == TextureFlavor::Shape::ShapeCube ? "" : cat("constexpr int", base.coordCount, " offset, "),
            "float clamp"
        ),
        "",
        spirvSampleIntrinsic(
            prefixInfo,
            noBias,
            noLodLevel,
            noGradX,
            noGradY,
            baseShape == TextureFlavor::Shape::ShapeCube ? nullptr : "offset",
            "clamp"
        )
    );

    // SPIR-V todo, use OpImageSparseSampleImplicitLod
    writeFunc(
        "T",
        "Sample",
        cat(
            samplerStateParam,
            "float", base.coordCount + isArray, " location, ",
            baseShape != TextureFlavor::Shape::ShapeCube ? cat("constexpr int", base.coordCount, " offset, ") : "",
            "float clamp, out uint status"
        )
    );

    writeFunc(
        "T",
        "SampleBias",
        cat(
            samplerStateParam,
            "float", base.coordCount + isArray, " location, ",
            "float bias"
        ),
        "$ctexture($p, $2, $3)$z",
        spirvSampleIntrinsic(prefixInfo, "bias")
    );

    if( baseShape != TextureFlavor::Shape::ShapeCube )
    {
        writeFunc(
            "T",
            "SampleBias",
            cat(
                samplerStateParam,
                "float", base.coordCount + isArray, " location, ",
                "float bias, ",
                "constexpr int", base.coordCount, " offset"
            ),
            "$ctextureOffset($p, $2, $3, $4)$z",
            spirvSampleIntrinsic(prefixInfo, "bias", noLodLevel, noGradX, noGradY, "offset")
        );
    }
    int baseCoordCount = base.coordCount;
    int arrCoordCount = baseCoordCount + isArray;
    if (arrCoordCount <= 3)
    {
        // `SampleCmp()` and `SampleCmpLevelZero`

        writeFunc(
            "float",
            "SampleCmp",
            cat(
                "SamplerComparisonState s, ",
                "float", base.coordCount + isArray, " location, ",
                "float compareValue"
            ),
            cat("texture($p, vec", arrCoordCount + 1, "($2, $3))")
        );

        sb << "__glsl_extension(GL_EXT_texture_shadow_lod)\n";
        writeFunc(
            "float",
            "SampleCmpLevelZero",
            cat(
                "SamplerComparisonState s, ",
                "float", base.coordCount + isArray, " location, ",
                "float compareValue"
            ),
            cat("textureLod($p, vec", arrCoordCount + 1, "($2, $3), 0)")
        );
    }

    if( baseShape != TextureFlavor::Shape::ShapeCube )
    {
        // Note(tfoley): MSDN seems confused, and claims that the `offset`
        // parameter for `SampleCmp` is available for everything but 3D
        // textures, while `Sample` and `SampleBias` are consistent in
        // saying they only exclude `offset` for cube maps (which makes
        // sense). I'm going to assume the documentation for `SampleCmp`
        // is just wrong.
        writeFunc(
            "float",
            "SampleCmp",
            cat(
                "SamplerComparisonState s, ",
                "float", base.coordCount + isArray, " location, ",
                "float compareValue, "
                "constexpr int", base.coordCount, " offset"
            ),
            cat("textureOffset($p, vec", arrCoordCount + 1, "($2, $3), $4)")
        );

        sb << "__glsl_extension(GL_EXT_texture_shadow_lod)\n";
        writeFunc(
            "float",
            "SampleCmpLevelZero",
            cat(
                "SamplerComparisonState s, ",
                "float", base.coordCount + isArray, " location, ",
                "float compareValue, "
                "constexpr int", base.coordCount, " offset"
            ),
            cat("textureLodOffset($p, vec", arrCoordCount + 1, "($2, $3), 0, $4)")
        );
    }

    // TODO(JS): Not clear how to map this to CUDA, because in HLSL, the gradient is a vector based on
    // the dimension. On CUDA there is texNDGrad, but it always just takes ddx, ddy.
    // I could just assume 0 for elements not supplied, and ignore z. For now will just leave
    writeFunc(
        "T",
        "SampleGrad",
        cat(
            samplerStateParam,
            "float", base.coordCount + isArray, " location, ",
            "float", base.coordCount, " gradX, ",
            "float", base.coordCount, " gradY, "
        ),
        "$ctextureGrad($p, $2, $3, $4)$z",
        spirvSampleIntrinsic(prefixInfo, noBias, noLodLevel, "gradX", "gradY")
    );

    if( baseShape != TextureFlavor::Shape::ShapeCube )
    {
        writeFunc(
            "T",
            "SampleGrad",
            cat(
                samplerStateParam,
                "float", base.coordCount + isArray, " location, ",
                "float", base.coordCount, " gradX, ",
                "float", base.coordCount, " gradY, ",
                "constexpr int", base.coordCount, " offset "
            ),
            "$ctextureGradOffset($p, $2, $3, $4, $5)$z",
            spirvSampleIntrinsic(prefixInfo, noBias, noLodLevel, "gradX", "gradY", "offset")
        );

        sb << i << "__glsl_extension(GL_ARB_sparse_texture_clamp)\n";
        writeFunc(
            "T",
            "SampleGrad",
            cat(
                samplerStateParam,
                "float", base.coordCount + isArray, " location, ",
                "float", base.coordCount, " gradX, ",
                "float", base.coordCount, " gradY, ",
                "constexpr int", base.coordCount, " offset, ",
                "float lodClamp"
            ),
            "$ctextureGradOffsetClampARB($p, $2, $3, $4, $5, $6)$z",
            spirvSampleIntrinsic(prefixInfo, noBias, noLodLevel, "gradX", "gradY", "offset", "lodClamp")
        );
    }

    // `SampleLevel`

    writeFunc(
        "T",
        "SampleLevel",
        cat(
            samplerStateParam,
            "float", base.coordCount + isArray, " location, ",
            "float level"
        ),
        "$ctextureLod($p, $2, $3)$z",
        spirvSampleIntrinsic(prefixInfo, noBias, "level"),
        cudaSampleIntrinsic(isArray, base, true)
    );

    if( baseShape != TextureFlavor::Shape::ShapeCube )
    {
        writeFunc(
            "T",
            "SampleLevel",
            cat(
                samplerStateParam,
                "float", base.coordCount + isArray, " location, ",
                "float level, ",
                "constexpr int", base.coordCount, " offset"
            ),
            "$ctextureLodOffset($p, $2, $3, $4)$z",
            spirvSampleIntrinsic(prefixInfo, noBias, "level", noGradX, noGradY, "offset")
        );
    }
}

void TextureTypeInfo::writeGatherExtensions()
{
    char const* baseName = prefixInfo.name;
    char const* baseShapeName = base.shapeName;

    auto access = accessInfo.access;

    bool isReadOnly = (access == SLANG_RESOURCE_ACCESS_READ);

    char const* samplerStateParam = prefixInfo.combined ? "" : "SamplerState s, ";

    // `Gather*()` operations are handled via an `extension` declaration,
    // because this lets us capture the element type of the texture.
    //
    // TODO: longer-term there should be something like a `TextureElementType`
    // interface, that both scalars and vectors implement, that then exposes
    // a `Scalar` associated type, and `Gather` can return `vector<T.Scalar, 4>`.
    //
    static const struct {
        char const* genericPrefix;
        char const* elementType;
        char const* outputType;
    } kGatherExtensionCases[] = {
        { "__generic<T, let N : int>", "vector<T,N>", "vector<T, 4>" },
        { "", "float", "vector<float, 4>" },
        { "", "int" , "vector<int, 4>"},
        { "", "uint", "vector<uint, 4>"},

        // TODO: need a case here for scalars `T`, but also
        // need to ensure that case doesn't accidentally match
        // for `T = vector<...>`, which requires actual checking
        // of constraints on generic parameters.
    };
    for(auto cc : kGatherExtensionCases)
    {
        // TODO: this should really be an `if` around the entire `Gather` logic
        if (isMultisample) break;

        EMIT_LINE_DIRECTIVE();
        sb << cc.genericPrefix << " __extension ";
        sb << accessInfo.name;
        sb << baseName;
        sb << baseShapeName;
        if (isArray) sb << "Array";
        sb << "<" << cc.elementType << " >";
        sb << "\n{\n";

        // `Gather`
        // (tricky because it returns a 4-vector of the element type
        // of the texture components...)
        //
        // TODO: is it actually correct to restrict these so that, e.g.,
        // `GatherAlpha()` isn't allowed on `Texture2D<float3>` because
        // it nominally doesn't have an alpha component?
        static const struct {
            int componentIndex;
            char const* componentName;
        } kGatherComponets[] = {
            { 0, "" },
            { 0, "Red" },
            { 1, "Green" },
            { 2, "Blue" },
            { 3, "Alpha" },
        };
        enum Cmp
        { NotCmp,
          Cmp
        };

        for(auto cmp : {NotCmp, Cmp})
        for(auto kk : kGatherComponets)
        {
            auto samplerOrComparisonSampler = cmp == Cmp ? "SamplerComparisonState s, " : samplerStateParam;

            auto componentIndex = kk.componentIndex;
            auto componentName = kk.componentName;

            auto outputType = cc.outputType;

            const auto cmpName          = cmp == Cmp ? "Cmp" : "";
            const auto cmpValueParam    = cmp == Cmp ? "float compareValue, " : "";
            const auto cmpValueParamEnd = cmp == Cmp ? ", float compareValue" : "";
            const auto supportsGLSL     = componentIndex == 0 || cmp == NotCmp;

            EMIT_LINE_DIRECTIVE();

            if(supportsGLSL)
            {
                if(cmp == Cmp)
                    sb << "__target_intrinsic(glsl, \"textureGather($p, $2, $3)\")\n";
                else
                    sb << "__target_intrinsic(glsl, \"textureGather($p, $2, " << componentIndex << ")\")\n";
            }
            if (base.coordCount == 2 && cmp == NotCmp)
            {
                // Gather only works on 2D in CUDA without comparison
                // "It is based on the base type of DataType except when readMode is equal to cudaReadModeNormalizedFloat (see Texture Reference API), in which case it is always float4."
                sb << "__target_intrinsic(cuda, \"tex2Dgather<$T0>($0, ($2).x, ($2).y, " << componentIndex << ")\")\n";
            }
            if (isReadOnly)
                sb << "[__readNone]\n";
            sb << outputType << " Gather" << cmpName << componentName << "(" << samplerOrComparisonSampler;
            sb << "float" << base.coordCount + isArray << " location" << cmpValueParamEnd << ");\n";

            if (isReadOnly)
                sb << "[__readNone]\n";
            EMIT_LINE_DIRECTIVE();
            if(supportsGLSL)
            {
                if(cmp == Cmp)
                    sb << "__target_intrinsic(glsl, \"textureGatherOffset($p, $2, $3, $4)\")\n";
                else
                    sb << "__target_intrinsic(glsl, \"textureGatherOffset($p, $2, $3, " << componentIndex << ")\")\n";
            }
            sb << outputType << " Gather" << cmpName << componentName << "(" << samplerOrComparisonSampler;
            sb << "float" << base.coordCount + isArray << " location, ";
            sb << cmpValueParam;
            sb << "constexpr int" << base.coordCount << " offset);\n";

            if (isReadOnly)
                sb << "[__readNone]\n";
            EMIT_LINE_DIRECTIVE();
            sb << outputType << " Gather" << cmpName << componentName << "(" << samplerOrComparisonSampler;
            sb << "float" << base.coordCount + isArray << " location, ";
            sb << cmpValueParam;
            sb << "constexpr int" << base.coordCount << " offset, ";
            sb << "out uint status);\n";

            if (isReadOnly)
                sb << "[__readNone]\n";
            EMIT_LINE_DIRECTIVE();
            if(supportsGLSL)
            {
                if(cmp == Cmp)
                    sb << "__target_intrinsic(glsl, \"textureGatherOffsets($p, $2, $3, ivec" << base.coordCount << "[]($4, $5, $6, $7))\")\n";
                else
                    sb << "__target_intrinsic(glsl, \"textureGatherOffsets($p, $2, ivec" << base.coordCount << "[]($3, $4, $5, $6), " << componentIndex << ")\")\n";
            }
            sb << outputType << " Gather" << cmpName << componentName << "(" << samplerOrComparisonSampler;
            sb << "float" << base.coordCount + isArray << " location, ";
            sb << cmpValueParam;
            sb << "int" << base.coordCount << " offset1, ";
            sb << "int" << base.coordCount << " offset2, ";
            sb << "int" << base.coordCount << " offset3, ";
            sb << "int" << base.coordCount << " offset4);\n";

            if (isReadOnly)
                sb << "[__readNone]\n";
            EMIT_LINE_DIRECTIVE();
            sb << outputType << " Gather" << cmpName << componentName << "(" << samplerOrComparisonSampler;
            sb << "float" << base.coordCount + isArray << " location, ";
            sb << cmpValueParam;
            sb << "int" << base.coordCount << " offset1, ";
            sb << "int" << base.coordCount << " offset2, ";
            sb << "int" << base.coordCount << " offset3, ";
            sb << "int" << base.coordCount << " offset4, ";
            sb << "out uint status);\n";
        }

        EMIT_LINE_DIRECTIVE();
        sb << "\n}\n";
    }
}

}
