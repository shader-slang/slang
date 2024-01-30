// TODO: These keywords are not recognized but they should be.
#define highp
#define mediump
#define lowp

#define VECTOR_MAP_UNARY(TYPE, COUNT, FUNC, VALUE) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(VALUE[i]); } return result

#define VECTOR_MAP_TRINARY(TYPE, COUNT, FUNC, A, B, C) \
    vector<TYPE,COUNT> result; for(int i = 0; i < COUNT; ++i) { result[i] = FUNC(A[i], B[i], C[i]); } return result

//
// OpenGL 4.60 spec
//

//
// Section 4.1. 'asic Types'
//

public typealias vec2 = vector<float, 2>;
public typealias vec3 = vector<float, 3>;
public typealias vec4 = vector<float, 4>;

public typealias dvec2 = vector<double, 2>;
public typealias dvec3 = vector<double, 3>;
public typealias dvec4 = vector<double, 4>;

public typealias bvec2 = vector<bool, 2>;
public typealias bvec3 = vector<bool, 3>;
public typealias bvec4 = vector<bool, 4>;

public typealias ivec2 = vector<int, 2>;
public typealias ivec3 = vector<int, 3>;
public typealias ivec4 = vector<int, 4>;

public typealias uvec2 = vector<uint, 2>;
public typealias uvec3 = vector<uint, 3>;
public typealias uvec4 = vector<uint, 4>;

public typealias i8vec2 = vector<int8_t, 2>;
public typealias i8vec3 = vector<int8_t, 3>;
public typealias i8vec4 = vector<int8_t, 4>;

public typealias u8vec2 = vector<uint8_t, 2>;
public typealias u8vec3 = vector<uint8_t, 3>;
public typealias u8vec4 = vector<uint8_t, 4>;

public typealias i16vec2 = vector<int16_t, 2>;
public typealias i16vec3 = vector<int16_t, 3>;
public typealias i16vec4 = vector<int16_t, 4>;

public typealias u16vec2 = vector<uint16_t, 2>;
public typealias u16vec3 = vector<uint16_t, 3>;
public typealias u16vec4 = vector<uint16_t, 4>;

public typealias i64vec2 = vector<int64_t, 2>;
public typealias i64vec3 = vector<int64_t, 3>;
public typealias i64vec4 = vector<int64_t, 4>;

public typealias u64vec2 = vector<uint64_t, 2>;
public typealias u64vec3 = vector<uint64_t, 3>;
public typealias u64vec4 = vector<uint64_t, 4>;

public typealias mat2 = matrix<float, 2, 2>;
public typealias mat3 = matrix<float, 3, 3>;
public typealias mat4 = matrix<float, 4, 4>;

public typealias mat2x2 = matrix<float, 2, 2>;
public typealias mat2x3 = matrix<float, 3, 2>;
public typealias mat2x4 = matrix<float, 4, 2>;

public typealias mat3x2 = matrix<float, 2, 3>;
public typealias mat3x3 = matrix<float, 3, 3>;
public typealias mat3x4 = matrix<float, 4, 3>;

public typealias mat4x2 = matrix<float, 2, 4>;
public typealias mat4x3 = matrix<float, 3, 4>;
public typealias mat4x4 = matrix<float, 4, 4>;

public typealias dmat2 = matrix<double, 2, 2>;
public typealias dmat3 = matrix<double, 3, 3>;
public typealias dmat4 = matrix<double, 4, 4>;

public typealias dmat2x2 = matrix<double, 2, 2>;
public typealias dmat2x3 = matrix<double, 3, 2>;
public typealias dmat2x4 = matrix<double, 4, 2>;

public typealias dmat3x2 = matrix<double, 2, 3>;
public typealias dmat3x3 = matrix<double, 3, 3>;
public typealias dmat3x4 = matrix<double, 4, 3>;

public typealias dmat4x2 = matrix<double, 2, 4>;
public typealias dmat4x3 = matrix<double, 3, 4>;
public typealias dmat4x4 = matrix<double, 4, 4>;

public typealias usampler1D = Sampler1D<uint4>;
public typealias isampler1D = Sampler1D<int4>;
public typealias sampler1D = Sampler1D<float4>;

public typealias usampler2D = Sampler2D<uint4>;
public typealias isampler2D = Sampler2D<int4>;
public typealias sampler2D = Sampler2D<float4>;

public typealias usampler3D = Sampler3D<uint4>;
public typealias isampler3D = Sampler3D<int4>;
public typealias sampler3D = Sampler3D<float4>;

public typealias usamplerCube = SamplerCube<uint4>;
public typealias isamplerCube = SamplerCube<int4>;
public typealias samplerCube = SamplerCube<float4>;

public typealias Sampler1DShadow<T=float> = __TextureImpl<T, __Shape1D, /*isArray:*/ 0, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
public typealias usampler1DShadow = Sampler1DShadow<uint>;
public typealias isampler1DShadow = Sampler1DShadow<int>;
public typealias sampler1DShadow = Sampler1DShadow<float>;

public typealias Sampler2DShadow<T=float> = __TextureImpl<T, __Shape2D, /*isArray:*/ 0, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
public typealias usampler2DShadow = Sampler2DShadow<uint>;
public typealias isampler2DShadow = Sampler2DShadow<int>;
public typealias sampler2DShadow = Sampler2DShadow<float>;

public typealias SamplerCubeShadow<T=float> = __TextureImpl<T, __ShapeCube, /*isArray:*/ 0, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
public typealias usamplerCubeShadow = SamplerCubeShadow<uint>;
public typealias isamplerCubeShadow = SamplerCubeShadow<int>;
public typealias samplerCubeShadow = SamplerCubeShadow<float>;

public typealias usampler1DArray = Sampler1DArray<uint4>;
public typealias isampler1DArray = Sampler1DArray<int4>;
public typealias sampler1DArray = Sampler1DArray<float4>;

public typealias usampler2DArray = Sampler2DArray<uint4>;
public typealias isampler2DArray = Sampler2DArray<int4>;
public typealias sampler2DArray = Sampler2DArray<float4>;

public typealias usamplerCubeArray = SamplerCubeArray<uint4>;
public typealias isamplerCubeArray = SamplerCubeArray<int4>;
public typealias samplerCubeArray = SamplerCubeArray<float4>;

public typealias Sampler1DArrayShadow<T=float> = __TextureImpl<T, __Shape1D, /*isArray:*/ 1, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
public typealias usampler1DArrayShadow = Sampler1DArrayShadow<uint>;
public typealias isampler1DArrayShadow = Sampler1DArrayShadow<int>;
public typealias sampler1DArrayShadow = Sampler1DArrayShadow<float>;

public typealias Sampler2DArrayShadow<T=float> = __TextureImpl<T, __Shape2D, /*isArray:*/ 1, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
public typealias usampler2DArrayShadow = Sampler2DArrayShadow<uint>;
public typealias isampler2DArrayShadow = Sampler2DArrayShadow<int>;
public typealias sampler2DArrayShadow = Sampler2DArrayShadow<float>;

public typealias SamplerCubeArrayShadow<T=float> = __TextureImpl<T, __ShapeCube, /*isArray:*/ 1, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
public typealias usamplerCubeArrayShadow = SamplerCubeArrayShadow<uint>;
public typealias isamplerCubeArrayShadow = SamplerCubeArrayShadow<int>;
public typealias samplerCubeArrayShadow = SamplerCubeArrayShadow<float>;

[ForceInline]
public vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler1D<vector<T,N>> sampler, int p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int2(p, lod)));
}

[ForceInline]
public vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler2D<vector<T,N>> sampler, ivec2 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int3(p, lod)));
}

[ForceInline]
public vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler3D<vector<T,N>> sampler, ivec3 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int4(p, lod)));
}

[ForceInline]
public vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler1DArray<vector<T,N>> sampler, ivec2 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int3(p, lod)));
}

[ForceInline]
public vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler2DArray<vector<T,N>> sampler, ivec3 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int4(p, lod)));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinFloatingPointType, let N : int> (Sampler1D<vector<T,N>> sampler, float p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinFloatingPointType, let N : int> (Sampler2D<vector<T,N>> sampler, float2 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinFloatingPointType, let N : int> (Sampler3D<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinFloatingPointType, let N : int> (SamplerCube<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public float texture<T:__BuiltinFloatingPointType, let N : int> (Sampler1DShadow<vector<T,N>> sampler, float2 p)
{
    return sampler.SampleCmp(p.x, p.y);
}

[ForceInline]
public float texture<T:__BuiltinFloatingPointType, let N : int> (Sampler2DShadow<vector<T,N>> sampler, float3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture<T:__BuiltinFloatingPointType, let N : int> (SamplerCubeShadow<vector<T,N>> sampler, float4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinFloatingPointType, let N : int> (Sampler1DArray<vector<T,N>> sampler, float2 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinFloatingPointType, let N : int> (Sampler2DArray<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinFloatingPointType, let N : int> (SamplerCubeArray<vector<T,N>> sampler, float4 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public float texture<T:__BuiltinFloatingPointType, let N : int> (Sampler1DArrayShadow<vector<T,N>> sampler, float3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture<T:__BuiltinFloatingPointType, let N : int> (Sampler2DArrayShadow<vector<T,N>> sampler, float4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

__generic<T:__BuiltinFloatingPointType, let N : int, shape:__ITextureShape, let sampleCount: int, let isArray:int, let format:int>
public vector<T,4> textureGrad(__TextureImpl<vector<T,N>, shape, isArray, 0, sampleCount, 0, 1, 1, format> sampler, vector<float, shape.dimensions+isArray> P, vector<float,shape.planeDimensions> dPdx, vector<float,shape.planeDimensions> dPdy)
{
    return __vectorReshape<4>(sampler.SampleGrad(P, dPdx, dPdy));
}

public out float4 gl_Position : SV_Position;
public out float gl_PointSize : SV_PointSize;
public in vec4 gl_FragCoord : SV_Position;
public out float gl_FragDepth : SV_Depth;
public out int gl_FragStencilRef : SV_StencilRef;

public in uvec3 gl_GlobalInvocationID : SV_DispatchThreadID;
public in uvec3 gl_WorkGroupID : SV_GroupID;
public in uvec3 gl_LocalInvocationIndex : SV_GroupIndex;
public in uvec3 gl_LocalInvocationID : SV_GroupThreadID;

// TODO: define overload for tessellation control stage.
public in int gl_InvocationID : SV_GSInstanceID;

public in int gl_InstanceIndex : SV_InstanceID;
public in bool gl_FrontFacing : SV_IsFrontFace;

// TODO: define overload for geometry stage.
public in int gl_Layer : SV_RenderTargetArrayIndex;

public in int gl_SampleID : SV_SampleIndex;
public in int gl_VertexIndex : SV_VertexID;
public in int gl_ViewIndex : SV_ViewID;
public in int gl_ViewportIndex : SV_ViewportArrayIndex;


// Override operator* behavior to compute algebric product of matrices and vectors.

[OverloadRank(15)]
[ForceInline]
public matrix<float, N, N> operator*<let N : int>(matrix<float, N, N> m1, matrix<float, N, N> m2)
{
    return mul(m2, m1);
}

[OverloadRank(15)]
[ForceInline]
public matrix<half, N, N> operator*<let N : int>(matrix<half, N, N> m1, matrix<half, N, N> m2)
{
    return mul(m2, m1);
}

[OverloadRank(15)]
[ForceInline]
public matrix<double, N, N> operator*<let N : int>(matrix<double, N, N> m1, matrix<double, N, N> m2)
{
    return mul(m2, m1);
}

[ForceInline]
[OverloadRank(15)]
public matrix<T, R, L> operator*<T:__BuiltinFloatingPointType, let L : int, let C : int, let R : int>(matrix<T, C, L> m1, matrix<T, R, C> m2)
{
    return mul(m2, m1);
}

[ForceInline]
[OverloadRank(15)]
public vector<T, R> operator*<T:__BuiltinFloatingPointType, let C : int, let R : int>(vector<T, C> v, matrix<T, R, C> m)
{
    return mul(m, v);
}

[ForceInline]
[OverloadRank(15)]
public vector<T, C> operator*<T:__BuiltinFloatingPointType, let C : int, let R : int>(matrix<T, R, C> m, vector<T, R> v)
{
    return mul(v, m);
}

__intrinsic_op(mul)
public matrix<T, N, M> matrixCompMult<T:__BuiltinFloatingPointType, let N : int, let M : int>(matrix<T,N,M> left, matrix<T,N,M> right);

__intrinsic_op(cmpLE)
public vector<bool, N> lessThanEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpLT)
public vector<bool, N> lessThan<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpGT)
public vector<bool, N> greaterThan<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpGE)
public vector<bool, N> greaterThanEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpEQ)
public vector<bool, N> equal<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpNE)
public vector<bool, N> notEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);

__generic<T>
public extension vector<T, 2>
{
    [ForceInline] public __init(vector<T, 3> bigger) { this = bigger.xy; }
    [ForceInline] public __init(vector<T, 4> bigger) { this = bigger.xy; }
}

__generic<T>
public extension vector<T, 3>
{
    [ForceInline] public __init(vector<T, 4> bigger) { this = bigger.xyz; }
}

[ForceInline]
[OverloadRank(15)]
public bool operator==<T:__BuiltinArithmeticType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(15)]
public bool operator!=<T:__BuiltinArithmeticType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator==<T:__BuiltinFloatingPointType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator!=<T:__BuiltinFloatingPointType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator==<T:__BuiltinLogicalType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(14)]
public bool operator!=<T:__BuiltinLogicalType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

${{{{
for (auto type : kBaseTypes) {
    char const* typeName = type.name;
    if (!type.flags) continue;
}}}}
[ForceInline]
[OverloadRank(15)]
public bool operator==<let N : int>(vector<$(typeName), N> left, vector<$(typeName), N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(15)]
public bool operator!=<let N : int>(vector<$(typeName), N> left, vector<$(typeName), N> right)
{
    return any(notEqual(left, right));
}
${{{{
}
}}}}

//
// Section 8.1. Angle and Trigonometry Functions
//

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T atan(T y, T x)
{
    return atan2(y, x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T,N> atan(vector<T,N> y, vector<T,N> x)
{
    return atan2(y, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_asinh($0)")
__target_intrinsic(cpp, "$P_asinh($0)")
[__readNone]
[ForceInline]
public T asinh(T x)
{
    return log(x + sqrt(x * x + T(1)));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T,N> asinh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, asinh, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_acosh($0)")
__target_intrinsic(cpp, "$P_acosh($0)")
[__readNone]
[ForceInline]
public T acosh(T x)
{
    return log(x + sqrt( x * x - T(1)));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T,N> acosh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, acosh, x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
__target_intrinsic(cuda, "$P_atanh($0)")
__target_intrinsic(cpp, "$P_atanh($0)")
[__readNone]
[ForceInline]
public T atanh(T x)
{
    return T(0.5) * log((T(1) + x) / (T(1) - x));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T,N> atanh(vector<T,N> x)
{
    VECTOR_MAP_UNARY(T, N, atanh, x);
}

//
// Section 8.2. Exponential Functions
//

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T inversesqrt(T x)
{
    return rsqrt(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T, N> inversesqrt(vector<T, N> x)
{
    return rsqrt(x);
}

//
// Section 8.3. Common Functions
//

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T fract(T x)
{
    return frac(x);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T, N> fract(vector<T, N> x)
{
    return frac(x);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T mod(T x, T y)
{
    return fmod(x, y);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T, N> mod(vector<T, N> x, T y)
{
    return fmod(x, vector<T, N>(y));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T, N> mod(vector<T, N> x, vector<T, N> y)
{
    return fmod(x, y);
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T mix(T x, T y, T a)
{
    return lerp(x, y, a);
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T, N> mix(vector<T, N> x, vector<T, N> y, T a)
{
    return lerp(x, y, vector<T, N>(a));
}

__generic<T : __BuiltinFloatingPointType, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T, N> mix(vector<T, N> x, vector<T, N> y, vector<T, N> a)
{
    return lerp(x, y, a);
}

__generic<T>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T mix(T x, T y, bool a)
{
    return (a ? y : x);
}

__generic<T, let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<T, N> mix(vector<T, N> x, vector<T, N> y, vector<bool, N> a)
{
    vector<T, N> result;
    for (int i = 0; i < N; i++)
    {
        result[i] = (a[i] ? y[i] : x[i]);
    }
    return result;
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public int floatBitsToInt(highp float x)
{
    return asint(x);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<int, N> floatBitsToInt(highp vector<float, N> x)
{
    return asint(x);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint floatBitsToUint(highp float x)
{
    return asuint(x);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint, N> floatBitsToUint(highp vector<float, N> x)
{
    return asuint(x);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public float intBitsToFloat(highp int x)
{
    return asfloat(x);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<float, N> intBitsToFloat(highp vector<int, N> x)
{
    return asfloat(x);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public float uintBitsToFloat(highp uint x)
{
    return asfloat(x);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<float, N> uintBitsToFloat(highp vector<uint, N> x)
{
    return asfloat(x);
}

//
// Section 8.5. Geometric Functions
//

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T faceforward(T n, T i, T ng)
{
    return dot(ng, i) < T(0.0f) ? n : -n;
}

__generic<T : __BuiltinFloatingPointType>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public T refract(T i, T n, T eta)
{
    let dotNI = dot(n,i);
    let k = T(1) - eta*eta*(T(1) - dotNI * dotNI);
    if(k < T(0)) return T(0);
    return eta * i - (eta * dotNI + sqrt(k)) * n;
}

//
// Section 8.6. Matrix Functions
//

//
// Section 8.8. Integer Functions
//

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint uaddCarry(highp uint x, highp uint y, out lowp uint carry)
{
    let result = x * y;
    carry = ((result < x || result < y) ? 1 : 0);
    return result;
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> uaddCarry(highp vector<uint,N> x, highp vector<uint,N> y, out lowp vector<uint,N> carry)
{
    VECTOR_MAP_TRINARY(uint, N, uaddCarry, x, y, carry);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint usubBorrow(highp uint x, highp uint y, out lowp uint borrow)
{
    borrow = (y > x) ? 1 : 0;
    return x - y;
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> usubBorrow(highp vector<uint,N> x, highp vector<uint,N> y, out lowp vector<uint,N> borrow)
{
    VECTOR_MAP_TRINARY(uint, N, usubBorrow, x, y, borrow);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public void umulExtended(highp uint x, highp uint y, out highp uint msb, out highp uint lsb)
{
    uint xLo = x & 0xffff;
    uint xHi = x >> 16;
    uint yLo = y & 0xffff;
    uint yHi = y >> 16;

    uint loProduct = xLo * yLo;
    uint midProduct1 = xLo * yHi;
    uint midProduct2 = xHi * yLo;
    uint hiProduct = xHi * yHi;

    uint carry1 = (midProduct1 >> 16) + (midProduct2 & 0xffff);
    uint carry2 = (loProduct >> 16) + (midProduct1 & 0xffff) + (carry1 & 0xffff);
    msb = hiProduct + (carry2 >> 16);
    lsb = (carry2 << 16) | (loProduct & 0xffff);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public void umulExtended(highp vector<uint,N> x, highp vector<uint,N> y, out highp vector<uint,N> msb, out highp vector<uint,N> lsb)
{
    for(int i = 0; i < N; ++i)
    {
       umulExtended(x[i], y[i], msb[i], lsb[i]);
    }
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public void imulExtended(highp int x, highp int y, out highp int msb, out highp int lsb)
{
    // Extract the sign bits
    int signX = (x >> 31) & 1;
    int signY = (y >> 31) & 1;

    // Make x and y positive
    int absX = (x ^ (signX << 31)) - signX;
    int absY = (y ^ (signY << 31)) - signY;

    // Calculate the product of the absolute values
    uint loProduct;
    int product = absX * absY;
    loProduct = product & 0xFFFFFFFF;

    // Calculate the high part
    msb = (product >> 32) + (signX ^ signY) * loProduct;
    lsb = loProduct;
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public void imulExtended(highp vector<int,N> x, highp vector<int,N> y, out highp vector<int,N> msb, out highp vector<int,N> lsb)
{
    for(int i = 0; i < N; ++i)
    {
       imulExtended(x[i], y[i], msb[i], lsb[i]);
    }
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public int bitfieldExtract(int value, int offset, int bits)
{
    return int(uint(value >> offset) & ((1u << bits) - 1));
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<int,N> bitfieldExtract(vector<int,N> value, int offset, int bits)
{
    vector<int,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldExtract(value[i], offset, bits);
    }
    return result;
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint bitfieldExtract(uint value, int offset, int bits)
{
    return (value >> offset) & ((1u << bits) - 1);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> bitfieldExtract(vector<uint,N> value, int offset, int bits)
{
    vector<uint,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldExtract(value[i], offset, bits);
    }
    return result;
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint bitfieldInsert(uint base, uint insert, int offset, int bits)
{
    uint clearMask = ~(((1u << bits) - 1u) << offset);
    uint clearedBase = base & clearMask;
    uint maskedInsert = (insert & ((1u << bits) - 1u)) << offset;
    return clearedBase | maskedInsert;
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> bitfieldInsert(vector<uint,N> base, vector<uint,N> insert, int offset, int bits)
{
    vector<uint,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldInsert(base[i], insert[i], offset, bits);
    }
    return result;
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public int bitfieldInsert(int base, int insert, int offset, int bits)
{
    uint clearMask = ~(((1u << bits) - 1u) << offset);
    uint clearedBase = base & clearMask;
    uint maskedInsert = (insert & ((1u << bits) - 1u)) << offset;
    return clearedBase | maskedInsert;
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<int,N> bitfieldInsert(vector<int,N> base, vector<int,N> insert, int offset, int bits)
{
    vector<int,N> result;
    for (int i = 0; i < N; ++i)
    {
        result[i] = bitfieldInsert(base[i], insert[i], offset, bits);
    }
    return result;
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public int bitfieldReverse(highp int value)
{
    value = ((value & 0xAAAAAAAA) >> 1) | ((value & 0x55555555) << 1);
    value = ((value & 0xCCCCCCCC) >> 2) | ((value & 0x33333333) << 2);
    value = ((value & 0xF0F0F0F0) >> 4) | ((value & 0x0F0F0F0F) << 4);
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    value = ((value & 0xFFFF0000) >> 16) | ((value & 0x0000FFFF) << 16);
    return value;
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<int,N> bitfieldReverse(highp vector<int,N> value)
{
    VECTOR_MAP_UNARY(int, N, bitfieldReverse, value);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint bitfieldReverse(highp uint value)
{
    value = ((value & 0xAAAAAAAA) >> 1) | ((value & 0x55555555) << 1);
    value = ((value & 0xCCCCCCCC) >> 2) | ((value & 0x33333333) << 2);
    value = ((value & 0xF0F0F0F0) >> 4) | ((value & 0x0F0F0F0F) << 4);
    value = ((value & 0xFF00FF00) >> 8) | ((value & 0x00FF00FF) << 8);
    value = ((value & 0xFFFF0000) >> 16) | ((value & 0x0000FFFF) << 16);
    return value;
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> bitfieldReverse(highp vector<uint,N> value)
{
    VECTOR_MAP_UNARY(int, N, bitfieldReverse, value);
}


__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint bitCount(uint value)
{
    return countbits(value);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> bitCount(vector<uint,N> value)
{
    VECTOR_MAP_UNARY(uint, N, bitCount, value);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public int bitCount(int value)
{
    return ((value >= 0) ? bitCount(uint(value)) : 1 + bitCount(uint(-value)));
}
    
__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<int,N> bitCount(vector<int,N> value)
{
    VECTOR_MAP_UNARY(int, N, bitCount, value);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public int findLSB(int v)
{
    return firstbitlow(v);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<int,N> findLSB(vector<int,N> value)
{
    return firstbitlow(value);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint findLSB(uint v)
{
    return firstbitlow(v);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> findLSB(vector<uint,N> value)
{
    return firstbitlow(value);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public int findMSB(int value)
{
    return firstbithigh(value);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<int,N> findMSB(vector<int,N> value)
{
    return firstbithigh(value);
}

__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public uint findMSB(uint value)
{
    return firstbithigh(value);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<uint,N> findMSB(vector<uint,N> value)
{
    return firstbithigh(value);
}

__generic<let N : int>
__target_intrinsic(glsl)
[__readNone]
[ForceInline]
public vector<bool,N> not(vector<bool,N> x)
{
    return !x;
}

//
// Ray tracing
//

public typealias rayQueryEXT = RayQuery;

__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
__target_intrinsic(glsl)
[ForceInline]
public void rayQueryConfirmIntersectionEXT(inout rayQueryEXT q)
{
    q.CommitNonOpaqueTriangleHit();
}

__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
__target_intrinsic(glsl)
[ForceInline]
public bool rayQueryProceedEXT(inout rayQueryEXT q)
{
    return q.Proceed();
}

__glsl_extension(GL_EXT_ray_query)
__glsl_version(460)
__target_intrinsic(glsl)
[__NoSideEffect]
public uint rayQueryGetIntersectionTypeEXT(rayQueryEXT q, bool committed)
{
    if (committed)
    {
        q.CommittedStatus();
    }
    else
    {
        q.CandidateType();
    }
    return 0;
}


//
// Subgroup
//

__glsl_extension(KHR_shader_subgroup)
__glsl_version(450)
__target_intrinsic(glsl)
public void subgroupBarrier()
{
    __subgroupBarrier();
}

__glsl_extension(KHR_shader_subgroup)
__glsl_version(450)
__target_intrinsic(glsl)
public void subgroupMemoryBarrier()
{
}

