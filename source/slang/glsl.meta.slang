// Slang GLSL compatibility library

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

for( int tt = 0; tt < kTypeCount; ++tt )
{
    // Declare GLSL aliases for HLSL types
    for (int vv = 2; vv <= 4; ++vv)
    {
        sb << "typedef vector<" << kTypes[tt].name << "," << vv << "> " << kTypes[tt].glslPrefix << "vec" << vv << ";\n";
        sb << "typedef matrix<" << kTypes[tt].name << "," << vv << "," << vv << "> " << kTypes[tt].glslPrefix << "mat" << vv << ";\n";
    }
    for (int rr = 2; rr <= 4; ++rr)
    for (int cc = 2; cc <= 4; ++cc)
    {
        sb << "typedef matrix<" << kTypes[tt].name << "," << rr << "," << cc << "> " << kTypes[tt].glslPrefix << "mat" << rr << "x" << cc << ";\n";
    }
}

// Multiplication operations for vectors + matrices

// scalar-vector and vector-scalar
sb << "__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(mul) vector<T,N> operator*(vector<T,N> x, T y);\n";
sb << "__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(mul) vector<T,N> operator*(T x, vector<T,N> y);\n";

// scalar-matrix and matrix-scalar
sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M :int> __intrinsic_op(mul) matrix<T,N,M> operator*(matrix<T,N,M> x, T y);\n";
sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M :int> __intrinsic_op(mul) matrix<T,N,M> operator*(T x, matrix<T,N,M> y);\n";

// vector-vector (dot product)
sb << "__generic<T : __BuiltinArithmeticType, let N : int> __intrinsic_op(dot) T operator*(vector<T,N> x, vector<T,N> y);\n";

// vector-matrix
sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(mulVectorMatrix) vector<T,M> operator*(vector<T,N> x, matrix<T,N,M> y);\n";

// matrix-vector
sb << "__generic<T : __BuiltinArithmeticType, let N : int, let M : int> __intrinsic_op(mulMatrixVector) vector<T,N> operator*(matrix<T,N,M> x, vector<T,M> y);\n";

// matrix-matrix
sb << "__generic<T : __BuiltinArithmeticType, let R : int, let N : int, let C : int> __intrinsic_op(mulMatrixMatrix) matrix<T,R,C> operator*(matrix<T,R,N> x, matrix<T,N,C> y);\n";



//

// TODO(tfoley): Need to handle `RW*` variants of texture types as well...
static const struct {
    char const*			name;
    TextureType::Shape	baseShape;
    int					coordCount;
} kBaseTextureTypes[] = {
    { "1D",		TextureType::Shape1D,	1 },
    { "2D",		TextureType::Shape2D,	2 },
    { "3D",		TextureType::Shape3D,	3 },
    { "Cube",	TextureType::ShapeCube,	3 },
    { "Buffer", TextureType::ShapeBuffer,   1 },
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
    char const* shapeName = kBaseTextureTypes[tt].name;
    TextureType::Shape baseShape = kBaseTextureTypes[tt].baseShape;

    for (int isArray = 0; isArray < 2; ++isArray)
    {
        // Arrays of 3D textures aren't allowed
        if (isArray && baseShape == TextureType::Shape3D) continue;

        for (int isMultisample = 0; isMultisample < 2; ++isMultisample)
        {
            auto readAccess = SLANG_RESOURCE_ACCESS_READ;
            auto readWriteAccess = SLANG_RESOURCE_ACCESS_READ_WRITE;

            // TODO: any constraints to enforce on what gets to be multisampled?

                        
            unsigned flavor = baseShape;
            if (isArray)		flavor |= TextureType::ArrayFlag;
            if (isMultisample)	flavor |= TextureType::MultisampleFlag;
//                        if (isShadow)		flavor |= TextureType::ShadowFlag;



            unsigned readFlavor = flavor | (readAccess << 8);
            unsigned readWriteFlavor = flavor | (readWriteAccess << 8);

            StringBuilder nameBuilder;
            nameBuilder << shapeName;
            if (isMultisample) nameBuilder << "MS";
            if (isArray) nameBuilder << "Array";
            auto name = nameBuilder.ProduceString();

            sb << "__generic<T> ";
            sb << "__magic_type(TextureSampler," << int(readFlavor) << ") struct ";
            sb << "__sampler" << name;
            sb << " {};\n";

            sb << "__generic<T> ";
            sb << "__magic_type(Texture," << int(readFlavor) << ") struct ";
            sb << "__texture" << name;
            sb << " {};\n";

            sb << "__generic<T> ";
            sb << "__magic_type(GLSLImageType," << int(readWriteFlavor) << ") struct ";
            sb << "__image" << name;
            sb << " {};\n";

            // TODO(tfoley): flesh this out for all the available prefixes
            static const struct
            {
                char const* prefix;
                char const* elementType;
            } kTextureElementTypes[] = {
                { "", "vec4" },
                { "i", "ivec4" },
                { "u", "uvec4" },
                { nullptr, nullptr },
            };
            for( auto ee = kTextureElementTypes; ee->prefix; ++ee )
            {
                sb << "typedef __sampler" << name << "<" << ee->elementType << "> " << ee->prefix << "sampler" << name << ";\n";
                sb << "typedef __texture" << name << "<" << ee->elementType << "> " << ee->prefix << "texture" << name << ";\n";
                sb << "typedef __image" << name << "<" << ee->elementType << "> " << ee->prefix << "image" << name << ";\n";
            }
        }
    }
}

sb << "__generic<T> __magic_type(GLSLInputParameterBlockType) struct __GLSLInputParameterBlock {};\n";
sb << "__generic<T> __magic_type(GLSLOutputParameterBlockType) struct __GLSLOutputParameterBlock {};\n";
sb << "__generic<T> __magic_type(GLSLShaderStorageBufferType) struct __GLSLShaderStorageBuffer {};\n";

sb << "__magic_type(SamplerState," << int(SamplerStateType::Flavor::SamplerState) << ") struct sampler {};";

sb << "__magic_type(GLSLInputAttachmentType) struct subpassInput {};";

// Define additional keywords

sb << "syntax buffer : GLSLBufferModifier;\n";

// [GLSL 4.3] Storage Qualifiers

// TODO: need to support `shared` here with its GLSL meaning

sb << "syntax patch : GLSLPatchModifier;\n";
// `centroid` and `sample` handled centrally

// [GLSL 4.5] Interpolation Qualifiers
sb << "syntax smooth : SimpleModifier;\n";
sb << "syntax flat : SimpleModifier;\n";
sb << "syntax noperspectie : SimpleModifier;\n";


// [GLSL 4.3.2] Constant Qualifier

// We need to handle GLSL `const` separately from HLSL `const`,
// since they mean such different things.

// [GLSL 4.7.2] Precision Qualifiers
sb << "syntax highp : SimpleModifier;\n";
sb << "syntax mediump : SimpleModifier;\n";
sb << "syntax lowp : SimpleModifier;\n";

// [GLSL 4.8.1] The Invariant Qualifier

sb << "syntax invariant : SimpleModifier;\n";

// [GLSL 4.10] Memory Qualifiers

sb << "syntax coherent : SimpleModifier;\n";
sb << "syntax volatile : SimpleModifier;\n";
sb << "syntax restrict : SimpleModifier;\n";
sb << "syntax readonly : GLSLReadOnlyModifier;\n";
sb << "syntax writeonly : GLSLWriteOnlyModifier;\n";

// We will treat `subroutine` as a qualifier for now
sb << "syntax subroutine : SimpleModifier;\n";



}}}}