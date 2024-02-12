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

// TODO: implementation of built-in variables; proper tests; these are stubs
// likley related to the following issue since GLSL adds new 
// 'system' variables: https://github.com/shader-slang/slang/issues/411

#define glsl_caps_shader_subgroup(param1) __glsl_extension(param1) [require(glsl)]
#define spirv_caps_shader_subgroup(param1) __spirv_version(param1) [require(spirv)]

//macros used to make implementation and modification reasonable under regex
#define macroAnyVecGenericBuiltIn __generic<T : __BuiltinType, let N : int>
#define macroAnyVecGenericInt __generic<T : __BuiltinIntegerType, let N : int>
#define macroAnyVecGenericArithmetic __generic<T : __BuiltinArithmeticType, let N : int>
#define macroAnyVec vector<T, N>

__generic<T : __BuiltinType>
void shader_subgroup_preamble() {
    // checks needed for shader_subgroup functions; __requireGLSLExtension does not work, the call fails to find the OP
    //__target_switch
    //{
    //case glsl:
        //if (__type_equals<T, half>()) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_float16");
        //if (__type_equals<T, uint8_t>()) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_int8");
        //if (__type_equals<T, uint16_t>()) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_int16");
        //if (__type_equals<T, uint64_t>()) __requireGLSLExtension("GL_EXT_shader_subgroup_extended_types_int64");
        //return;
    //}
    return;
} 

// GL_KHR_shader_subgroup_basic Built-in Variables 

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_NumSubgroups;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupID;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupSize;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupInvocationID;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupEqMask;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupGeMask;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupGtMask;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupLeMask;

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupLtMask;

// GL_KHR_shader_subgroup_basic

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public void subgroupBarrier()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__syncwarp()";
    case hlsl:
        __intrinsic_asm "AllMemoryBarrierWithGroupSync()";
    case glsl:
        __intrinsic_asm "subgroupBarrier()";
    case spirv:
        spirv_asm {
            OpControlBarrier Subgroup Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
        };
        spirv_asm {
            OpControlBarrier Subgroup Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
        };
    case cpp:
        // TODO: cpp
        return;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public void subgroupMemoryBarrier()
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public void subgroupMemoryBarrier()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "AllMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrier()";
    case spirv:
        spirv_asm {
            OpMemoryBarrier Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
        };
        spirv_asm {
            OpMemoryBarrier Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
        };
    case cpp:
        // TODO: cpp
        return;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public void subgroupMemoryBarrierBuffer()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "DeviceMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrierBuffer()";
    case spirv:
        spirv_asm {
            OpMemoryBarrier Subgroup AcquireRelease|UniformMemory
        };
    case cpp:
        // TODO: cpp
        return;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public void subgroupMemoryBarrierImage()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "DeviceMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrierImage()";
    case spirv:
        spirv_asm {
            OpMemoryBarrier Subgroup AcquireRelease|ImageMemory
        };
    case cpp:
        // TODO: cpp
        return;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public void subgroupMemoryBarrierShared()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "__threadfence_block()";
    case hlsl:
        __intrinsic_asm "GroupMemoryBarrier()";
    case glsl:
        __intrinsic_asm "subgroupMemoryBarrierShared()";
    case spirv:
        spirv_asm {
            OpMemoryBarrier Subgroup AcquireRelease|SubgroupMemory
        };
    case cpp:
        // TODO: cpp
        return;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_basic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public bool subgroupElect()
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "( (__activemask() & (__activemask()*-1)) == _getLaneId())";    
    case glsl:
    case spirv:
    case hlsl:
        return WaveIsFirstLane();
    case cpp:
        // TODO: cpp
        return false;
    }
}

// GL_KHR_shader_subgroup_vote

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_vote)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public bool subgroupAll(bool value)
{
    __target_switch
    {
    case cuda: 
    case glsl:
    case spirv:
    case hlsl:
        return WaveActiveAllTrue(value);
    case cpp:
        // TODO: cpp
        return false;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_vote)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public bool subgroupAny(bool value)
{
    return WaveMaskAnyTrue(WaveGetActiveMask(), value);
    
}

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_vote)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public bool subgroupAllEqual(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskAllEqual(WaveGetActiveMask(), value);
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_vote)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public bool subgroupAllEqual(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskAllEqual(WaveGetActiveMask(), value);
}

// GL_KHR_shader_subgroup_arithmetic

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupAdd(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskSum(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupMul(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskProduct(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupMin(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskMin(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupMax(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskMax(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupAnd(T value)
__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupAnd(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBitAnd(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupOr(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBitOr(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupXor(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBitXor(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupInclusiveAdd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveAdd($0)";
        __target_intrinsic "subgroupInclusiveAdd($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$T result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$T result Subgroup InclusiveScan $value};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
        // TODO: implementation
        return value;
    }
    return value;
    return value;
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupInclusiveMul(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveMul($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$T result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$T result Subgroup InclusiveScan $value};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupInclusiveMin(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveMin($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$T result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$T result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$T result Subgroup InclusiveScan $value};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupInclusiveMax(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveMax($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$T result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$T result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$T result Subgroup InclusiveScan $value};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupInclusiveAnd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveAnd($0)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$T result Subgroup InclusiveScan $value};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupInclusiveOr(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveOr($0)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseOr $$T result Subgroup InclusiveScan $value};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupInclusiveXor(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveXor($0)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$T result Subgroup InclusiveScan $value};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupExclusiveAdd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixSum(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}


__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupExclusiveMul(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixProduct(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupExclusiveMin(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupExclusiveMax(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupExclusiveAnd(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixBitAnd(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupExclusiveOr(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixBitOr(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupExclusiveXor(T value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixBitXor(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

// GL_KHR_shader_subgroup_arithmetic
//note: this is a seperate section because it is so huge that the only reasonable way to implement this is to just regex replace code


macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupAdd(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskSum(WaveGetActiveMask(), value);
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupMul(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskProduct(WaveGetActiveMask(), value);
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupMin(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskMin(WaveGetActiveMask(), value);
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupMax(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskMax(WaveGetActiveMask(), value);
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupAnd(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBitAnd(WaveGetActiveMask(), value);
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupOr(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBitOr(WaveGetActiveMask(), value);
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupXor(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBitXor(WaveGetActiveMask(), value);
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupInclusiveAdd(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveAdd($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$macroAnyVec result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$macroAnyVec result Subgroup InclusiveScan $value};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupInclusiveMul(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveMul($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$macroAnyVec result Subgroup InclusiveScan $value};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIMul $$macroAnyVec result Subgroup InclusiveScan $value};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupInclusiveMin(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveMin($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMin $$macroAnyVec result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMin $$macroAnyVec result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMin $$macroAnyVec result Subgroup InclusiveScan $value};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupInclusiveMax(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveMax($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMax $$macroAnyVec result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformSMax $$macroAnyVec result Subgroup InclusiveScan $value};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformUMax $$macroAnyVec result Subgroup InclusiveScan $value};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupInclusiveAnd(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveAnd($0)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseAnd $$macroAnyVec result Subgroup InclusiveScan $value};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupInclusiveOr(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveOr($0)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseOr $$macroAnyVec result Subgroup InclusiveScan $value};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupInclusiveXor(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveXor($0)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformBitwiseXor $$macroAnyVec result Subgroup InclusiveScan $value};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupExclusiveAdd(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixSum(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}


macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupExclusiveMul(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixProduct(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupExclusiveMin(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupExclusiveMax(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupExclusiveAnd(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixBitAnd(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupExclusiveOr(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixBitOr(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_arithmetic)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupExclusiveXor(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
    case spirv:
    case hlsl:
        return WaveMaskPrefixBitXor(WaveGetActiveMask(), value);
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
}

// GL_KHR_shader_subgroup_ballot

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupBroadcast(T value, uint id)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, id);
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupBroadcast(macroAnyVec value, uint id)
{
    shader_subgroup_preamble<T>();
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, id);
}

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupBroadcastFirst(T value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), value);
}
macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupBroadcastFirst(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), value);
}

// WaveMaskBallot is not the same; it force trunc's
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public uvec4 subgroupBallot(bool value)
{
    return WaveActiveBallot(value);
}

// logic for HLSL and CUDA which lack InverseBalloc
// CUDA: works exclusivly 32 waves, therefore only need comp x
// HLSL:{
// 1. index into comp I want: index = trunc(float(lane)*(1/32))
// 2. lane & value[index]
// note: 1/32 wil be converted to multiplication
// we do 1/32 since 1 uint stores 32 threads 
// note 2: we have a waveLaneCount check because based on wave lane count we can determine if we can do a 
// fast path or slow path (know index is 0 or non 0)
// }
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public bool subgroupInverseBallot(uvec4 value)
{
    __target_switch
    {
    case cuda:
        // only has 32 warps
        __intrinsic_asm "(($0).x >> (c()) & 1)";
    case hlsl:
        // much like _WaveCountBits, but here we hope that we hit case 0; we can then avoid the expensive logic
        const uint waveLaneCount = WaveGetLaneCount();
        switch ((waveLaneCount - 1) / 32)
        {
        case 0:
            __intrinsic_asm "(($0)[0] >> WaveGetLaneIndex()) & 1)";
        case 1:
        case 2:
        case 3:
            __intrinsic_asm "((($0)[uint(float(WaveGetLaneIndex())*0.03125f)] >> WaveGetLaneIndex()) & 1)";
        }
    case glsl:
        __intrinsic_asm "subgroupInverseBallot($0)";
    case spirv:
        return spirv_asm {
                OpCapability GroupNonUniformBallot; 
                OpGroupNonUniformInverseBallot $$bool result Subgroup $value
        };
    case cpp:
        // TODO: cpp
        return false;
    }
    return false;
}

// same logic as subgroupInverseBallot
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public bool subgroupBallotBitExtract(uvec4 value, uint index)
{
    __target_switch
    {
    case cuda:
        __intrinsic_asm "($1 & ($0).x) != 0";
    case hlsl:
        const uint waveLaneCount = WaveGetLaneCount();
        switch ((waveLaneCount - 1) / 32)
        {
        case 0:
            __intrinsic_asm "($0)[0] & ($1)";
        case 1:
        case 2:
        case 3:
            __intrinsic_asm "($0)[uint(float($1)*0.03125f)] & ($1)";
        }
    case glsl:
        __intrinsic_asm "subgroupBallotBitExtract($0, $1)";
    case spirv:
        return spirv_asm {
                OpCapability GroupNonUniformBallot; 
                OpGroupNonUniformBallotBitExtract $$bool result Subgroup $value $index
        };
    case cpp:
        // TODO: cpp
        return false;
    }
    return false;
}


// the count is only supposed to use uvec4 values within bottom bits of subgroup launched, not a simple countbits
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public uint subgroupBallotBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotBitCount($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup Reduce $value
        };
    case hlsl:
    case cuda:
    case cpp:
        //TODO: implement
        return 0;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public uint subgroupBallotInclusiveBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotInclusiveBitCount($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup InclusiveScan $value
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public uint subgroupBallotExclusiveBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotExclusiveBitCount($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup ExclusiveScan $value
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public uint subgroupBallotFindLSB(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotFindLSB($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotFindLSB $$uint result Subgroup $value
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_ballot)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public uint subgroupBallotFindMSB(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotFindMSB($0)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotFindMSB $$uint result Subgroup $value
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

// GL_KHR_shader_subgroup_shuffle

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupShuffle(T value, uint index)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case hlsl:
    case cuda:
    case glsl:
    case spirv:
    case cpp: 
        return WaveShuffle(value, index);
    }
}

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupShuffleXor(T value, uint mask)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupShuffleXor($0,$1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleXor $$T result Subgroup $value $mask
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupShuffle(macroAnyVec value, uint index)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case hlsl:
    case cuda:
    case glsl:
    case spirv:
    case cpp: 
        return WaveShuffle(value, index);
    }
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupShuffleXor(macroAnyVec value, uint mask)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupShuffleXor($0,$1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleXor $$macroAnyVec result Subgroup $value $mask
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}


// GL_KHR_shader_subgroup_shuffle_relative

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle_relative)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupShuffleUp(T value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupShuffleUp($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleUp $$T result Subgroup $value $delta
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle_relative)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupShuffleDown(T value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupShuffleDown($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleDown $$T result Subgroup $value $delta
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}


macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle_relative)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupShuffleUp(macroAnyVec value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupShuffleUp($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleUp $$macroAnyVec result Subgroup $value $delta
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_shuffle_relative)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupShuffleDown(macroAnyVec value, uint delta)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupShuffleDown($0, $1)";
    case spirv:
        return spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformShuffleDown $$macroAnyVec result Subgroup $value $delta
        };
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}
// GL_KHR_shader_subgroup_clustered

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupClusteredAdd(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredAdd($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFAdd $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIAdd $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupClusteredMul(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredMul($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMul $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIMul $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    } 
    return value;
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupClusteredMin(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredMin($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMin $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMin $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformUMin $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinArithmeticType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupClusteredMax(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredMax($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMax $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMax $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered;  OpGroupNonUniformUMax $$T result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupClusteredAnd(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredAnd($0, $1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseAnd $$T result Subgroup ClusteredReduce $value $clusterSize};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupClusteredOr(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredOr($0, $1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseOr $$T result Subgroup ClusteredReduce $value $clusterSize};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}



__generic<T : __BuiltinIntegerType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupClusteredXor(T value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredXor($0, $1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseXor $$T result Subgroup ClusteredReduce $value $clusterSize};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}



macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupClusteredAdd(macroAnyVec value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredAdd($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; 
            OpGroupNonUniformFAdd $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIAdd $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupClusteredMul(macroAnyVec value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredMul($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMul $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformIMul $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    } 
    return value;
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupClusteredMin(macroAnyVec value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredMin($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMin $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMin $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformUMin $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericArithmetic
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupClusteredMax(macroAnyVec value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredMax($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMax $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformSMax $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered;  OpGroupNonUniformUMax $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
        else return value;
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupClusteredAnd(macroAnyVec value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredAnd($0, $1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseAnd $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupClusteredOr(macroAnyVec value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredOr($0, $1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseOr $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

macroAnyVecGenericInt
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_clustered)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupClusteredXor(macroAnyVec value, uint clusterSize)
{
    shader_subgroup_preamble<T>();
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredXor($0, $1)";
    case spirv:
        return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformBitwiseXor $$macroAnyVec result Subgroup ClusteredReduce $value $clusterSize};
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return value;
    }
    return value;
}

// GL_KHR_shader_subgroup_quad

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupQuadBroadcast(T value, uint id)
{
    shader_subgroup_preamble<T>();
    return QuadReadLaneAt(value, id);
}

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupQuadSwapHorizontal(T value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossX(value);
}

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupQuadSwapVertical(T value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossY(value);
}

__generic<T : __BuiltinType>
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public T subgroupQuadSwapDiagonal(T value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossDiagonal(value);
}


macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupQuadBroadcast(macroAnyVec value, uint id)
{
    shader_subgroup_preamble<T>();
    return QuadReadLaneAt(value, id);
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupQuadSwapHorizontal(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossX(value);
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupQuadSwapVertical(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossY(value);
}

macroAnyVecGenericBuiltIn
glsl_caps_shader_subgroup(GL_KHR_shader_subgroup_quad)
spirv_caps_shader_subgroup(1.3)
[ForceInline] public macroAnyVec subgroupQuadSwapDiagonal(macroAnyVec value)
{
    shader_subgroup_preamble<T>();
    return QuadReadAcrossDiagonal(value);
}




