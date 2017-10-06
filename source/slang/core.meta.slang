// Slang `core` library

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
    case BaseType::Float:
        sb << "\n    , __BuiltinFloatingPointType\n";
        sb << "\n    ,  __BuiltinRealType\n";
        // fall through to:
    case BaseType::Int:
        sb << "\n    ,  __BuiltinSignedArithmeticType\n";
        // fall through to:
    case BaseType::UInt:
    case BaseType::UInt64:
        sb << "\n    ,  __BuiltinArithmeticType\n";
        // fall through to:
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

    sb << "};\n";
}

// Declare built-in pointer type
// (eventually we can have the traditional syntax sugar for this)

}}}}

__generic<T>
__magic_type(PtrType)
struct Ptr
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

// TODO: Probably need to do similar
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
    {"float", ""},
    {"int", "i"},
    {"uint", "u"},
    {"bool", "b"},
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



sb << "__magic_type(SamplerState," << int(SamplerStateType::Flavor::SamplerState) << ")\n";
sb << "__intrinsic_type(" << kIROp_SamplerType << ", " << int(SamplerStateType::Flavor::SamplerState) << ")\n";
sb << "struct SamplerState {};";
        
sb << "__magic_type(SamplerState," << int(SamplerStateType::Flavor::SamplerComparisonState) << ")\n";
sb << "__intrinsic_type(" << kIROp_SamplerType << ", " << int(SamplerStateType::Flavor::SamplerComparisonState) << ")\n";
sb << "struct SamplerComparisonState {};";

// TODO(tfoley): Need to handle `RW*` variants of texture types as well...
static const struct {
    char const*			name;
    TextureType::Shape	baseShape;
    int					coordCount;
} kBaseTextureTypes[] = {
    { "Texture1D",		TextureType::Shape1D,	1 },
    { "Texture2D",		TextureType::Shape2D,	2 },
    { "Texture3D",		TextureType::Shape3D,	3 },
    { "TextureCube",	TextureType::ShapeCube,	3 },
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

for (int tt = 0; tt < kBaseTextureTypeCount; ++tt)
{
    char const* name = kBaseTextureTypes[tt].name;
    TextureType::Shape baseShape = kBaseTextureTypes[tt].baseShape;

    for (int isArray = 0; isArray < 2; ++isArray)
    {
        // Arrays of 3D textures aren't allowed
        if (isArray && baseShape == TextureType::Shape3D) continue;

        for (int isMultisample = 0; isMultisample < 2; ++isMultisample)
        for (int accessLevel = 0; accessLevel < kBaseTextureAccessLevelCount; ++accessLevel)
        {
            auto access = kBaseTextureAccessLevels[accessLevel].access;

            // TODO: any constraints to enforce on what gets to be multisampled?

            unsigned flavor = baseShape;
            if (isArray)		flavor |= TextureType::ArrayFlag;
            if (isMultisample)	flavor |= TextureType::MultisampleFlag;
//                        if (isShadow)		flavor |= TextureType::ShadowFlag;

            flavor |= (access << 8);

            // emit a generic signature
            // TODO: allow for multisample count to come in as well...
            sb << "__generic<T = float4> ";

            sb << "__magic_type(Texture," << int(flavor) << ")\n";
            sb << "__intrinsic_type(" << kIROp_TextureType << ", " << flavor << ")\n";
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
                    sb << "__target_intrinsic(glsl, \"(";

                    int aa = 0;
                    String lodStr = "0";
                    if (includeMipInfo)
                    {
                        int mipLevelArg = aa++;
                        lodStr = "int($";
                        lodStr.append(mipLevelArg);
                        lodStr.append(")");
                    }

                    int cc = 0;
                    switch(baseShape)
                    {
                    case TextureType::Shape1D:
                        sb << "($" << aa++ << " = textureSize($$P, " << lodStr << "))";
                        cc = 1;
                        break;

                    case TextureType::Shape2D:
                    case TextureType::ShapeCube:
                        sb << "($" << aa++ << " = textureSize($$P, " << lodStr << ").x)";
                        sb << ", ($" << aa++ << " = textureSize($$P, " << lodStr << ").y)";
                        cc = 2;
                        break;

                    case TextureType::Shape3D:
                        sb << "($" << aa++ << " = textureSize($$P, " << lodStr << ").x)";
                        sb << ", ($" << aa++ << " = textureSize($$P, " << lodStr << ").y)";
                        sb << ", ($" << aa++ << " = textureSize($$P, " << lodStr << ").z)";
                        cc = 3;
                        break;

                    default:
                        SLANG_UNEXPECTED("unhandled resource shape");
                        break;
                    }

                    if(isArray)
                    {
                        sb << ", ($" << aa++ << " = textureSize($$P, " << lodStr << ")." << kComponentNames[cc] << ")";
                    }

                    if(isMultisample)
                    {
                        sb << ", ($" << aa++ << " = textureSamples($$P))";
                    }

                    if (includeMipInfo)
                    {
                        sb << ", ($" << aa++ << " = textureQueryLevels($$P))";
                    }


                    sb << ")\")\n";


					// TIM: Making `GetDimensions` *not* be marked as
					// an intrinsic, just so we can see how defining
					// things as `extern` functions would work.
//                    sb << "__intrinsic_op\n";

                }

                char const* t = isFloat ? "out float " : "out uint ";

                sb << "void GetDimensions(";
                if(includeMipInfo)
                    sb << "uint mipLevel, ";

                switch(baseShape)
                {
                case TextureType::Shape1D:
                    sb << t << "width";
                    break;

                case TextureType::Shape2D:
                case TextureType::ShapeCube:
                    sb << t << "width,";
                    sb << t << "height";
                    break;

                case TextureType::Shape3D:
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
                int loadCoordCount = kBaseTextureTypes[tt].coordCount + isArray + (isMultisample?0:1);

                // When translating to GLSL, we need to break apart the `location` argument.
                //
                // TODO: this should realy be handled by having this member actually get lowered!
                static const char* kGLSLLoadCoordsSwizzle[] = { "", "", "x", "xy", "xyz", "xyzw" };
                static const char* kGLSLLoadLODSwizzle[]    = { "", "", "y", "z", "w", "error" };

                if (isMultisample)
                {
                    sb << "__target_intrinsic(glsl, \"texelFetch($$P, $0, $1)\")\n";
                }
                else
                {
                    sb << "__target_intrinsic(glsl, \"texelFetch($$P, ($0)." << kGLSLLoadCoordsSwizzle[loadCoordCount] << ", ($0)." << kGLSLLoadLODSwizzle[loadCoordCount] << ")\")\n";
                }
                sb << "__intrinsic_op\n";
                sb << "T Load(";
                sb << "int" << loadCoordCount << " location";
                if(isMultisample)
                {
                    sb << ", int sampleIndex";
                }
                sb << ");\n";

                if (isMultisample)
                {
                    sb << "__target_intrinsic(glsl, \"texelFetchOffset($$P, $0, $1, $2)\")\n";
                }
                else
                {
                    sb << "__target_intrinsic(glsl, \"texelFetch($$P, ($0)." << kGLSLLoadCoordsSwizzle[loadCoordCount] << ", ($0)." << kGLSLLoadLODSwizzle[loadCoordCount] << ", $1)\")\n";
                }
                sb << "__intrinsic_op\n";
                sb << "T Load(";
                sb << "int" << loadCoordCount << " location";
                if(isMultisample)
                {
                    sb << ", int sampleIndex";
                }
                sb << ", int" << loadCoordCount << " offset";
                sb << ");\n";


                sb << "T Load(";
                sb << "int" << loadCoordCount << " location";
                if(isMultisample)
                {
                    sb << ", int sampleIndex";
                }
                sb << ", int" << kBaseTextureTypes[tt].coordCount << " offset";
                sb << ", out uint status";
                sb << ");\n";
            }

            if(baseShape != TextureType::ShapeCube)
            {
				// TODO: In the case where `access` includes writeability,
				// this should have both `get` and `set` accessors.

                // subscript operator
                sb << "__intrinsic_op __subscript(uint";
				if(kBaseTextureTypes[tt].coordCount + isArray > 1)
				{
					sb << kBaseTextureTypes[tt].coordCount + isArray;
				}
				sb << " location) -> T;\n";
            }

            if( !isMultisample )
            {
                // `Sample()`

                sb << "__target_intrinsic(glsl, \"texture($$p, $1)\")\n";

                // TODO: only enable if IR is being used?
                sb << "__intrinsic_op(sample)\n";

                sb << "__intrinsic_op\n";
                sb << "T Sample(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location);\n";

                if( baseShape != TextureType::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureOffset($$p, $1, $2)\")\n";
                    sb << "__intrinsic_op\n";
                    sb << "T Sample(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }

                sb << "T Sample(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                if( baseShape != TextureType::ShapeCube )
                {
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                }
                sb << "float clamp);\n";

                sb << "T Sample(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                if( baseShape != TextureType::ShapeCube )
                {
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                }
                sb << "float clamp, out uint status);\n";


                // `SampleBias()`
                sb << "__target_intrinsic(glsl, \"texture($$p, $1, $2)\")\n";
                sb << "__intrinsic_op\n";
                sb << "T SampleBias(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, float bias);\n";

                if( baseShape != TextureType::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureOffset($$p, $1, $2, $3)\")\n";
                    sb << "__intrinsic_op\n";
                    sb << "T SampleBias(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, float bias, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }

                // `SampleCmp()` and `SampleCmpLevelZero`
                sb << "T SampleCmp(SamplerComparisonState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                sb << "float compareValue";
                sb << ");\n";

                int baseCoordCount = kBaseTextureTypes[tt].coordCount;
                int arrCoordCount = baseCoordCount + isArray;
                if (arrCoordCount < 3)
                {
                    int extCoordCount = arrCoordCount + 1;

                    if (extCoordCount < 3)
                        extCoordCount = 3;

                    sb << "__target_intrinsic(glsl, \"textureLod($$p, ";

                    sb << "vec" << extCoordCount << "($1,";
                    for (int ii = arrCoordCount; ii < extCoordCount - 1; ++ii)
                    {
                        sb << " 0.0,";
                    }
                    sb << "$2)";

                    sb << ", 0.0)\")\n";
                }
                else if(arrCoordCount <= 3)
                {
                    int extCoordCount = arrCoordCount + 1;

                    if (extCoordCount < 3)
                        extCoordCount = 3;

                    sb << "__target_intrinsic(glsl, \"textureGrad($$p, ";

                    sb << "vec" << extCoordCount << "($1,";
                    for (int ii = arrCoordCount; ii < extCoordCount - 1; ++ii)
                    {
                        sb << " 0.0,";
                    }
                    sb << "$2)";

                    // Construct gradients
                    sb << ", vec" << baseCoordCount << "(0.0)";
                    sb << ", vec" << baseCoordCount << "(0.0)";
                    sb << ")\")\n";
                }
                sb << "__intrinsic_op\n";
                sb << "T SampleCmpLevelZero(SamplerComparisonState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                sb << "float compareValue";
                sb << ");\n";

                if( baseShape != TextureType::ShapeCube )
                {
                    // Note(tfoley): MSDN seems confused, and claims that the `offset`
                    // parameter for `SampleCmp` is available for everything but 3D
                    // textures, while `Sample` and `SampleBias` are consistent in
                    // saying they only exclude `offset` for cube maps (which makes
                    // sense). I'm going to assume the documentation for `SampleCmp`
                    // is just wrong.

                    sb << "T SampleCmp(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float compareValue, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                    sb << "T SampleCmpLevelZero(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float compareValue, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }


                sb << "__target_intrinsic(glsl, \"textureGrad($$p, $1, $2, $3)\")\n";
                sb << "__intrinsic_op(sampleGrad)\n";
                sb << "T SampleGrad(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY";
                sb << ");\n";

                if( baseShape != TextureType::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureGradOffset($$p, $1, $2, $3, $4)\")\n";
                    sb << "__intrinsic_op(sampleGrad)\n";
                    sb << "T SampleGrad(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                }

                // `SampleLevel`

                sb << "__target_intrinsic(glsl, \"textureLod($$p, $1, $2)\")\n";
                sb << "__intrinsic_op\n";
                sb << "T SampleLevel(SamplerState s, ";
                sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                sb << "float level);\n";

                if( baseShape != TextureType::ShapeCube )
                {
                    sb << "__target_intrinsic(glsl, \"textureLodOffset($$p, $1, $2, $3)\")\n";
                    sb << "__intrinsic_op\n";
                    sb << "T SampleLevel(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float level, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
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
                            
                    sb << "__target_intrinsic(glsl, \"textureGather($$p, $1, " << componentIndex << ")\")\n";
                    sb << "__intrinsic_op\n";
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";

                    EMIT_LINE_DIRECTIVE();
                    sb << "__target_intrinsic(glsl, \"textureGatherOffset($$p, $1, $2, " << componentIndex << ")\")\n";
                    sb << "__intrinsic_op\n";
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                    EMIT_LINE_DIRECTIVE();
                    sb << "vector<T, 4> Gather" << componentName << "(SamplerState s, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location, ";
                    sb << "int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                    sb << "out uint status);\n";

                    EMIT_LINE_DIRECTIVE();
                    sb << "__target_intrinsic(glsl, \"textureGatherOffsets($$p, $1, int" << kBaseTextureTypes[tt].coordCount << "[]($2, $3, $4, $5), " << componentIndex << ")\")\n";
                    sb << "__intrinsic_op\n";
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
