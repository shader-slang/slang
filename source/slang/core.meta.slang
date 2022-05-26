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

/// Modifier to disable inteprolation and force per-vertex passing of a varying attribute.
///
/// When a varying attribute passed to the fragment shader is marked `pervertex`, it will
/// not be interpolated during rasterization (similar to `nointerpolate` attributes).
/// Unlike a plain `nointerpolate` attribute, this modifier indicates that the attribute
/// should *only* be acccessed through the `GetAttributeAtVertex()` operation, so access its
/// distinct per-vertex values.
///
syntax pervertex : PerVertexModifier;

/// Modifier to indicate a buffer or texture element type is
/// backed by data in an unsigned normalized format.
///
/// The `unorm` modifier is only valid on `float` and `vector`s
/// with `float` elements.
///
/// This modifier does not affect the semantics of any variable,
/// parameter, or field that uses it. The semantics of a `float`
/// or vector are the same with or without `unorm`.
///
/// The `unorm` modifier can be used for the element type of a
/// buffer or texture, to indicate that the data that is bound
/// to that buffer or texture is in a matching normalized format.
/// Some platforms may require a `unorm` qualifier for such buffers
/// and textures, and others may operate correctly without it.
///
syntax unorm : UNormModifier;

/// Modifier to indicate a buffer or texture element type is
/// backed by data in an signed normalized format.
///
/// The `snorm` modifier is only valid on `float` and `vector`s
/// with `float` elements.
///
/// This modifier does not affect the semantics of any variable,
/// parameter, or field that uses it. The semantics of a `float`
/// or vector are the same with or without `snorm`.
///
/// The `snorm` modifier can be used for the element type of a
/// buffer or texture, to indicate that the data that is bound
/// to that buffer or texture is in a matching normalized format.
/// Some platforms may require a `unorm` qualifier for such buffers
/// and textures, and others may operate correctly without it.
///
syntax snorm : SNormModifier;

/// Modifier to indicate that a function name should not be mangled
/// by the Slang compiler.
///
/// The `__extern_cpp` modifier makes a symbol to have unmangled
/// name in source/output C++ code.
///
syntax __extern_cpp : ExternCppModifier;

/// A type that can be used as an operand for builtins
[sealed]
[builtin]
interface __BuiltinType {}

/// A type that can be used for arithmetic operations
[sealed]
[builtin]
interface __BuiltinArithmeticType : __BuiltinType
{
        /// Initialize from a 32-bit signed integer value.
    __init(int value);
}

/// A type that can be used for logical/bitwise operations
[sealed]
[builtin]
interface __BuiltinLogicalType : __BuiltinType {}

/// A type that logically has a sign (positive/negative/zero)
[sealed]
[builtin]
interface __BuiltinSignedArithmeticType : __BuiltinArithmeticType {}

/// A type that can represent integers
[sealed]
[builtin]
interface __BuiltinIntegerType : __BuiltinArithmeticType
{}

/// A type that can represent non-integers
[sealed]
[builtin]
interface __BuiltinRealType : __BuiltinSignedArithmeticType {}

/// A type that uses a floating-point representation
[sealed]
[builtin]
interface __BuiltinFloatingPointType : __BuiltinRealType
{
        /// Initialize from a 32-bit floating-point value.
    __init(float value);

        /// Get the value of the mathematical constant pi in this type.
    static This getPi();
}

//@ hidden:

// A type resulting from an `enum` declaration.
[builtin]
__magic_type(EnumTypeType)
interface __EnumType
{
    // The type of tags for this `enum`
    //
    // Note: using `__Tag` instead of `Tag` to avoid any
    // conflict if a user had an `enum` case called `Tag`
    associatedtype __Tag : __BuiltinIntegerType;
};

// Use an extension to declare that every `enum` type
// inherits an initializer based on the tag type.
//
// Note: there is an important and subtle point here.
// If we declared these initializers inside the `interface`
// declaration above, then they would implicitly be
// *requirements* of the `__EnumType` interface, and any
// type that declares conformance to it would need to
// provide implementations. That would put the onus on
// the semantic checker to synthesize such initializers
// when conforming an `enum` type to `__EnumType` (just
// as it currently synthesizes the `__Tag` requirement.
// Putting the declaration in an `extension` makes them
// concrete declerations rather than interface requirements.
// (Admittedly, they are "concrete" declarations with
// no bodies, because currently all initializers are
// assumed to be intrinsics).
//
// TODO: It might be more accurate to express this as:
//
//      __generic<T:__EnumType> extension T { ... }
//
// That alternative would express an extension of every
// type that conforms to `__EnumType`, rather than an
// extension of `__EnumType` itself. The distinction
// is subtle, and unfortunately not one the Slang type
// checker is equiped to handle right now. For now we
// will stick with the syntax that actually works, even
// if it might be the less technically correct one.
//
//
extension __EnumType
{
    // TODO: this should be a single initializer using
    // the `__Tag` associated type from the `__EnumType`
    // interface, but right now the scoping for looking
    // up that type isn't working right.
    //
    __init(int value);
    __init(uint value);
}

// A type resulting from an `enum` declaration
// with the `[flags]` attribute.
[builtin]
interface __FlagsEnumType : __EnumType
{
};

// The "comma operator" is effectively just a generic function that returns its second
// argument. The left-to-right evaluation order guaranteed by Slang then ensures that
// `left` is evaluated before `right`.
//
__generic<T,U>
[__unsafeForceInlineEarly]
U operator,(T left, U right)
{
    return right;
}

// The ternary `?:` operator does not short-circuit in HLSL, and Slang continues to
// follow that definition, so that this operator is effectively just an ordinary
// function, rather than a special-case piece of syntax.
//
__generic<T> __intrinsic_op(select) T operator?:(bool condition, T ifTrue, T ifFalse);
__generic<T, let N : int> __intrinsic_op(select) vector<T,N> operator?:(vector<bool,N> condition, vector<T,N> ifTrue, vector<T,N> ifFalse);

${{{{
// We are going to use code generation to produce the
// declarations for all of our base types.
static const int kBaseTypeCount = sizeof(kBaseTypes) / sizeof(kBaseTypes[0]);
for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
}}}}

__builtin_type($(int(kBaseTypes[tt].tag)))
struct $(kBaseTypes[tt].name)
    : __BuiltinType

${{{{
    switch (kBaseTypes[tt].tag)
    {
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
}}}}
    ,  __BuiltinFloatingPointType
    ,  __BuiltinRealType
    ,  __BuiltinSignedArithmeticType
    ,  __BuiltinArithmeticType
${{{{
        break;
    case BaseType::Int8:
    case BaseType::Int16:
    case BaseType::Int:
    case BaseType::Int64:
}}}}
    ,  __BuiltinSignedArithmeticType
${{{{
        ; // fall through to:
    case BaseType::UInt8:
    case BaseType::UInt16:
    case BaseType::UInt:
    case BaseType::UInt64:
}}}}
    ,  __BuiltinArithmeticType
    ,  __BuiltinIntegerType
${{{{
        ; // fall through to:
    case BaseType::Bool:
}}}}
    ,  __BuiltinLogicalType
${{{{
        break;

    default:
        break;
    }
}}}}
{

${{{{
    // Declare initializers to convert from various other types
    for (int ss = 0; ss < kBaseTypeCount; ++ss)
    {
        // Don't allow conversion to or from `void`
        if (kBaseTypes[tt].tag == BaseType::Void)
            continue;
        if (kBaseTypes[ss].tag == BaseType::Void)
            continue;

        // We need to emit a modifier so that the semantic-checking
        // layer will know it can use these operations for implicit
        // conversion.
        ConversionCost conversionCost = getBaseTypeConversionCost(
            kBaseTypes[tt],
            kBaseTypes[ss]);
}}}}

    __implicit_conversion($(conversionCost))
    __init($(kBaseTypes[ss].name) value);

${{{{
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

    // If this is a floating-point type, then we need to
    // define the basic `getPi()` function that is used
    // to implement generic versions of `degrees()` and
    // `radians()`.
    //
    switch (kBaseTypes[tt].tag)
    {
    default:
        break;
    case BaseType::Half:
    case BaseType::Float:
    case BaseType::Double:
}}}}
        static $(kBaseTypes[tt].name) getPi() { return $(kBaseTypes[tt].name)(3.14159265358979323846264338328); }
${{{{
        break;
    }

    // If this is the `void` type, then we want to allow
    // explicit conversion to it from any other type, using
    // `(void) someExpression`.
    //
    if( kBaseTypes[tt].tag == BaseType::Void )
    {
}}}}
        __generic<T>
        [__readNone]
        __init(T value)
        {}
${{{{
    }

}}}}

}

${{{{
}

// Declare built-in pointer type
// (eventually we can have the traditional syntax sugar for this)
}}}}

__magic_type(NullPtrType)
struct NullPtr
{
};

__generic<T>
__magic_type(PtrType)
__intrinsic_type($(kIROp_PtrType))
struct Ptr
{
};

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

__magic_type(NativeStringType)
__intrinsic_type($(kIROp_NativeStringType))
struct NativeString
{};

__magic_type(DynamicType)
__intrinsic_type($(kIROp_DynamicType))
struct __Dynamic
{};

    /// An `N` component vector with elements of type `T`.
__generic<T = float, let N : int = 4>
__magic_type(Vector)
struct vector
{
        /// The element type of the vector
    typedef T Element;


        /// Initialize a vector where all elements have the same scalar `value`.
    __implicit_conversion($(kConversionCost_ScalarToVector))
    __intrinsic_op($(kIROp_constructVectorFromScalar))
    __init(T value);

        /// Initialize a vector from a value of the same type
    // TODO: we should revise semantic checking so this kind of "identity" conversion is not required
    __init(vector<T,N> value);
}

    /// A matrix with `R` rows and `C` columns, with elements of type `T`.
__generic<T = float, let R : int = 4, let C : int = 4>
__magic_type(Matrix)
struct matrix
{
}

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
}}}}

//@ public:

__generic<T>
__intrinsic_type($(kIROp_ConstantBufferType))
__magic_type(ConstantBuffer)
struct ConstantBuffer {}

__generic<T>
__intrinsic_type($(kIROp_TextureBufferType))
__magic_type(TextureBuffer)
struct TextureBuffer {}

__generic<T>
__intrinsic_type($(kIROp_ParameterBlockType))
__magic_type(ParameterBlockType)
struct ParameterBlock {}

//@ hidden:

// Need to add constructors to the types above

__generic<T> __extension vector<T, 2>
{
    __init(T x, T y);
}
__generic<T> __extension vector<T, 3>
{
    __init(T x, T y, T z);
    __init(vector<T,2> xy, T z);
    __init(T x, vector<T,2> yz);
}
__generic<T> __extension vector<T, 4>
{
    __init(T x, T y, T z, T w);
    __init(vector<T,2> xy, T z, T w);
    __init(T x, vector<T,2> yz, T w);
    __init(T x, T y, vector<T,2> zw);
    __init(vector<T,2> xy, vector<T,2> zw);
    __init(vector<T,3> xyz, T w);
    __init(T x, vector<T,3> yzw);
}

${{{{

static const char* kComponentNames[]{ "x", "y", "z", "w" };

// The above extensions are generic in the *type* of the vector,
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

    // initialize from a matrix of larger size
    for(int rr = R; rr <= 4; ++rr)
    for( int cc = C; cc <= 4; ++cc )
    {
        if(rr == R && cc == C) continue;
        sb << "__init(matrix<T," << rr << "," << cc << "> value);\n";
    }

    sb << "}\n";
}

for (int tt = 0; tt < kBaseTypeCount; ++tt)
{
    if(kBaseTypes[tt].tag == BaseType::Void) continue;
    auto toType = kBaseTypes[tt].name;
}}}}
__generic<let R : int, let C : int> extension matrix<$(toType),R,C>
{
${{{{
    for (int ff = 0; ff < kBaseTypeCount; ++ff)
    {
        if(kBaseTypes[ff].tag == BaseType::Void) continue;
        if( tt == ff ) continue;

        auto cost = getBaseTypeConversionCost(
            kBaseTypes[tt],
            kBaseTypes[ff]);
        auto fromType = kBaseTypes[ff].name;
}}}}
    __implicit_conversion($(cost))
    __init(matrix<$(fromType),R,C> value);
${{{{
    }
}}}}
}
${{{{
}
}}}}


//@ public:

    /// Sampling state for filtered texture fetches.
__magic_type(SamplerState, $(int(SamplerStateFlavor::SamplerState)))
__intrinsic_type($(kIROp_SamplerStateType))
struct SamplerState
{
}

    /// Sampling state for filtered texture fetches that include a comparison operation before filtering.
__magic_type(SamplerState, $(int(SamplerStateFlavor::SamplerComparisonState)))
__intrinsic_type($(kIROp_SamplerComparisonStateType))
struct SamplerComparisonState
{
}

${{{{

static const struct {
    char const*			    shapeName;
    TextureFlavor::Shape	baseShape;
    int					    coordCount;
} kBaseTextureTypes[] = {
    { "1D",		TextureFlavor::Shape::Shape1D,	1 },
    { "2D",		TextureFlavor::Shape::Shape2D,	2 },
    { "3D",		TextureFlavor::Shape::Shape3D,	3 },
    { "Cube",	TextureFlavor::Shape::ShapeCube,3 },
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

static const struct TextureTypePrefixInfo
{
    char const* name;
    bool        combined;
} kTexturePrefixes[] =
{
    { "Texture", false },
    { "Sampler", true },
};

for(auto& prefixInfo : kTexturePrefixes)
for (int tt = 0; tt < kBaseTextureTypeCount; ++tt)
{
    char const* baseName = prefixInfo.name;
    char const* baseShapeName = kBaseTextureTypes[tt].shapeName;
    TextureFlavor::Shape baseShape = kBaseTextureTypes[tt].baseShape;

    for (int isArray = 0; isArray < 2; ++isArray)
    {
        // Arrays of 3D textures aren't allowed
        if (isArray && baseShape == TextureFlavor::Shape::Shape3D) continue;

        for (int isMultisample = 0; isMultisample < 2; ++isMultisample)
        {
            for (int accessLevel = 0; accessLevel < kBaseTextureAccessLevelCount; ++accessLevel)
            {
                auto access = kBaseTextureAccessLevels[accessLevel].access;

                // No such thing as RWTextureCube
                if (access == SLANG_RESOURCE_ACCESS_READ_WRITE && baseShape == TextureFlavor::Shape::ShapeCube)
                {
                    continue;
                }

                // TODO: any constraints to enforce on what gets to be multisampled?

                unsigned flavor = baseShape;
                if (isArray)		flavor |= TextureFlavor::ArrayFlag;
                if (isMultisample)	flavor |= TextureFlavor::MultisampleFlag;
    //                        if (isShadow)		flavor |= TextureFlavor::ShadowFlag;

                flavor |= (access << 8);

                // emit a generic signature
                // TODO: allow for multisample count to come in as well...
                sb << "__generic<T = float4> ";

                if(prefixInfo.combined)
                {
                    sb << "__magic_type(TextureSampler," << int(flavor) << ")\n";
                    sb << "__intrinsic_type(" << (kIROp_TextureSamplerType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
                }
                else
                {
                    sb << "__magic_type(Texture," << int(flavor) << ")\n";
                    sb << "__intrinsic_type(" << (kIROp_TextureType + (int(flavor) << kIROpMeta_OtherShift)) << ")\n";
                }
                sb << "struct ";
                sb << kBaseTextureAccessLevels[accessLevel].name;
                sb << baseName;
                sb << baseShapeName;
                if (isMultisample) sb << "MS";
                if (isArray) sb << "Array";
    //                        if (isShadow) sb << "Shadow";
                sb << "\n{";

                char const* samplerStateParam = prefixInfo.combined ? "" : "SamplerState s, ";

                if( !isMultisample )
                {
                    sb << "float CalculateLevelOfDetail(" << samplerStateParam;
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " location);\n";

                    sb << "float CalculateLevelOfDetailUnclamped(" << samplerStateParam;
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
                            sb << "($" << aa++ << opStr << ")"; 
                            if (isArray)
                            {
                                sb << ".x";
                            }
                            sb << ")";
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

                    if (isMultisample)
                    {
                        sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                        sb << "__target_intrinsic(glsl, \"$c" << glslFuncName << "($0, $1, $2)$z\")\n";
                    }
                    else
                    {
                        sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                        sb << "__target_intrinsic(glsl, \"$c" << glslFuncName << "($0, ";
                        if( needsMipLevel )
                        {
                            sb << "($1)." << kGLSLLoadCoordsSwizzle[loadCoordCount] << ", ($1)." << kGLSLLoadLODSwizzle[loadCoordCount];
                        }
                        else
                        {
                            sb << "$1";
                        }
                        sb << ")$z\")\n";

                    }

                    // CUDA
                    if (isMultisample)
                    {
                    }
                    else
                    {
                        if (access == SLANG_RESOURCE_ACCESS_READ_WRITE)
                        {
                            const int coordCount = kBaseTextureTypes[tt].coordCount;
                            const int vecCount = coordCount + int(isArray);

                            if( baseShape != TextureFlavor::Shape::ShapeCube )
                            {
                                sb << "__target_intrinsic(cuda, \"surf" << coordCount << "D";
                                if (isArray)
                                {
                                    sb << "Layered";
                                }
                                sb << "read";
                                sb << "<$T0>($0";
                                for (int i = 0; i < coordCount; ++i)
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
                                if (isArray)
                                {
                                    sb << ", int(($1)." << char(coordCount + 'x') << ")";
                                }
                                sb << ", SLANG_CUDA_BOUNDARY_MODE)\")\n";
                            }
                            else
                            {
                                sb << "__target_intrinsic(cuda, \"surfCubemap";
                                if (isArray)
                                {
                                    sb << "Layered";
                                }
                                sb << "read";

                                // Surface access is *byte* addressed in x in CUDA
                                sb << "<$T0>($0, ($1).x * $E, ($1).y, ($1).z"; 
                                if (isArray)
                                {
                                    sb << ", int(($1).w)";
                                }
                                sb << ", SLANG_CUDA_BOUNDARY_MODE)\")\n";
                            }
                        }
                        else if (access == SLANG_RESOURCE_ACCESS_READ)
                        {
                            // We can allow this on Texture1D
                            if( baseShape == TextureFlavor::Shape::Shape1D && isArray == false)
                            {
                                sb << "__target_intrinsic(cuda, \"tex1Dfetch<$T0>($0, ($1).x)\")\n";
                            }
                        }
                    }

                    sb << "T Load(";
                    sb << "int" << loadCoordCount << " location";
                    if(isMultisample)
                    {
                        sb << ", int sampleIndex";
                    }
                    sb << ");\n";

                    // GLSL
                    if (isMultisample)
                    {
                        sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                        sb << "__target_intrinsic(glsl, \"$c" << glslFuncName << "($0, $0, $1, $2)$z\")\n";
                    }
                    else
                    {
                        sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                        sb << "__target_intrinsic(glsl, \"$c" << glslFuncName << "($0, ";
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

                    // !!!!!!!!!!!!!!!!!!!! get !!!!!!!!!!!!!!!!!!!!!!!

                    // GLSL/SPIR-V distinguished sampled vs. non-sampled images
                    {
                        switch( access )
                        {
                        case SLANG_RESOURCE_ACCESS_NONE:
                        case SLANG_RESOURCE_ACCESS_READ:
                            sb << "__glsl_extension(GL_EXT_samplerless_texture_functions)";
                            sb << "__target_intrinsic(glsl, \"$ctexelFetch($0, " << ivecN << "($1)";
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
                            sb << "__target_intrinsic(glsl, \"$cimageLoad($0, " << ivecN << "($1)";
                            if( isMultisample )
                            {
                                // TODO: how to handle passing through sample index?
                                sb << ", 0";
                            }
                            break;
                        }
                        sb << ")$z\")\n";
                    }

                    // CUDA
                    {
                        if (access == SLANG_RESOURCE_ACCESS_READ_WRITE)
                        {
                            const int coordCount = kBaseTextureTypes[tt].coordCount;
                            const int vecCount = coordCount + int(isArray);

                            sb << "__target_intrinsic(cuda, \"surf";
                            if( baseShape != TextureFlavor::Shape::ShapeCube )
                            {
                                sb << coordCount << "D";
                            }
                            else
                            {
                                sb << "Cubemap";
                            }

                            sb << (isArray ? "Layered" : "");
                            sb << "read$C<$T0>($0";
                                
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
                        else if (access == SLANG_RESOURCE_ACCESS_READ)
                        {
                            // We can allow this on Texture1D
                            if( baseShape == TextureFlavor::Shape::Shape1D && isArray == false)
                            {
                                sb << "__target_intrinsic(cuda, \"tex1Dfetch<$T0>($0, $1)\")\n";
                            }
                        }
                    }

                    // Output that has get
                    sb << " get;\n";

                    // !!!!!!!!!!!!!!!!!!!! set !!!!!!!!!!!!!!!!!!!!!!!

                    if (!(access == SLANG_RESOURCE_ACCESS_NONE || access == SLANG_RESOURCE_ACCESS_READ))
                    {
                        // GLSL
                        sb << "__target_intrinsic(glsl, \"imageStore($0, " << ivecN << "($1), $V2)\")\n";

                        // CUDA
                        {
                            const int coordCount = kBaseTextureTypes[tt].coordCount;
                            const int vecCount = coordCount + int(isArray);

                            sb << "__target_intrinsic(cuda, \"surf";
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
                        sb << " [nonmutating] set;\n";
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
                        sb << "__intrinsic_op(" << int(kIROp_ImageSubscript) << ") ref;\n";
                        break;
                    }

                    sb << "}\n";
                }

                if( !isMultisample )
                {
                    // `Sample()`

                    sb << "__target_intrinsic(glsl, \"$ctexture($p, $2)$z\")\n";

                    // CUDA
                    {
                        const int coordCount = kBaseTextureTypes[tt].coordCount;
                        const int vecCount = coordCount + int(isArray);

                        if( baseShape != TextureFlavor::Shape::ShapeCube )
                        {
                            sb << "__target_intrinsic(cuda, \"tex" << coordCount << "D";
                            if (isArray)
                            {
                                sb << "Layered";
                            }
                            sb << "<$T0>($0";
                            for (int i = 0; i < coordCount; ++i)
                            {
                                sb << ", ($2)";
                                if (vecCount > 1)
                                {
                                    sb << '.' << char(i + 'x');
                                }
                            }
                            if (isArray)
                            {
                                sb << ", int(($2)." << char(coordCount + 'x') << ")";
                            }
                            sb << ")\")\n";
                        }
                        else
                        {
                            sb << "__target_intrinsic(cuda, \"texCubemap";
                            if (isArray)
                            {
                                sb << "Layered";
                            }
                            sb << "<$T0>($0, ($2).x, ($2).y, ($2).z"; 
                            if (isArray)
                            {
                                sb << ", int(($2).w)";
                            }
                            sb << ")\")\n";
                        }
                    }

                    sb << "T Sample(" << samplerStateParam;;
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location);\n";

                    if( baseShape != TextureFlavor::Shape::ShapeCube )
                    {
                        sb << "__target_intrinsic(glsl, \"$ctextureOffset($p, $2, $3)$z\")\n";
                        sb << "T Sample(" << samplerStateParam;;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";
                    }

                    sb << "T Sample(" << samplerStateParam;
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    if( baseShape != TextureFlavor::Shape::ShapeCube )
                    {
                        sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                    }
                    sb << "float clamp);\n";

                    sb << "T Sample(" << samplerStateParam;
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    if( baseShape != TextureFlavor::Shape::ShapeCube )
                    {
                        sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                    }
                    sb << "float clamp, out uint status);\n";

                    // `SampleBias()`
                    sb << "__target_intrinsic(glsl, \"$ctexture($p, $2, $3)$z\")\n";
                    sb << "T SampleBias(" << samplerStateParam;
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, float bias);\n";

                    if( baseShape != TextureFlavor::Shape::ShapeCube )
                    {
                        sb << "__target_intrinsic(glsl, \"$ctextureOffset($p, $2, $3, $4)$z\")\n";
                        sb << "T SampleBias(" << samplerStateParam;
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

                        sb << "__target_intrinsic(glsl, \"$ctextureLod($p, ";

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

                        sb << "__target_intrinsic(glsl, \"$ctextureGrad($p, ";

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

                    // TODO(JS): Not clear how to map this to CUDA, because in HLSL, the gradient is a vector based on
                    // the dimension. On CUDA there is texNDGrad, but it always just takes ddx, ddy.
                    // I could just assume 0 for elements not supplied, and ignore z. For now will just leave                  
                    sb << "__target_intrinsic(glsl, \"$ctextureGrad($p, $2, $3, $4)$z\")\n";
                    sb << "T SampleGrad(" << samplerStateParam;
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                    sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY";
                    sb << ");\n";

                    if( baseShape != TextureFlavor::Shape::ShapeCube )
                    {
                        sb << "__target_intrinsic(glsl, \"$ctextureGradOffset($p, $2, $3, $4, $5)$z\")\n";
                        sb << "T SampleGrad(" << samplerStateParam;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY, ";
                        sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                        sb << "__glsl_extension(GL_ARB_sparse_texture_clamp)";
                        sb << "__target_intrinsic(glsl, \"$ctextureGradOffsetClampARB($p, $2, $3, $4, $5, $6)$z\")\n";
                        sb << "T SampleGrad(" << samplerStateParam;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " gradX, ";
                        sb << "float" << kBaseTextureTypes[tt].coordCount << " gradY, ";
                        sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                        sb << "float lodClamp);\n";

                    }

                    // `SampleLevel`

                    sb << "__target_intrinsic(glsl, \"$ctextureLod($p, $2, $3)$z\")\n";

                    // CUDA
                    {
                        const int coordCount = kBaseTextureTypes[tt].coordCount;
                        const int vecCount = coordCount + int(isArray);

                        if( baseShape != TextureFlavor::Shape::ShapeCube )
                        {
                            sb << "__target_intrinsic(cuda, \"tex" << coordCount << "D";
                            if (isArray)
                            {
                                sb << "Layered";
                            }
                            sb << "Lod<$T0>($0";
                            for (int i = 0; i < coordCount; ++i)
                            {
                                sb << ", ($2)";
                                if (vecCount > 1)
                                {
                                    sb << '.' << char(i + 'x');
                                }
                            }
                            if (isArray)
                            {
                                sb << ", int(($2)." << char(coordCount + 'x') << ")";
                            }
                            sb << ", $3)\")\n";
                        }
                        else
                        {
                            sb << "__target_intrinsic(cuda, \"texCubemap";
                            if (isArray)
                            {
                                sb << "Layered";
                            }
                            sb << "Lod<$T0>($0, ($2).x, ($2).y, ($2).z";
                            if (isArray)
                            {
                                sb << ", int(($2).w)"; 
                            }
                            sb << ", $3)\")\n";
                        }
                    }

                    sb << "T SampleLevel(" << samplerStateParam;
                    sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                    sb << "float level);\n";

                    if( baseShape != TextureFlavor::Shape::ShapeCube )
                    {
                        sb << "__target_intrinsic(glsl, \"$ctextureLodOffset($p, $2, $3, $4)$z\")\n";
                        sb << "T SampleLevel(" << samplerStateParam;
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
                    sb << kBaseTextureAccessLevels[accessLevel].name;
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

                    for(auto kk : kGatherComponets)
                    {
                        auto componentIndex = kk.componentIndex;
                        auto componentName = kk.componentName;

                        auto outputType = cc.outputType;

                        EMIT_LINE_DIRECTIVE();

                        sb << "__target_intrinsic(glsl, \"textureGather($p, $2, " << componentIndex << ")\")\n";
                        if (kBaseTextureTypes[tt].coordCount == 2)
                        {
                            // Gather only works on 2D in CUDA
                            // "It is based on the base type of DataType except when readMode is equal to cudaReadModeNormalizedFloat (see Texture Reference API), in which case it is always float4."
                            sb << "__target_intrinsic(cuda, \"tex2Dgather<$T0>($0, ($2).x, ($2).y, " << componentIndex << ")\")\n";
                        }
                        sb << outputType << " Gather" << componentName << "(" << samplerStateParam;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location);\n";

                        EMIT_LINE_DIRECTIVE();
                        sb << "__target_intrinsic(glsl, \"textureGatherOffset($p, $2, $3, " << componentIndex << ")\")\n";
                        sb << outputType << " Gather" << componentName << "(" << samplerStateParam;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset);\n";

                        EMIT_LINE_DIRECTIVE();
                        sb << outputType << " Gather" << componentName << "(" << samplerStateParam;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "constexpr int" << kBaseTextureTypes[tt].coordCount << " offset, ";
                        sb << "out uint status);\n";

                        EMIT_LINE_DIRECTIVE();
                        sb << "__target_intrinsic(glsl, \"textureGatherOffsets($p, $2, int" << kBaseTextureTypes[tt].coordCount << "[]($3, $4, $5, $6), " << componentIndex << ")\")\n";
                        sb << outputType << " Gather" << componentName << "(" << samplerStateParam;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
                        sb << "int" << kBaseTextureTypes[tt].coordCount << " offset1, ";
                        sb << "int" << kBaseTextureTypes[tt].coordCount << " offset2, ";
                        sb << "int" << kBaseTextureTypes[tt].coordCount << " offset3, ";
                        sb << "int" << kBaseTextureTypes[tt].coordCount << " offset4);\n";

                        EMIT_LINE_DIRECTIVE();
                        sb << outputType << " Gather" << componentName << "(" << samplerStateParam;
                        sb << "float" << kBaseTextureTypes[tt].coordCount + isArray << " location, ";
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
}

}}}}

//@ hidden:

${{{{

for (auto op : intrinsicUnaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* resultType = type.name;
        if (op.flags & BOOL_RESULT) resultType = "bool";

        // scalar version
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << type.name << " value);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << "vector<" << type.name << ",N> value);\n";

        // matrix version
        sb << "__generic<let N : int, let M : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" <<  "matrix<" << type.name << ",N,M> value);\n";
    }

    // Synthesize generic versions
    if(op.interface)
    {
        char const* resultType = "T";
        if (op.flags & BOOL_RESULT) resultType = "bool";

        // scalar version
        sb << "__generic<T : " << op.interface << ">\n";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << "T value);\n";

        // vector version
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<T,N> value);\n";

        // matrix version
        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "__prefix __intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<T,N,M> value);\n";
    }
}

}}}}

__generic<T : __BuiltinArithmeticType>
[__unsafeForceInlineEarly]
__prefix T operator+(T value)
{ return value; }

__generic<T : __BuiltinArithmeticType, let N : int>
[__unsafeForceInlineEarly]
__prefix vector<T,N> operator+(vector<T,N> value)
{ return value; }

__generic<T : __BuiltinArithmeticType, let R : int, let C : int>
[__unsafeForceInlineEarly]
__prefix matrix<T,R,C> operator+(matrix<T,R,C> value)
{ return value; }

${{{{

static const struct IncDecOpInfo
{
    char const* name;
    char const* binOp;
} kIncDecOps[] =
{
    { "++", "+" },
    { "--", "-" },
};
static const struct IncDecOpFixity
{
    char const* qual;
    char const* bodyPrefix;
    char const* returnVal;
} kIncDecFixities[] =
{
    { "__prefix", "", "value" },
    { "__postfix", " let result = value;", "result" },
};
for(auto op : kIncDecOps)
for(auto fixity : kIncDecFixities)
{
}}}}

$(fixity.qual)
__generic<T : __BuiltinArithmeticType>
[__unsafeForceInlineEarly]
T operator$(op.name)(in out T value)
{$(fixity.bodyPrefix) value = value $(op.binOp) T(1); return $(fixity.returnVal); }

$(fixity.qual)
__generic<T : __BuiltinArithmeticType, let N : int>
[__unsafeForceInlineEarly]
vector<T,N> operator$(op.name)(in out vector<T,N> value)
{$(fixity.bodyPrefix) value = value $(op.binOp) T(1); return $(fixity.returnVal); }

$(fixity.qual)
__generic<T : __BuiltinArithmeticType, let R : int, let C : int>
[__unsafeForceInlineEarly]
matrix<T,R,C> operator$(op.name)(in out matrix<T,R,C> value)
{$(fixity.bodyPrefix) value = value $(op.binOp) T(1); return $(fixity.returnVal); }

${{{{
}

for (auto op : intrinsicBinaryOps)
{
    for (auto type : kBaseTypes)
    {
        if ((type.flags & op.flags) == 0)
            continue;

        char const* leftType = type.name;
        char const* rightType = leftType;
        char const* resultType = leftType;

        if (op.flags & BOOL_RESULT) resultType = "bool";

        // TODO: We should handle a `SHIFT` flag on the op
        // by changing `rightType` to `int` in order to
        // account for the fact that the shift amount should
        // always have a fixed type independent of the LHS.
        //
        // (It is unclear why this change hadn't been made
        // already, so it is possible that such a change
        // breaks overload resolution or other parts of
        // the compiler)

        // scalar version
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftType << " left, " << rightType << " right);\n";

        // vector version
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

        // matrix version
        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";

        // We currently synthesize addiitonal overloads
        // for the case where one or the other operand
        // is a scalar. This choice serves a few purposes:
        //
        // 1. It avoids introducing scalar-to-vector or
        // scalar-to-matrix promotions before the operator,
        // which might allow some back ends to produce
        // more optimal code.
        //
        // 2. It avoids concerns about making overload resolution
        // and the inference rules for `N` and `M` able to
        // handle the mixed vector/scalar or matrix/scalar case.
        //
        // 3. Having explicit overloads for the matrix/scalar cases
        // here means that we do *not* need to support a general
        // implicit conversion from scalars to matrices, unless
        // we decide we want to.
        //
        // Note: Case (2) of the motivation shouldn't really apply
        // any more, because we end up having to support similar
        // inteference for built-in binary math functions where
        // vectors and scalars might be combined (and where defining
        // additional overloads to cover all the combinations doesn't
        // seem practical or desirable).
        //
        // TODO: We should consider whether dropping these extra
        // overloads is possible and worth it. The optimization
        // concern (1) could possibly be addressed in specific
        // back-ends. The issue (3) about not wanting to support
        // implicit scalar-to-matrix conversion may be moot if
        // we end up needing to support mixed scalar/matrix input
        // for builtin in non-operator functions anyway.

        // scalar-vector and scalar-matrix
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftType << " left, vector<" << rightType << ",N> right);\n";

        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" << leftType << " left, matrix<" << rightType << ",N,M> right);\n";

        // vector-scalar and matrix-scalar
        sb << "__generic<let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, " << rightType << " right);\n";

        sb << "__generic<let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, " << rightType << " right);\n";
    }

    // Synthesize generic versions
    if(op.interface)
    {
        char const* leftType = "T";
        char const* rightType = leftType;
        char const* resultType = leftType;

        if (op.flags & BOOL_RESULT) resultType = "bool";
        // TODO: handle `SHIFT`

        // scalar version
        sb << "__generic<T : " << op.interface << ">\n";
        sb << "__intrinsic_op(" << int(op.opCode) << ") " << resultType << " operator" << op.opName << "(" << leftType << " left, " << rightType << " right);\n";

        // vector version
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, vector<" << rightType << ",N> right);\n";

        // matrix version
        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, matrix<" << rightType << ",N,M> right);\n";

        // scalar-vector and scalar-matrix
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(" << leftType << " left, vector<" << rightType << ",N> right);\n";

        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(" <<  leftType << " left, matrix<" << rightType << ",N,M> right);\n";

        // vector-scalar and matrix-scalar
        sb << "__generic<T : " << op.interface << ", let N : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") vector<" << resultType << ",N> operator" << op.opName << "(vector<" << leftType << ",N> left, " << rightType << " right);\n";

        sb << "__generic<T : " << op.interface << ", let N : int, let M : int> ";
        sb << "__intrinsic_op(" << int(op.opCode) << ") matrix<" << resultType << ",N,M> operator" << op.opName << "(matrix<" << leftType << ",N,M> left, " << rightType << " right);\n";
    }
}

// We will declare the shift operations entirely as generics
// rather than try to handle all the pairings of left-hand
// and right-hand side types.
//
static const struct ShiftOpInfo
{
    char const* name;
    int op;
} kShiftOps[] =
{
    { "<<",  kIROp_Lsh },
    { ">>",  kIROp_Rsh },
};
for(auto info : kShiftOps) {
}}}}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>
__intrinsic_op($(info.op))
L operator$(info.name)(L left, R right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType>
[__unsafeForceInlineEarly]
L operator$(info.name)=(in out L left, R right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
__intrinsic_op($(info.op))
vector<L,N> operator$(info.name)(vector<L,N> left, vector<R,N> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
[__unsafeForceInlineEarly]
vector<L,N> operator$(info.name)=(in out vector<L,N> left, vector<R,N> right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
__intrinsic_op($(info.op))
matrix<L,N,M> operator$(info.name)(matrix<L,N,M> left, matrix<R,N,M> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
[__unsafeForceInlineEarly]
matrix<L, N, M> operator$(info.name)=(in out matrix<L, N, M> left, matrix<R, N, M> right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
__intrinsic_op($(info.op))
vector<L,N> operator$(info.name)(L left, vector<R,N> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
__intrinsic_op($(info.op))
matrix<L,N,M> operator$(info.name)(L left, matrix<R,N,M> right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
__intrinsic_op($(info.op))
vector<L,N> operator$(info.name)(vector<L,N> left, R right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int>
[__unsafeForceInlineEarly]
vector<L, N> operator$(info.name)=(in out vector<L, N> left, R right)
{
    left = left $(info.name) right;
    return left;
}

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
__intrinsic_op($(info.op))
matrix<L,N,M> operator$(info.name)(matrix<L,N,M> left, R right);

__generic<L: __BuiltinIntegerType, R: __BuiltinIntegerType, let N : int, let M : int>
[__unsafeForceInlineEarly]
matrix<L,N,M> operator$(info.name)=(in out matrix<L,N,M> left, R right)
{
    left = left $(info.name) right;
    return left;
}

${{{{
}

    static const struct CompoundBinaryOpInfo
    {
        char const* name;
        char const* interface;
    } kCompoundBinaryOps[] =
    {
        { "+",  "__BuiltinArithmeticType" },
        { "-",  "__BuiltinArithmeticType" },
        { "*",  "__BuiltinArithmeticType" },
        { "/",  "__BuiltinArithmeticType" },
        { "%",  "__BuiltinIntegerType" },
        { "%",  "__BuiltinFloatingPointType" },
        { "&",  "__BuiltinLogicalType" },
        { "|",  "__BuiltinLogicalType" },
        { "^",  "__BuiltinLogicalType" },
    };
    for( auto op : kCompoundBinaryOps )
    {
    }}}}

    __generic<T : $(op.interface)>
    [__unsafeForceInlineEarly]
    T operator$(op.name)=(in out T left, T right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let N : int>
    [__unsafeForceInlineEarly]
    vector<T,N> operator$(op.name)=(in out vector<T,N> left, vector<T,N> right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let N : int>
    [__unsafeForceInlineEarly]
    vector<T,N> operator$(op.name)=(in out vector<T,N> left, T right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let R : int, let C : int>
    [__unsafeForceInlineEarly]
    matrix<T,R,C> operator$(op.name)=(in out matrix<T,R,C> left, matrix<T,R,C> right)
    {
        left = left $(op.name) right;
        return left;
    }

    __generic<T : $(op.interface), let R : int, let C : int>
    [__unsafeForceInlineEarly]
    matrix<T,R,C> operator$(op.name)=(in out matrix<T,R,C> left, T right)
    {
        left = left $(op.name) right;
        return left;
    }

    ${{{{
    }

}}}}

//@ public:

// Bit cast
__generic<T, U>
[__unsafeForceInlineEarly]
__intrinsic_op($(kIROp_BitCast))
T bit_cast(U value);

// Create Existential object
__generic<T, U>
[__unsafeForceInlineEarly]
__intrinsic_op($(kIROp_CreateExistentialObject))
T createDynamicObject(uint typeId, U value);

// Reinterpret
__generic<T, U>
[__unsafeForceInlineEarly]
__intrinsic_op($(kIROp_Reinterpret))
T reinterpret(U value);

// Specialized function

/// Given a string returns an integer hash of that string.
__intrinsic_op
int getStringHash(String string);

/// Use will produce a syntax error in downstream compiler
/// Useful for testing diagnostics around compilation errors of downstream compiler
/// It 'returns' an int so can be used in expressions without the front end complaining.
__target_intrinsic(hlsl, " @ ")
__target_intrinsic(glsl, " @ ")
__target_intrinsic(cuda, " @ ")
__target_intrinsic(cpp, " @ ")
int __SyntaxError();

/// For downstream compilers that allow sizeof/alignof/offsetof
/// Can't be called in the C/C++ style. Need to use __size_of<some_type>() as opposed to sizeof(some_type).
__generic<T>
__target_intrinsic(cuda, "sizeof($G0)")
__target_intrinsic(cpp, "sizeof($G0)")
int __sizeOf();

__generic<T>
__target_intrinsic(cuda, "sizeof($T0)")
__target_intrinsic(cpp, "sizeof($T0)")
int __sizeOf(T v);

__generic<T>
__target_intrinsic(cuda, "SLANG_ALIGN_OF($G0)")
__target_intrinsic(cpp, "SLANG_ALIGN_OF($G0)")
int __alignOf();

__generic<T>
__target_intrinsic(cuda, "SLANG_ALIGN_OF($T0)")
__target_intrinsic(cpp, "SLANG_ALIGN_OF($T0)")
int __alignOf(T v);

// It would be nice to have offsetof equivalent, but it's not clear how that would work in terms of the Slang language.
// Here we allow calculating the offset of a field in bytes from an *instance* of the type.
__generic<T,F>
__target_intrinsic(cuda, "int(((char*)&($1)) - ((char*)&($0)))")
__target_intrinsic(cpp, "int(((char*)&($1)) - ((char*)&($0))")
int __offsetOf(in T t, in F field);

/// Mark beginning of "interlocked" operations in a fragment shader.
__target_intrinsic(glsl, "beginInvocationInterlockARB")
__glsl_extension(GL_ARB_fragment_shader_interlock)
__glsl_version(420)
void beginInvocationInterlock() {}

/// Mark end of "interlocked" operations in a fragment shader.
__target_intrinsic(glsl, "endInvocationInterlockARB")
__glsl_extension(GL_ARB_fragment_shader_interlock)
__glsl_version(420)
void endInvocationInterlock() {}

// Operators to apply to `enum` types

//@ hidden:

__generic<E : __EnumType>
__intrinsic_op($(kIROp_Eql))
bool operator==(E left, E right);

__generic<E : __EnumType>
__intrinsic_op($(kIROp_Neq))
bool operator!=(E left, E right);

//@ public:

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

__attributeTarget(VarDeclBase)
attribute_syntax [vk_location(locaiton : int)] : GLSLLocationAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [vk_index(index : int)] : GLSLIndexAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [vk_spirv_instruction(op : int)]                   : SPIRVInstructionOpAttribute;

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
attribute_syntax [__vulkanRayPayload(location : int = -1)] : VulkanRayPayloadAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanCallablePayload(location : int = -1)] : VulkanCallablePayloadAttribute;

__attributeTarget(VarDeclBase)
attribute_syntax [__vulkanHitAttributes] : VulkanHitAttributesAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [mutating] : MutatingAttribute;

__attributeTarget(SetterDecl)
attribute_syntax [nonmutating] : NonmutatingAttribute;

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

__attributeTarget(VarDeclBase)
attribute_syntax [format(format : String)] : FormatAttribute;

__attributeTarget(Decl)
attribute_syntax [allow(diagnostic: String)] : AllowAttribute;

// Linking
__attributeTarget(Decl)
attribute_syntax [__extern] : ExternAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [__unsafeForceInlineEarly] : UnsafeForceInlineEarlyAttribute;

__attributeTarget(FuncDecl)
attribute_syntax [DllImport(modulePath: String)] : DllImportAttribute;

__attributeTarget(InterfaceDecl)
attribute_syntax [COM] : ComInterfaceAttribute;

// Inheritance Control
__attributeTarget(AggTypeDecl)
attribute_syntax [sealed] : SealedAttribute;

__attributeTarget(AggTypeDecl)
attribute_syntax [open] : OpenAttribute;

__attributeTarget(InterfaceDecl)
attribute_syntax [anyValueSize(size:int)] : AnyValueSizeAttribute;

__attributeTarget(DeclBase)
attribute_syntax [builtin] : BuiltinAttribute;

__attributeTarget(DeclBase)
attribute_syntax [__requiresNVAPI] : RequiresNVAPIAttribute;

__attributeTarget(FunctionDeclBase)
attribute_syntax [noinline] : NoInlineAttribute;

__attributeTarget(StructDecl)
attribute_syntax [payload] : PayloadAttribute;

