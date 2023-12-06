//
// From the GLSL spec, section 4.1. 'asic Types'
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
public vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler1D<vector<T,N>> sampler, float p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler2D<vector<T,N>> sampler, float2 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler3D<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (SamplerCube<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public float texture<T:__BuiltinArithmeticType, let N : int> (Sampler1DShadow<vector<T,N>> sampler, float2 p)
{
    return sampler.SampleCmp(p.x, p.y);
}

[ForceInline]
public float texture<T:__BuiltinArithmeticType, let N : int> (Sampler2DShadow<vector<T,N>> sampler, float3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture<T:__BuiltinArithmeticType, let N : int> (SamplerCubeShadow<vector<T,N>> sampler, float4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler1DArray<vector<T,N>> sampler, float2 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (Sampler2DArray<vector<T,N>> sampler, float3 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public vector<T,4> texture<T:__BuiltinArithmeticType, let N : int> (SamplerCubeArray<vector<T,N>> sampler, float4 p, float bias = 0.0)
{
    return __vectorReshape<4>(sampler.SampleBias(p, bias));
}

[ForceInline]
public float texture<T:__BuiltinArithmeticType, let N : int> (Sampler1DArrayShadow<vector<T,N>> sampler, float3 p)
{
    return sampler.SampleCmp(p.xy, p.z);
}

[ForceInline]
public float texture<T:__BuiltinArithmeticType, let N : int> (Sampler2DArrayShadow<vector<T,N>> sampler, float4 p)
{
    return sampler.SampleCmp(p.xyz, p.w);
}

__generic<T:__BuiltinArithmeticType, let N : int, shape:__ITextureShape, let sampleCount: int, let isArray:int, let format:int>
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

[ForceInline] public int findLSB(int v) { return firstbitlow(v); }
[ForceInline] public uint findLSB(uint v) { return firstbitlow(v); }
[ForceInline] public vector<int,N> findLSB<let N:int>(vector<int,N> value)
{
    return firstbitlow(value);
}
[ForceInline] public vector<uint,N> findLSB<let N:int>(vector<uint,N> value)
{
    return firstbitlow(value);
}
