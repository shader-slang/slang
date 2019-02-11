// Slang `core` library

// Aliases for base types
typedef half float16_t;
typedef float float32_t;
typedef double float64_t;

typedef int int32_t;
typedef uint uint32_t;


// Modifier for variables that must resolve to compile-time constants
// as part of translation.
syntax constexpr : ConstExprModifier;

// Modifier for variables that should have writes be made
// visible at the global-memory scope
syntax globallycoherent : GloballyCoherentModifier;

// A type that can be used as an operand for builtins
interface __BuiltinType {}

// A type that can be used for arithmetic operations
interface __BuiltinArithmeticType : __BuiltinType {}

// A type that logically has a sign (positive/negative/zero)
interface __BuiltinSignedArithmeticType : __BuiltinArithmeticType {}

// A type that can represent integers
interface __BuiltinIntegerType : __BuiltinArithmeticType
{}

// A type that can represent non-integers
interface __BuiltinRealType : __BuiltinArithmeticType {}

// A type that uses a floating-point representation
interface __BuiltinFloatingPointType : __BuiltinRealType, __BuiltinSignedArithmeticType
{
    // A builtin floating-point type must have an initializer that takes
    // a floating-point value...
    __init(float value);
}

// A type resulting from an `enum` declaration.
__magic_type(EnumTypeType)
interface __EnumType
{
    // The type of tags for this `enum`
    //
    // Note: using `__Tag` instead of `Tag` to avoid any
    // conflict if a user had an `enum` case called `Tag`
    associatedtype __Tag : __BuiltinIntegerType;
};

// A type resulting from an `enum` declaration
// with the `[flags]` attribute.
interface __FlagsEnumType : __EnumType
{
};

__generic<T,U> __intrinsic_op(Sequence) U operator,(T left, U right);

__generic<T> __intrinsic_op(select) T operator?:(bool condition, T ifTrue, T ifFalse);
__generic<T, let N : int> __intrinsic_op(select) vector<T,N> operator?:(vector<bool,N> condition, vector<T,N> ifTrue, vector<T,N> ifFalse);

${{{{
// We are going to use code generation to produce the
// declarations for all of our base types.

static const int kBaseTypeCount = sizeof(kBaseTypes) / sizeof(kBaseTypes[0]);
for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
    EMIT_LINE_DIRECTIVE();
    sb << "__builtin_type(" << int(kBaseTypes[tt].tag) << ") struct " << kBaseTypes[tt].name;

    // Declare interface conformances for this type

    sb << "\n    : __BuiltinType\n";

    switch (kBaseTypes[tt].tag)
    {
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
        sb << "\n    , __BuiltinFloatingPointType\n";
        sb << "\n    ,  __BuiltinRealType\n";
        ; // fall through to:
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
        sb << "\n    ,  __BuiltinSignedArithmeticType\n";
        ; // fall through to:
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
        sb << "\n    ,  __BuiltinArithmeticType\n";
        ; // fall through to:
    case BaseType::Bool:
        sb << "\n    ,  __BuiltinType\n";
        break;

    default:
        break;
    }

    sb << "\n{\n";


    // Declare initializers to convert from various other types
    for (int ss = 0; ss < kBaseTypeCount; ++ss)
    {
        // Don't allow conversion from `void`
        if (kBaseTypes[ss].tag == BaseType::Void)
            continue;

        // We need to emit a modifier so that the semantic-checking
        // layer will know it can use these operations for implicit
        // conversion.
        ConversionCost conversionCost = getBaseTypeConversionCost(
            kBaseTypes[tt],
            kBaseTypes[ss]);

        EMIT_LINE_DIRECTIVE();
        sb << "__implicit_conversion(" << conversionCost << ")\n";

        EMIT_LINE_DIRECTIVE();
        sb << "__init(" << kBaseTypes[ss].name << " value);\n";
    }

    // If this is a basic integer type, then define explicit
    // initializers that take a value of an `enum` type.
    //
    // TODO: This should actually be restricted, so that this
    // only applies `where T.__Tag == Self`, but we don't have
    // the needed features in our type system to implement
    // that constraint right now.
    //
    switch (kBaseTypes[tt].tag)
    {
        // TODO: should this cover the full gamut of integer types?
    case BaseType::Int:
    case BaseType::UInt:
}}}}
        __generic<T:__EnumType>
        __init(T value);
${{{{
        break;

    default:
        break;
    }

    sb << "};\n";
}

// Declare built-in pointer type
// (eventually we can have the traditional syntax sugar for this)
}}}}

__generic<T>
__magic_type(PtrType)
__intrinsic_type($(kIROp_PtrType))
struct Ptr
{};

__generic<T>
__magic_type(OutType)
__intrinsic_type($(kIROp_OutType))
struct Out
{};

__generic<T>
__magic_type(InOutType)
__intrinsic_type($(kIROp_InOutType))
struct InOut
{};

__generic<T>
__magic_type(RefType)
__intrinsic_type($(kIROp_RefType))
struct Ref
{};

__magic_type(StringType)
__intrinsic_type($(kIROp_StringType))
struct String
{};

${{{{
// Declare vector and matrix types

sb << "__generic<T = float, let N : int = 4> __magic_type(Vector) struct vector\n{\n";
sb << "    typedef T Element;\n";

// Declare initializer taking a single scalar of the elemnt type
sb << "    __implicit_conversion(" << kConversionCost_ScalarToVector << ")\n";
sb << "    __intrinsic_op(" << kIROp_constructVectorFromScalar << ")\n";
sb << "    __init(T value);\n";

// Allow initialization from same type
sb << "    __init(vector<T,N> value);\n";

sb << "};\n";
}}}}

__generic<T = float, let R : int = 4, let C : int = 4>
__magic_type(Matrix)
struct matrix {};

${{{{
static const struct {
    char const* name;
    char const* glslPrefix;
} kTypes[] =
{
    {"half",        "f16"},
    {"float",       ""},
    {"double",      "d"},

    {"float16_t",   "f16"},
    {"float32_t",   "f32"},
    {"float64_t",   "f64"},

    {"int8_t",      "i8"},
    {"int16_t",     "i16"},
    {"int32_t",     "i32"},
    {"int",         "i"},
    {"int64_t",     "i64"},

    {"uint8_t",     "u8"},
    {"uint16_t",    "u16"},
    {"uint32_t",    "u32"},
    {"uint",        "u"},
    {"uint64_t",    "u64"},

    {"bool",        "b"},
};
static const int kTypeCount = sizeof(kTypes) / sizeof(kTypes[0]);

for (int tt = 0; tt < kTypeCount; ++tt)
{
    // Declare HLSL vector types
    for (int ii = 1; ii <= 4; ++ii)
    {
        sb << "typedef vector<" << kTypes[tt].name << "," << ii << "> " << kTypes[tt].name << ii << ";\n";
    }

    // Declare HLSL matrix types
    for (int rr = 2; rr <= 4; ++rr)
    for (int cc = 2; cc <= 4; ++cc)
    {
        sb << "typedef matrix<" << kTypes[tt].name << "," << rr << "," << cc << "> " << kTypes[tt].name << rr << "x" << cc << ";\n";
    }
}

// Declare additional built-in generic types
//        EMIT_LINE_DIRECTIVE();


sb << "__generic<T>\n";
sb << "__intrinsic_type(" << kIROp_ConstantBufferType << ")\n";
sb << "__magic_type(ConstantBuffer) struct ConstantBuffer {};\n";

sb << "__generic<T>\n";
sb << "__intrinsic_type(" << kIROp_TextureBufferType << ")\n";
sb << "__magic_type(TextureBuffer) struct TextureBuffer {};\n";

sb << "__generic<T>\n";
sb << "__intrinsic_type(" << kIROp_ParameterBlockType << ")\n";
sb << "__magic_type(ParameterBlockType) struct ParameterBlock {};\n";

static const char* kComponentNames[]{ "x", "y", "z", "w" };
static const char* kVectorNames[]{ "", "x", "xy", "xyz", "xyzw" };

// Need to add constructors to the types above
for (int N = 2; N <= 4; ++N)
{
    sb << "__generic<T> __extension vector<T, " << N << ">\n{\n";

    // initialize from N scalars
    sb << "__init(";
    for (int ii = 0; ii < N; ++ii)
    {
        if (ii != 0) sb << ", ";
        sb << "T " << kComponentNames[ii];
    }
    sb << ");\n";

    // Initialize from an M-vector and then scalars
    for (int M = 2; M < N; ++M)
    {
        sb << "__init(vector<T," << M << "> " << kVectorNames[M];
        for (int ii = M; ii < N; ++ii)
        {
            sb << ", T " << kComponentNames[ii];
        }
        sb << ");\n";
    }

    // Initialize from two vectors, of size M and N-M
    for(int M = 2; M <= (N-2); ++M)
    {
        int K = N - M;
        SLANG_ASSERT(K >= 2);

        sb << "__init(vector<T," << M << "> " << kVectorNames[M];
        sb << ", vector<T," << K << "> ";
        for (int ii = 0; ii < K; ++ii)
        {
            sb << kComponentNames[ii];
        }
        sb << ");\n";
    }

    sb << "}\n";
}

// The above extension was generic in the *type* of the vector,
// but explicit in the *size*. We will now declare an extension
// for each builtin type that is generic in the size.
//
for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
    if(kBaseTypes[tt].tag == BaseType::Void) continue;

    sb << "__generic<let N : int> __extension vector<"
        << kBaseTypes[tt].name << ",N>\n{\n";

    for (int ff = 0; ff < kBaseTypeCount; ++ff)
    {
        if(kBaseTypes[ff].tag == BaseType::Void) continue;


        if( tt != ff )
        {
            auto cost = getBaseTypeConversionCost(
                kBaseTypes[tt],
                kBaseTypes[ff]);

			// Implicit conversion from a vector of the same
			// size, but different element type.
            sb << "    __implicit_conversion(" << cost << ")\n";
            sb << "    __init(vector<" << kBaseTypes[ff].name << ",N> value);\n";

			// Constructor to make a vector from a scalar of another type.
            cost += kConversionCost_ScalarToVector;
            sb << "    __implicit_conversion(" << cost << ")\n";
            sb << "    __init(" << kBaseTypes[ff].name << " value);\n";
        }
    }

    sb << "}\n";
}

for( int R = 2; R <= 4; ++R )
for( int C = 2; C <= 4; ++C )
{
    sb << "__generic<T> __extension matrix<T, " << R << "," << C << ">\n{\n";

    // initialize from R*C scalars
    sb << "__init(";
    for( int ii = 0; ii < R; ++ii )
    for( int jj = 0; jj < C; ++jj )
    {
        if ((ii+jj) != 0) sb << ", ";
        sb << "T m" << ii << jj;
    }
    sb << ");\n";

    // Initialize from R C-vectors
    sb << "__init(";
    for (int ii = 0; ii < R; ++ii)
    {
        if(ii != 0) sb << ", ";
        sb << "vector<T," << C << "> row" << ii;
    }
    sb << ");\n";


    // initialize from another matrix of the same size
    //
    // TODO(tfoley): See comment about how this overlaps
    // with implicit conversion, in the `vector` case above
    sb << "__generic<U> __init(matrix<U," << R << ", " << C << ">);\n";

    // initialize from a matrix of larger size
    for(int rr = R; rr <= 4; ++rr)
    for( int cc = C; cc <= 4; ++cc )
    {
        if(rr == R && cc == C) continue;
        sb << "__init(matrix<T," << rr << "," << cc << "> value);\n";
    }

    sb << "}\n";
}

// Declare built-in texture and sampler types



sb << "__magic_type(SamplerState," << int(SamplerStateFlavor::SamplerState) << ")\n";
sb << "__intrinsic_type(" << kIROp_SamplerStateType << ")\n";
sb << "struct SamplerState {};";

sb << "__magic_type(SamplerState," << int(SamplerStateFlavor::SamplerComparisonState) << ")\n";
sb << "__intrinsic_type(" << kIROp_SamplerComparisonStateType << ")\n";
sb << "struct SamplerComparisonState {};";

// TODO(tfoley): Need to handle `RW*` variants of texture types as well...
static const struct {
    char const*			    name;
    TextureFlavor::Shape	baseShape;
    int					    coordCount;
} kBaseTextureTypes[] = {
    { "Texture1D",		TextureFlavor::Shape::Shape1D,	1 },
    { "Texture2D",		TextureFlavor::Shape::Shape2D,	2 },
    { "Texture3D",		TextureFlavor::Shape::Shape3D,	3 },
    { "TextureCube",	TextureFlavor::Shape::ShapeCube,	3 },
};
static const int kBaseTextureTypeCount = sizeof(kBaseTextureTypes) / sizeof(kBaseTextureTypes[0]);


static const struct {
    char const*         name;
    SlangResourceAccess access;
} kBaseTextureAccessLevels[] = {
    { "",                   SLANG_RESOURCE_ACCESS_READ },
    { "RW",                 SLANG_RESOURCE_ACCESS_READ_WRITE },
    { "RasterizerOrdered",  SLANG_RESOURCE_ACCESS_RASTER_ORDERED },
};
static const int kBaseTextureAccessLevelCount = sizeof(kBaseTextureAccessLevels) / sizeof(kBaseTextureAccessLevels[0]);

// Declare the GLSL types here for compatibility...
//
// TODO: The stdlib should include a module that declares the GLSL types, to keep
// them separate...
for (int tt = 0; tt < kBaseTextureTypeCount; ++tt)
{
    char const* name = kBaseTextureTypes[tt].name;
    TextureFlavor::Shape baseShape = kBaseTextureTypes[tt].baseShape;

    for (int isArray = 0; isArray < 2; ++isArray)
    {
        // Arrays of 3D textures aren't allowed
        if (isArray && baseShape == TextureFlavor::Shape::Shape3D) continue;

        for (int isMultisample = 0; isMultisample < 2; ++isMultisample)
        for (int accessLevel = 0; accessLevel < kBaseTextureAccessLevelCount; ++accessLevel)
        {
            auto access = kBaseTextureAccessLevels[accessLevel].access;

            // TODO: any constraints to enforce on what gets to be multisampled?

            unsigned flavor = baseShape;
            if (isArray)		flavor |= TextureFlavor::ArrayFlag;
            if (isMultisample)	flavor |= TextureFlavor::MultisampleFlag;
//                        if (isShadow)		flavor |= TextureFlavor::ShadowFlag;

            flavor |= (access << 8);

            // emit a generic signature
            // TODO: allow for multisample count to come in as well...
            sb << "__generic<T = float4> ";

            sb << "__magic_type(TextureSampler," << int(flavor) << ")\n";
            sb << "__intrinsic_type(" << (kIROp_TextureSamplerType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
            sb << "struct Sampler";
            sb << kBaseTextureAccessLevels[accessLevel].name;
            sb << name;
            if (isMultisample) sb << "MS";
            if (isArray) sb << "Array";
//                        if (isShadow) sb << "Shadow";
            sb << "\n{\n";
            sb << "__specialized_for_target(glsl)\n";
			sb << "__init(";
			sb << kBaseTextureAccessLevels[accessLevel].name;
            sb << name;
            if (isMultisample) sb << "MS";
            if (isArray) sb << "Array";
			sb << "<T> t, ";
			sb << "SamplerState s);\n";
			sb << "};\n";

            sb << "__specialized_for_target(glsl)\n";
            sb << "T texture<T>(Sampler";
			sb << kBaseTextureAccessLevels[accessLevel].name;
            sb << name;
            if (isMultisample) sb << "MS";
            if (isArray) sb << "Array";
			sb << "<T> t, float" << kBaseTextureTypes[tt].coordCount + isArray << " location);\n";
		}
	}
}

for (int tt = 0; tt < kBaseTextureTypeCount; ++tt)
{
    char const* name = kBaseTextureTypes[tt].name;
    TextureFlavor::Shape baseShape = kBaseTextureTypes[tt].baseShape;

    for (int isArray = 0; isArray < 2; ++isArray)
    {
        // Arrays of 3D textures aren't allowed
        if (isArray && baseShape == TextureFlavor::Shape::Shape3D) continue;

        for (int isMultisample = 0; isMultisample < 2; ++isMultisample)
        for (int accessLevel = 0; accessLevel < kBaseTextureAccessLevelCount; ++accessLevel)
        {
            auto access = kBaseTextureAccessLevels[accessLevel].access;

            // TODO: any constraints to enforce on what gets to be multisampled?

            unsigned flavor = baseShape;
            if (isArray)		flavor |= TextureFlavor::ArrayFlag;
            if (isMultisample)	flavor |= TextureFlavor::MultisampleFlag;
//                        if (isShadow)		flavor |= TextureFlavor::ShadowFlag;

            flavor |= (access << 8);

            // emit a generic signature
            // TODO: allow for multisample count to come in as well...
            sb << "__generic<T = float4> ";

            sb << "__magic_type(Texture," << int(flavor) << ")\n";
            sb << "__intrinsic_type(" << (kIROp_TextureType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
            sb << "struct ";
            sb << kBaseTextureAccessLevels[accessLevel].name;
            sb << name;
            if (isMultisample) sb << "MS";
            if (isArray) sb << "Array";
//                        if (isShadow) sb << "Shadow";
            sb << "\n{";

            if( !isMultisample )
            {
                sb << "float CalculateLevelOfDetail(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";

                sb << "float CalculateLevelOfDetailUnclamped(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";
            }

            // `GetDimensions`

            for(int isFloat = 0; isFloat < 2; ++isFloat)
            for(int includeMipInfo = 0; includeMipInfo < 2; ++includeMipInfo)
            {
                {
                    sb << "__glsl_version(450)\n";
                    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                    sb << "__target_intrinsic(glsl, \"(";

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
                        sb << "($" << aa++ << opStr << "))";
                        cc = 1;
                        break;

                    case TextureFlavor::Shape::Shape2D:
                    case TextureFlavor::Shape::ShapeCube:
                        sb << "($" << aa++ << opStr << ").x)";
                        sb << ", ($" << aa++ << opStr << ").y)";
                        cc = 2;
                        break;

                    case TextureFlavor::Shape::Shape3D:
                        sb << "($" << aa++ << opStr << ").x)";
                        sb << ", ($" << aa++ << opStr << ").y)";
                        sb << ", ($" << aa++ << opStr << ").z)";
                        cc = 3;
                        break;

                    default:
                        SLANG_UNEXPECTED("unhandled resource shape");
                        break;
                    }

                    if(isArray)
                    {
                        sb << ", ($" << aa++ << opStr << ")." << kComponentNames[cc] << ")";
                    }

                    if(isMultisample)
                    {
                        sb << ", ($" << aa++ << " = textureSamples($0))";
                    }

                    if (includeMipInfo)
                    {
                        sb << ", ($" << aa++ << " = textureQueryLevels($0))";
                    }


                    sb << ")\")\n";
                }

                char const* t = isFloat ? "out float " : "out uint ";

                sb << "void GetDimensions(";
                if(includeMipInfo)
                    sb << "uint mipLevel, ";

                switch(baseShape)
                {
                case TextureFlavor::Shape::Shape1D:
                    sb << t << "width";
                    break;

                case TextureFlavor::Shape::Shape2D:
                case TextureFlavor::Shape::ShapeCube:
                    sb << t << "width,";
                    sb << t << "height";
                    break;

                case TextureFlavor::Shape::Shape3D:
                    sb << t << "width,";
                    sb << t << "height,";
                    sb << t << "depth";
                    break;

                default:
                    assert(!"unexpected");
                    break;
                }

                if(isArray)
                {
                    sb << ", " << t << "elements";
                }

                if(isMultisample)
                {
                    sb << ", " << t << "sampleCount";
                }

                if(includeMipInfo)
                    sb << ", " << t << "numberOfLevels";

                sb << ");\n";
            }

            // `GetSamplePosition()`
            if( isMultisample )
            {
                sb << "float2 GetSamplePosition(int s);\n";
            }

            // `Load()`

            if( kBaseTextureTypes[tt].coordCount + isArray < 4 )
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

                int loadCoordCount = kBaseTextureTypes[tt].coordCount + isArray + (needsMipLevel?1:0);

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

                if (isMultisample)
                {
                    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                    sb << "__target_intrinsic(glsl, \"texelFetch($0, $1, $3)$z\")\n";
                }
                else
                {
                    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                    sb << "__target_intrinsic(glsl, \"texelFetch($0, ";
                    if( needsMipLevel )
                    {
                        sb << "($1)." << kGLSLLoadCoordsSwizzle[loadCoordCount] << ", ($1)." << kGLSLLoadLODSwizzle[loadCoordCount];
                    }
                    else
                    {
                        sb << "$1, 0";
                    }
                    sb << ")$z\")\n";
                }
                sb << "T Load(";
                sb << "int" << loadCoordCount << " location";
                if(isMultisample)
                {
                    sb << ", int sampleIndex";
                }
                sb << ");\n";

                if (isMultisample)
                {
                    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                    sb << "__target_intrinsic(glsl, \"texelFetchOffset($0, $0, $1, $2)$z\")\n";
                }
                else
                {
                    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                    sb << "__target_intrinsic(glsl, \"texelFetch($0, ";
                    if( needsMipLevel )
                    {
                        sb << "($1)." << kGLSLLoadCoordsSwizzle[loadCoordCount] << ", ($1)." << kGLSLLoadLODSwizzle[loadCoordCount];
                    }
                    else
                    {
                        sb << "$1, 0";
                    }
                    sb << ", $2)$z\")\n";
                }
                sb << "T Load(";
                sb << "int" << loadCoordCount << " location";
                if(isMultisample)
                {
                    sb << ", int sampleIndex";
                }
                sb << ", constexpr int" << kBaseTextureTypes[tt].coordCount << " offset";
                sb << ");\n";


                sb << "T Load(";
                sb << "int" << loadCoordCount << " location";
                if(isMultisample)
                {
                    sb << ", int sampleIndex";
                }
                sb << ", constexpr int" << kBaseTextureTypes[tt].coordCount << " offset";
                sb << ", out uint status";
                sb << ");\n";
            }

            if(baseShape != TextureFlavor::Shape::ShapeCube)
            {
                int N = kBaseTextureTypes[tt].coordCount + isArray;

                char const* uintNs[] = { "", "uint", "uint2", "uint3", "uint4" };
                char const* ivecNs[] = {  "", "int", "ivec2", "ivec3", "ivec4" };

                auto uintN = uintNs[N];
                auto ivecN = ivecNs[N];

                // subscript operator
                sb << "__subscript(" << uintN << " location) -> T {\n";

                // GLSL/SPIR-V distinguished sampled vs. non-sampled images
                switch( access )
                {
                case SLANG_RESOURCE_ACCESS_NONE:
                case SLANG_RESOURCE_ACCESS_READ:
                    sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                    sb << "__target_intrinsic(glsl, \"texelFetch($0, " << ivecN << "($1)";
                    if( !isMultisample )
                    {
                        sb << ", 0";
                    }
                    else
                    {
                        // TODO: how to handle passing through sample index?
                        sb << ", 0";
                    }
                    break;

                default:
                    sb << "__target_intrinsic(glsl, \"imageLoad($0, " << ivecN << "($1)";
                    if( isMultisample )
                    {
                        // TODO: how to handle passing through sample index?
                        sb << ", 0";
                    }
                    break;
                }


                sb << ")$z\") get;\n";

                // Depending on the access level of the texture type,
                // we either have just a getter (the default), or both
                // a getter and setter.
                switch( access )
                {
                case SLANG_RESOURCE_ACCESS_NONE:
                case SLANG_RESOURCE_ACCESS_READ:
                    break;

                default:
                    sb << "__target_intrinsic(glsl, \"imageStore($0, " << ivecN << "($1), $V2)\") set;\n";

                    sb << "__intrinsic_op(" << int(kIROp_ImageSubscript) << ") ref;\n";
                    break;
                }

                sb << "}\n";
            }

            if( !isMultisample )
            {
                // `Sample()`

                sb << "__target_intrinsic(glsl, \"texture($p, $2)$z\")\n";
                sb << "T Sample(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location);\n";

                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureOffset($p, $2, $3)$z\")\n";
                    sb << "T Sample(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }

                sb << "T Sample(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                }
                sb << "float clamp);\n";

                sb << "T Sample(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                }
                sb << "float clamp, out uint status);\n";


                // `SampleBias()`
                sb << "__target_intrinsic(glsl, \"texture($p, $2, $3)$z\")\n";
                sb << "T SampleBias(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, float bias);\n";

                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureOffset($p, $2, $3, $4)$z\")\n";
                    sb << "T SampleBias(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, float bias, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }
                int baseCoordCount = kBaseTextureTypes[tt].coordCount;
                int arrCoordCount = baseCoordCount + isArray;
				if (arrCoordCount <= 3)
                {
					// `SampleCmp()` and `SampleCmpLevelZero`
					sb << "__target_intrinsic(glsl, \"texture($p, vec" << arrCoordCount + 1 << "($2, $3))\")";
					sb << "float SampleCmp(SamplerComparisonState s, ";
					sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
					sb << "float compareValue";
					sb << ");\n";
					sb << "__target_intrinsic(glsl, \"texture($p, vec" << arrCoordCount + 1 << "($2, $3))\")";
					sb << "float SampleCmpLevelZero(SamplerComparisonState s, ";
					sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
					sb << "float compareValue";
					sb << ");\n";
				}
                if (arrCoordCount < 3)
                {
                    int extCoordCount = arrCoordCount + 1;

                    if (extCoordCount < 3)
                        extCoordCount = 3;

                    sb << "__target_intrinsic(glsl, \"textureLod($p, ";

                    sb << "vec" << extCoordCount << "($2,";
                    for (int ii = arrCoordCount; ii < extCoordCount - 1; ++ii)
                    {
                        sb << " 0.0,";
                    }
                    sb << "$3)";

                    sb << ", 0.0)$z\")\n";
                }
                else if(arrCoordCount <= 3)
                {
                    int extCoordCount = arrCoordCount + 1;

                    if (extCoordCount < 3)
                        extCoordCount = 3;

                    sb << "__target_intrinsic(glsl, \"textureGrad($p, ";

                    sb << "vec" << extCoordCount << "($2,";
                    for (int ii = arrCoordCount; ii < extCoordCount - 1; ++ii)
                    {
                        sb << " 0.0,";
                    }
                    sb << "$3)";

                    // Construct gradients
                    sb << ", vec" << baseCoordCount << "(0.0)";
                    sb << ", vec" << baseCoordCount << "(0.0)";
                    sb << ")$z\")\n";
                }
                

                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    // Note(tfoley): MSDN seems confused, and claims that the `offset`
                    // parameter for `SampleCmp` is available for everything but 3D
                    // textures, while `Sample` and `SampleBias` are consistent in
                    // saying they only exclude `offset` for cube maps (which makes
                    // sense). I'm going to assume the documentation for `SampleCmp`
                    // is just wrong.

                    sb << "float SampleCmp(SamplerComparisonState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float compareValue, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                    sb << "float SampleCmpLevelZero(SamplerComparisonState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float compareValue, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }


                sb << "__target_intrinsic(glsl, \"textureGrad($p, $2, $3, $4)$z\")\n";
//                sb << "__intrinsic_op(sampleGrad)\n";
                sb << "T SampleGrad(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY";
                sb << ");\n";

                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureGradOffset($p, $2, $3, $4, $5)$z\")\n";
//                    sb << "__intrinsic_op(sampleGrad)\n";
                    sb << "T SampleGrad(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }

                // `SampleLevel`

                sb << "__target_intrinsic(glsl, \"textureLod($p, $2, $3)$z\")\n";
                sb << "T SampleLevel(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                sb << "float level);\n";

                if( baseShape != TextureFlavor::Shape::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureLodOffset($p, $2, $3, $4)$z\")\n";
                    sb << "T SampleLevel(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float level, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }
            }

            sb << "\n};\n";

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
            } kGatherExtensionCases[] = {
                { "__generic<T, let N : int>", "vector<T,N>" },

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
                sb << kBaseTextureAccessLevels[accessLevel].name;
                sb << name;
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

                for(auto kk : kGatherComponets)
                {
                    auto componentIndex = kk.componentIndex;
                    auto componentName = kk.componentName;

                    EMIT_LINE_DIRECTIVE();

                    sb << "__target_intrinsic(glsl, \"textureGather($p, $2, " << componentIndex << ")\")\n";
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";

                    EMIT_LINE_DIRECTIVE();
                    sb << "__target_intrinsic(glsl, \"textureGatherOffset($p, $2, $3, " << componentIndex << ")\")\n";
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                    EMIT_LINE_DIRECTIVE();
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                    sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                    sb << "out uint status);\n";

                    EMIT_LINE_DIRECTIVE();
                    sb << "__target_intrinsic(glsl, \"textureGatherOffsets($p, $2, int" << kBaseTextureTypes[tt].coordCount << "[]($3, $4, $5, $6), " << componentIndex << ")\")\n";
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset1, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset2, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset3, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset4);\n";

                    EMIT_LINE_DIRECTIVE();
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset1, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset2, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset3, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset4, ";
                    sb << "out uint status);\n";
                }

                EMIT_LINE_DIRECTIVE();
                sb << "\n}\n";
            }
        }
    }
}


for (auto op : unaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* fixity = (op.flags & POSTFIX) != 0 ? "__postfix " : "__prefix ";
        char const* qual = (op.flags & ASSIGNMENT) != 0 ? "in out " : "";

        // scalar version
        sb << fixity;
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << type.name << " operator" << op.opName << "(" << qual << type.name << " value);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << fixity;
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << type.name << ",N> operator" << op.opName << "(" << qual << "vector<" << type.name << ",N> value);\n";

        // matrix version
        sb << "__generic<let N : int, let M : int> ";
        sb << fixity;
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << type.name << ",N,M> operator" << op.opName << "(" << qual << "matrix<" << type.name << ",N,M> value);\n";
    }
}

for (auto op : binaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* leftType = type.name;
        char const* rightType = leftType;
        char const* resultType = leftType;

        if (op.flags & COMPARISON) resultType = "bool";

        char const* leftQual = "";
        if(op.flags & ASSIGNMENT) leftQual = "in out ";

        // TODO: handle `SHIFT`

        // scalar version
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftQual << leftType << " left, " << rightType << " right);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftQual << "vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

        // matrix version

        // skip matrix-matrix multiply operations here, so that GLSL doesn't see them
        switch (op.opCode)
        {
        case kIROp_Mul:
        case kIRPseudoOp_MulAssign:
            break;

        default:
            sb << "__generic<let N : int, let M : int> ";
            sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftQual << "matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";
            break;
        }

        // We are going to go ahead and explicitly define combined
        // operations for the scalar-op-vector, etc. cases, rather
        // than rely on promotion rules.

        // scalar-vector and scalar-matrix
        if (!(op.flags & ASSIGNMENT))
        {
            sb << "__generic<let N : int> ";
            sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftQual << leftType << " left, vector<" << rightType << ",N> right);\n";

            sb << "__generic<let N : int, let M : int> ";
            sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftQual << leftType << " left, matrix<" << rightType << ",N,M> right);\n";
        }

        // vector-scalar and matrix-scalar
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftQual << "vector<" << leftType << ",N> left, " << rightType << " right);\n";

        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftQual << "matrix<" << leftType << ",N,M> left, " << rightType << " right);\n";
    }
}
}}}}

// Operators to apply to `enum` types

__generic<E : __EnumType>
__intrinsic_op($(kIROp_Eql))
bool operator==(E left, E right);

// Binding Attributes

__attributeTarget(DeclBase)
attribute_syntax [vk_binding(binding: int, set: int = 0)]			: GLSLBindingAttribute;

__attributeTarget(DeclBase)
attribute_syntax [gl_binding(binding: int, set: int = 0)]			: GLSLBindingAttribute;


__attributeTarget(VarDeclBase)
attribute_syntax [vk_shader_record]			                        : ShaderRecordAttribute;
__attributeTarget(VarDeclBase)
attribute_syntax [shader_record]			                        : ShaderRecordAttribute;

__attributeTarget(DeclBase)
attribute_syntax [vk_push_constant]									: PushConstantAttribute;
__attributeTarget(DeclBase)
attribute_syntax [push_constant]									: PushConstantAttribute;

// Statement Attributes

__attributeTarget(LoopStmt)
attribute_syntax [unroll(count: int = 0)]   : UnrollAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [loop]                 : LoopAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [fastopt]              : FastOptAttribute;

__attributeTarget(LoopStmt)
attribute_syntax [allow_uav_condition]  : AllowUAVConditionAttribute;

__attributeTarget(IfStmt)
attribute_syntax [flatten]              : FlattenAttribute;

__attributeTarget(IfStmt)
__attributeTarget(SwitchStmt)
attribute_syntax [branch]               : BranchAttribute;

__attributeTarget(SwitchStmt)
attribute_syntax [forcecase]            : ForceCaseAttribute;

__attributeTarget(SwitchStmt)
attribute_syntax [call]                 : CallAttribute;

// Entry-point Attributes

// All Stages
__attributeTarget(FuncDecl)
attribute_syntax [shader(stage)]    : EntryPointAttribute;

// Hull Shader
__attributeTarget(FuncDecl)
attribute_syntax [maxtessfactor(factor: float)]     : MaxTessFactorAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [outputcontrolpoints(count: int)]  : OutputControlPointsAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [outputtopology(topology)]         : OutputTopologyAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [partitioning(mode)]               : PartitioningAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [patchconstantfunc(name)]          : PatchConstantFuncAttribute;

// Hull/Domain Shader
__attributeTarget(FuncDecl)
attribute_syntax [domain(domain)]   : DomainAttribute;

// Geometry Shader
__attributeTarget(FuncDecl)
attribute_syntax [maxvertexcount(count: int)]   : MaxVertexCountAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [instance(count: int)]         : InstanceAttribute;

// Fragment ("Pixel") Shader
__attributeTarget(FuncDecl)
attribute_syntax [earlydepthstencil]    : EarlyDepthStencilAttribute;

// Compute Shader
__attributeTarget(FuncDecl)
attribute_syntax [numthreads(x: int, y: int = 1, z: int = 1)]   : NumThreadsAttribute;

//
__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanRayPayload] : VulkanRayPayloadAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanCallablePayload] : VulkanCallablePayloadAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanHitAttributes] : VulkanHitAttributesAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [mutating] : MutatingAttribute;

    /// Indicates that a function computes its result as a function of its arguments without loading/storing any memory or other state.
    ///
    /// This is equivalent to the LLVM `readnone` function attribute.
__attributeTarget(FunctionDeclBase)
attribute_syntax [__readNone] : ReadNoneAttribute;

enum _AttributeTargets
{
    Struct = $( (int) UserDefinedAttributeTargets::Struct),
    Var = $( (int) UserDefinedAttributeTargets::Var),
    Function = $( (int) UserDefinedAttributeTargets::Function),
};
__attributeTarget(StructDecl)
attribute_syntax [__AttributeUsage(target : _AttributeTargets)] : AttributeUsageAttribute;
