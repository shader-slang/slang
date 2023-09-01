#include "slang-stdlib-textures.h"

#define EMIT_LINE_DIRECTIVE() sb << "#line " << (__LINE__+1) << " \"slang-stdlib-textures.h\"\n"

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
        SLANG_ASSERT(i != spaces);
        sb << i << "{\n";
        i -= indentWidth;
    }
    ~BraceScope()
    {
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
            sb << i << "__intrinsic_asm \"." << glsl << "\";\n";
        }
        if(cuda.getLength())
        {
            sb << i << "case cuda:\n";
            sb << i << "__intrinsic_asm \"." << cuda << "\";\n";
        }
        if(spirv.getLength())
        {
            sb << i << "case spirv:\n";
            sb << i << "return spirv_asm\n";
            BraceScope spirvScope{i, sb, ";\n"};
            sb << i << spirv << "\n";
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
    const String& cuda,
    const String& spirv,
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
    const String& cuda,
    const String& spirv,
    const ReadNoneMode readNoneMode)
{
    writeFuncWithSig(
        funcName,
        cat(returnType, " ", funcName, "(", params, ")"),
        glsl,
        cuda,
        spirv,
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
    for(auto t : dimParamTypes)
    for(int includeMipInfo = 0; includeMipInfo < 2; ++includeMipInfo)
    {
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
            switch( access )
            {
            case SLANG_RESOURCE_ACCESS_READ_WRITE:
            case SLANG_RESOURCE_ACCESS_RASTER_ORDERED:
                opStr = " = imageSize($0";
                break;

            default:
                break;
            }


            int cc = 0;
            switch(baseShape)
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

            if(isArray)
            {
                glsl << ", ($" << aa++ << opStr << ")." << kComponentNames[cc] << ")";
            }

            if(isMultisample)
            {
                glsl << ", ($" << aa++ << " = textureSamples($0))";
            }

            if (includeMipInfo)
            {
                glsl << ", ($" << aa++ << " = textureQueryLevels($0))";
            }


            glsl << ")";
        }

        StringBuilder params;
        if(includeMipInfo)
            params << "uint mipLevel, ";

        switch(baseShape)
        {
        case TextureFlavor::Shape::Shape1D:
            params << t << "width";
            break;

        case TextureFlavor::Shape::Shape2D:
        case TextureFlavor::Shape::ShapeCube:
            params << t << "width,";
            params << t << "height";
            break;

        case TextureFlavor::Shape::Shape3D:
            params << t << "width,";
            params << t << "height,";
            params << t << "depth";
            break;

        default:
            assert(!"unexpected");
            break;
        }

        if(isArray)
        {
            params << ", " << t << "elements";
        }

        if(isMultisample)
        {
            params << ", " << t << "sampleCount";
        }

        if(includeMipInfo)
            params << ", " << t << "numberOfLevels";

        sb << "    __glsl_version(450)\n";
        sb << "    __glsl_extension(GL_EXT_samplerless_texture_functions)\n";
        writeFunc(
            "void",
            "GetDimensions",
            params,
            glsl,
            "",
            "",
            ReadNoneMode::Always);
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
                    for (int i = 0; i < coordCount; ++i)
                    {
                        cudaBuilder << ", ($1)";
                        if (vecCount > 1)
                        {
                            cudaBuilder << '.' << char(i + 'x');
                        }

                        // Surface access is *byte* addressed in x in CUDA
                        if (i == 0)
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
                : cat("$c", glslFuncName, "($0, $1, 0, $2)$z")
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
            sb << i << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
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

            for (int i = 0; i < vecCount; ++i)
            {
                cudaBuilder << ", ($1)";
                if (vecCount > 1)
                {
                    cudaBuilder << '.' << char(i + 'x');
                }
                // Surface access is *byte* addressed in x in CUDA
                if (i == 0)
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
    writeFuncWithSig(".operator[]", "get", glslBuilder, cudaBuilder);

    // !!!!!!!!!!!!!!!!!!!! set !!!!!!!!!!!!!!!!!!!!!!!

    if (!(access == SLANG_RESOURCE_ACCESS_NONE || access == SLANG_RESOURCE_ACCESS_READ))
    {
        // GLSL
        sb << i << "__target_intrinsic(glsl, \"imageStore($0, " << ivecN << "($1), $V2)\")\n";

        // CUDA
        {
            const int coordCount = base.coordCount;
            const int vecCount = coordCount + int(isArray);

            sb << i << "__target_intrinsic(cuda, \"surf";
            if( baseShape != TextureFlavor::Shape::ShapeCube )
            {
                sb << coordCount << "D";
            }
            else
            {
                sb << "Cubemap";
            }

            sb << (isArray ? "Layered" : "");
            sb << "write$C<$T0>($2, $0";
            for (int i = 0; i < vecCount; ++i)
            {
                sb << ", ($1)";
                if (vecCount > 1)
                {
                    sb << '.' << char(i + 'x');
                }

                // Surface access is *byte* addressed in x in CUDA
                if (i == 0)
                {
                    sb << " * $E";
                }
            }

            sb << ", SLANG_CUDA_BOUNDARY_MODE)\")\n";
        }

        // Set
        sb << i << "[nonmutating]\n" << i << "set;\n";
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

void TextureTypeInfo::writeSampleFunctions()
{
    TextureFlavor::Shape baseShape = base.baseShape;
    char const* samplerStateParam = prefixInfo.combined ? "" : "SamplerState s, ";

    // `Sample()`

    // CUDA
    StringBuilder cudaBuilder;
    {
        const int coordCount = base.coordCount;
        const int vecCount = coordCount + int(isArray);

        if( baseShape != TextureFlavor::Shape::ShapeCube )
        {
            cudaBuilder << "tex" << coordCount << "D";
            if (isArray)
            {
                cudaBuilder << "Layered";
            }
            cudaBuilder << "<$T0>($0";
            for (int i = 0; i < coordCount; ++i)
            {
                cudaBuilder << ", ($2)";
                if (vecCount > 1)
                {
                    cudaBuilder << '.' << char(i + 'x');
                }
            }
            if (isArray)
            {
                cudaBuilder << ", int(($2)." << char(coordCount + 'x') << ")";
            }
            cudaBuilder << ")";
        }
        else
        {
            cudaBuilder << "texCubemap";
            if (isArray)
            {
                cudaBuilder << "Layered";
            }
            cudaBuilder << "<$T0>($0, ($2).x, ($2).y, ($2).z";
            if (isArray)
            {
                cudaBuilder << ", int(($2).w)";
            }
            cudaBuilder << ")";
        }
    }

    writeFunc(
        "T",
        "Sample",
        cat(samplerStateParam, "float", base.coordCount + isArray, " location"),
        "$ctexture($p, $2)$z",
        cudaBuilder
    );

    if( baseShape != TextureFlavor::Shape::ShapeCube )
    {
        writeFunc(
            "T",
            "Sample",
            cat(samplerStateParam, "float", base.coordCount + isArray, " location, ", "constexpr int", base.coordCount, " offset"),
            "$ctextureOffset($p, $2, $3)$z"
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
        )
    );

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
        "$ctexture($p, $2, $3)$z"
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
            "$ctextureOffset($p, $2, $3, $4)$z"
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
        "$ctextureGrad($p, $2, $3, $4)$z"
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
            "$ctextureGradOffset($p, $2, $3, $4, $5)$z"
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
            "$ctextureGradOffsetClampARB($p, $2, $3, $4, $5, $6)$z"
        );
    }

    // `SampleLevel`


    // CUDA
    cudaBuilder.clear();
    {
        const int coordCount = base.coordCount;
        const int vecCount = coordCount + int(isArray);

        if( baseShape != TextureFlavor::Shape::ShapeCube )
        {
            cudaBuilder << "tex" << coordCount << "D";
            if (isArray)
            {
                cudaBuilder << "Layered";
            }
            cudaBuilder << "Lod<$T0>($0";
            for (int i = 0; i < coordCount; ++i)
            {
                cudaBuilder << ", ($2)";
                if (vecCount > 1)
                {
                    cudaBuilder << '.' << char(i + 'x');
                }
            }
            if (isArray)
            {
                cudaBuilder << ", int(($2)." << char(coordCount + 'x') << ")";
            }
            cudaBuilder << ", $3)";
        }
        else
        {
            cudaBuilder << "texCubemap";
            if (isArray)
            {
                cudaBuilder << "Layered";
            }
            cudaBuilder << "Lod<$T0>($0, ($2).x, ($2).y, ($2).z";
            if (isArray)
            {
                cudaBuilder << ", int(($2).w)";
            }
            cudaBuilder << ", $3)";
        }
    }

    // SPIR-V
    const auto spirv = false && prefixInfo.combined ? R"(
        %sampledImageType = OpTypeSampledImage $$This;
        %sampledImage : %sampledImageType = OpSampledImage $this $s;
        %sampled : __sampledType(T) = OpImageSampleExplicitLod %sampledImage $location Lod $level;
        __truncate $$T result __sampledType(T) %sampled;
    )" : "";

    writeFunc(
        "T",
        "SampleLevel",
        cat(
            samplerStateParam,
            "float", base.coordCount + isArray, " location, ",
            "float level"
        ),
        "$ctextureLod($p, $2, $3)$z",
        cudaBuilder,
        UnownedStringSlice{spirv}
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
            "$ctextureLodOffset($p, $2, $3, $4)$z"
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
