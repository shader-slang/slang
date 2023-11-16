//
// From the GLSL spec, section 4.1. 'asic Types'
//

typealias vec2 = vector<float, 2>;
typealias vec3 = vector<float, 3>;
typealias vec4 = vector<float, 4>;

typealias dvec2 = vector<double, 2>;
typealias dvec3 = vector<double, 3>;
typealias dvec4 = vector<double, 4>;

typealias bvec2 = vector<bool, 2>;
typealias bvec3 = vector<bool, 3>;
typealias bvec4 = vector<bool, 4>;

typealias ivec2 = vector<int, 2>;
typealias ivec3 = vector<int, 3>;
typealias ivec4 = vector<int, 4>;

typealias uvec2 = vector<uint, 2>;
typealias uvec3 = vector<uint, 3>;
typealias uvec4 = vector<uint, 4>;

typealias i8vec2 = vector<int8_t, 2>;
typealias i8vec3 = vector<int8_t, 3>;
typealias i8vec4 = vector<int8_t, 4>;

typealias u8vec2 = vector<uint8_t, 2>;
typealias u8vec3 = vector<uint8_t, 3>;
typealias u8vec4 = vector<uint8_t, 4>;

typealias i16vec2 = vector<int16_t, 2>;
typealias i16vec3 = vector<int16_t, 3>;
typealias i16vec4 = vector<int16_t, 4>;

typealias u16vec2 = vector<uint16_t, 2>;
typealias u16vec3 = vector<uint16_t, 3>;
typealias u16vec4 = vector<uint16_t, 4>;

typealias i64vec2 = vector<int64_t, 2>;
typealias i64vec3 = vector<int64_t, 3>;
typealias i64vec4 = vector<int64_t, 4>;

typealias u64vec2 = vector<uint64_t, 2>;
typealias u64vec3 = vector<uint64_t, 3>;
typealias u64vec4 = vector<uint64_t, 4>;

typealias mat2 = matrix<float, 2, 2>;
typealias mat3 = matrix<float, 3, 3>;
typealias mat4 = matrix<float, 4, 4>;

typealias mat2x2 = matrix<float, 2, 2>;
typealias mat2x3 = matrix<float, 3, 2>;
typealias mat2x4 = matrix<float, 4, 2>;

typealias mat3x2 = matrix<float, 2, 3>;
typealias mat3x3 = matrix<float, 3, 3>;
typealias mat3x4 = matrix<float, 4, 3>;

typealias mat4x2 = matrix<float, 2, 4>;
typealias mat4x3 = matrix<float, 3, 4>;
typealias mat4x4 = matrix<float, 4, 4>;

typealias dmat2 = matrix<double, 2, 2>;
typealias dmat3 = matrix<double, 3, 3>;
typealias dmat4 = matrix<double, 4, 4>;

typealias dmat2x2 = matrix<double, 2, 2>;
typealias dmat2x3 = matrix<double, 3, 2>;
typealias dmat2x4 = matrix<double, 4, 2>;

typealias dmat3x2 = matrix<double, 2, 3>;
typealias dmat3x3 = matrix<double, 3, 3>;
typealias dmat3x4 = matrix<double, 4, 3>;

typealias dmat4x2 = matrix<double, 2, 4>;
typealias dmat4x3 = matrix<double, 3, 4>;
typealias dmat4x4 = matrix<double, 4, 4>;

typealias usampler1D = Sampler1D<uint4>;
typealias isampler1D = Sampler1D<int4>;
typealias sampler1D = Sampler1D<float4>;

typealias usampler2D = Sampler2D<uint4>;
typealias isampler2D = Sampler2D<int4>;
typealias sampler2D = Sampler2D<float4>;

typealias usampler3D = Sampler3D<uint4>;
typealias isampler3D = Sampler3D<int4>;
typealias sampler3D = Sampler3D<float4>;

typealias usamplerCube = SamplerCube<uint4>;
typealias isamplerCube = SamplerCube<int4>;
typealias samplerCube = SamplerCube<float4>;

typealias Sampler1DShadow<T=float> = __TextureImpl<T, __Shape1D, /*isArray:*/ 0, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
typealias usampler1DShadow = Sampler1DShadow<uint>;
typealias isampler1DShadow = Sampler1DShadow<int>;
typealias sampler1DShadow = Sampler1DShadow<float>;

typealias Sampler2DShadow<T=float> = __TextureImpl<T, __Shape2D, /*isArray:*/ 0, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
typealias usampler2DShadow = Sampler2DShadow<uint>;
typealias isampler2DShadow = Sampler2DShadow<int>;
typealias sampler2DShadow = Sampler2DShadow<float>;

typealias SamplerCubeShadow<T=float> = __TextureImpl<T, __ShapeCube, /*isArray:*/ 0, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
typealias usamplerCubeShadow = SamplerCubeShadow<uint>;
typealias isamplerCubeShadow = SamplerCubeShadow<int>;
typealias samplerCubeShadow = SamplerCubeShadow<float>;

typealias usampler1DArray = Sampler1DArray<uint4>;
typealias isampler1DArray = Sampler1DArray<int4>;
typealias sampler1DArray = Sampler1DArray<float4>;

typealias usampler2DArray = Sampler2DArray<uint4>;
typealias isampler2DArray = Sampler2DArray<int4>;
typealias sampler2DArray = Sampler2DArray<float4>;

typealias usamplerCubeArray = SamplerCubeArray<uint4>;
typealias isamplerCubeArray = SamplerCubeArray<int4>;
typealias samplerCubeArray = SamplerCubeArray<float4>;

typealias Sampler1DArrayShadow<T=float> = __TextureImpl<T, __Shape1D, /*isArray:*/ 1, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
typealias usampler1DArrayShadow = Sampler1DArrayShadow<uint>;
typealias isampler1DArrayShadow = Sampler1DArrayShadow<int>;
typealias sampler1DArrayShadow = Sampler1DArrayShadow<float>;

typealias Sampler2DArrayShadow<T=float> = __TextureImpl<T, __Shape2D, /*isArray:*/ 1, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
typealias usampler2DArrayShadow = Sampler2DArrayShadow<uint>;
typealias isampler2DArrayShadow = Sampler2DArrayShadow<int>;
typealias sampler2DArrayShadow = Sampler2DArrayShadow<float>;

typealias SamplerCubeArrayShadow<T=float> = __TextureImpl<T, __ShapeCube, /*isArray:*/ 1, /*isMS:*/ 0, /*sampleCount:*/ 0, /*access:*/ 0, /*isShadow: */ 1, /*isCombined: */ 1, /*format*/ 0>;
typealias usamplerCubeArrayShadow = SamplerCubeArrayShadow<uint>;
typealias isamplerCubeArrayShadow = SamplerCubeArrayShadow<int>;
typealias samplerCubeArrayShadow = SamplerCubeArrayShadow<float>;

[ForceInline]
vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler1D<vector<T,N>> sampler, int p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int2(p, lod)));
}

[ForceInline]
vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler2D<vector<T,N>> sampler, ivec2 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int3(p, lod)));
}

[ForceInline]
vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler3D<vector<T,N>> sampler, ivec3 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int4(p, lod)));
}

[ForceInline]
vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler1DArray<vector<T,N>> sampler, ivec2 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int3(p, lod)));
}

[ForceInline]
vector<T,4> texelFetch<T:__BuiltinArithmeticType, let N : int> (Sampler2DArray<vector<T,N>> sampler, ivec3 p, int lod)
{
    return __vectorReshape<4>(sampler.Load(int4(p, lod)));
}

[ForceInline]
vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler1D<vector<T,N>> sampler, float p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler2D<vector<T,N>> sampler, float2 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler3D<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (SamplerCube<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
float texture<T:__BuiltinArithmeticType, let N : int> (Sampler1DShadow<vector<T,N>> sampler, float2 p)
{
    return sampler.SampleCmp(p.x, p.y);
}

[ForceInline]
float texture<T:__BuiltinArithmeticType, let N : int> (Sampler2DShadow<vector<T,N>> sampler, float3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
float texture<T:__BuiltinArithmeticType, let N : int> (SamplerCubeShadow<vector<T,N>> sampler, float4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

[ForceInline]
vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler1DArray<vector<T,N>> sampler, float2 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler2DArray<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (SamplerCubeArray<vector<T,N>> sampler, float4 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
float texture<T:__BuiltinArithmeticType, let N : int> (Sampler1DArrayShadow<vector<T,N>> sampler, float3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
float texture<T:__BuiltinArithmeticType, let N : int> (Sampler2DArrayShadow<vector<T,N>> sampler, float4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

__generic<T:__BuiltinArithmeticType, let N : int, shape:__ITextureShape, let sampleCount: int, let isArray:int, let format:int>
vector<T,4> textureGrad(__TextureImpl<vector<T,N>, shape, isArray, 0, sampleCount, 0, 1, 1, format> sampler, vector<float, shape.dimensions+isArray> P, vector<float,shape.planeDimensions> dPdx, vector<float,shape.planeDimensions> dPdy)
{
    return __vectorReshape<4>(sampler.SampleGrad(P, dPdx, dPdy));
}

out float4 gl_Position : SV_Position;
out float gl_PointSize : SV_PointSize;
in vec4 gl_FragCoord : SV_Position;
out float gl_FragDepth : SV_Depth;
out int gl_FragStencilRef : SV_StencilRef;

in uvec3 gl_GlobalInvocationID : SV_DispatchThreadID;
in uvec3 gl_WorkGroupID : SV_GroupID;
in uvec3 gl_LocalInvocationIndex : SV_GroupIndex;
in uvec3 gl_LocalInvocationID : SV_GroupThreadID;

// TODO: define overload for tessellation control stage.
in int gl_InvocationID : SV_GSInstanceID;

in int gl_InstanceIndex : SV_InstanceID;
in bool gl_FrontFacing : SV_IsFrontFace;

// TODO: define overload for geometry stage.
in int gl_Layer : SV_RenderTargetArrayIndex;

in int gl_SampleID : SV_SampleIndex;
in int gl_VertexIndex : SV_VertexID;
in int gl_ViewIndex : SV_ViewID;
in int gl_ViewportIndex : SV_ViewportArrayIndex;


// Override operator* behavior to compute algebric product of matrices and vectors.

[OverloadRank(15)]
[ForceInline]
matrix<float, N, N> operator*<let N : int>(matrix<float, N, N> m1, matrix<float, N, N> m2)
{
    return mul(m2, m1);
}

[OverloadRank(15)]
[ForceInline]
matrix<half, N, N> operator*<let N : int>(matrix<half, N, N> m1, matrix<half, N, N> m2)
{
    return mul(m2, m1);
}

[OverloadRank(15)]
[ForceInline]
matrix<double, N, N> operator*<let N : int>(matrix<double, N, N> m1, matrix<double, N, N> m2)
{
    return mul(m2, m1);
}

[ForceInline]
[OverloadRank(15)]
matrix<T, R, L> operator*<T:__BuiltinFloatingPointType, let L : int, let C : int, let R : int>(matrix<T, C, L> m1, matrix<T, R, C> m2)
{
    return mul(m2, m1);
}

[ForceInline]
[OverloadRank(15)]
vector<T, R> operator*<T:__BuiltinFloatingPointType, let C : int, let R : int>(vector<T, C> v, matrix<T, R, C> m)
{
    return mul(m, v);
}

[ForceInline]
[OverloadRank(15)]
vector<T, C> operator*<T:__BuiltinFloatingPointType, let C : int, let R : int>(matrix<T, R, C> m, vector<T, R> v)
{
    return mul(v, m);
}

__intrinsic_op(mul)
matrix<T, N, M> matrixCompMult<T:__BuiltinFloatingPointType, let N : int, let M : int>(matrix<T,N,M> left, matrix<T,N,M> right);

__intrinsic_op(cmpLE)
vector<bool, N> lessThanEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpLT)
vector<bool, N> lessThan<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpGT)
vector<bool, N> greaterThan<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpGE)
vector<bool, N> greaterThanEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpEQ)
vector<bool, N> equal<T, let N:int>(vector<T, N> x, vector<T, N> y);
__intrinsic_op(cmpNE)
vector<bool, N> notEqual<T, let N:int>(vector<T, N> x, vector<T, N> y);

__generic<T>
extension vector<T, 2>
{
    [ForceInline] __init(vector<T, 3> bigger) { this = bigger.xy; }
    [ForceInline] __init(vector<T, 4> bigger) { this = bigger.xy; }
}

__generic<T>
extension vector<T, 3>
{
    [ForceInline] __init(vector<T, 4> bigger) { this = bigger.xyz; }
}

[ForceInline]
[OverloadRank(15)]
bool operator==<T:__BuiltinArithmeticType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(15)]
bool operator!=<T:__BuiltinArithmeticType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

[ForceInline]
[OverloadRank(14)]
bool operator==<T:__BuiltinFloatingPointType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(14)]
bool operator!=<T:__BuiltinFloatingPointType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return any(notEqual(left, right));
}

[ForceInline]
[OverloadRank(14)]
bool operator==<T:__BuiltinLogicalType, let N : int>(vector<T, N> left, vector<T, N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(14)]
bool operator!=<T:__BuiltinLogicalType, let N : int>(vector<T, N> left, vector<T, N> right)
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
bool operator==<let N : int>(vector<$(typeName), N> left, vector<$(typeName), N> right)
{
    return all(equal(left, right));
}

[ForceInline]
[OverloadRank(15)]
bool operator!=<let N : int>(vector<$(typeName), N> left, vector<$(typeName), N> right)
{
    return any(notEqual(left, right));
}
${{{{
}
}}}}

[ForceInline] int findLSB(int v) { return firstbitlow(v); }
[ForceInline] uint findLSB(uint v) { return firstbitlow(v); }
[ForceInline] vector<int,N> findLSB<let N:int>(vector<int,N> value)
{
    return firstbitlow(value);
}
[ForceInline] vector<uint,N> findLSB<let N:int>(vector<uint,N> value)
{
    return firstbitlow(value);
}
