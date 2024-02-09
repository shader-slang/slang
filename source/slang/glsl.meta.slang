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

//// GL_KHR_shader_subgroup 

// GL_KHR_shader_subgroup_*/GLSL ref: https://github.com/KhronosGroup/GLSL/blob/main/extensions/khr/GL_KHR_shader_subgroup.txt
// also: https://www.khronos.org/blog/vulkan-subgroup-tutorial
// also: https://www.khronos.org/assets/uploads/developers/library/2018-vulkan-devday/06-subgroups.pdf
//
// HLSL ref: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-intrinsic-functions
// also: https://github.com/Microsoft/DirectXShaderCompiler/wiki/Wave-Intrinsics
//
// CUDA ref: https://docs.nvidia.com/cuda/cuda-c-programming-guide/index.html
//
// SPIR-V ref: https://registry.khronos.org/SPIR-V/specs/unified1/SPIRV.html#_memory_semantics_id

/// GL_KHR_shader_subgroup Built-In Variables
// TODO: implementation of built-in variables; proper tests; these are stubs
// likley related to the following issue since GLSL adds new 
// 'system' variables: https://github.com/shader-slang/slang/issues/411
__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_NumSubgroups;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupID;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupSize;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupInvocationID;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupEqMask;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupGeMask;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupGtMask;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupLeMask;

__glsl_extension(GL_KHR_shader_subgroup_basic)
public uint gl_SubgroupLtMask;
/// ~GL_KHR_shader_subgroup Built-In Variables

/// GL_KHR_shader_subgroup Functions
/*
the cuda code is illistrated by the following c++:
1. (active & -active) gets the lowest non 0 value
2. now I need to check if "your lane" == lowest non 0 value
```
#include <iostream>
#include <stdint.h>

//NOTE: active implies that the thread is running;
//if a divergent branch is taken, the thread is INACTIVE

uint32_t active = 0b1010;
uint32_t v1 = 2;
uint32_t v2 = 4;

uint32_t isMin(uint32_t u){
    return (active & -active) == u;
}
int main(){
    printf("%b\n", isMin(v1));
    printf("%b\n", isMin(v2));
}
```
*/
[ForceInline] bool __isMinimumWarpActiveInBranchCUDA() {
    __intrinsic_asm "( (__activemask() & __activemask()*-1) == _getLaneId())";
}

__spirv_version(1.3)
[ForceInline] void __subgroupBarrierSPIRV()
{
    spirv_asm {
        OpControlBarrier Subgroup Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
    };
}
__spirv_version(1.3)
[ForceInline] void __subgroupMemoryBarrierSPIRV()
{
    spirv_asm {
        OpMemoryBarrier Subgroup AcquireRelease|SubgroupMemory|ImageMemory|UniformMemory
    };
}
__spirv_version(1.3)
[ForceInline] void __subgroupMemoryBarrierBufferSPIRV()
{
    spirv_asm {
        OpMemoryBarrier Subgroup AcquireRelease|UniformMemory
    };
}
__spirv_version(1.3)
[ForceInline] void __subgroupMemoryBarrierSharedSPIRV()
{
    spirv_asm {
        OpMemoryBarrier Subgroup AcquireRelease|SubgroupMemory
    };
}
__spirv_version(1.3)
[ForceInline] void __subgroupMemoryBarrierImageSPIRV()
{
    spirv_asm {
        OpMemoryBarrier Subgroup AcquireRelease|ImageMemory
    };
}

__spirv_version(1.3)
[ForceInline] bool __subgroupAllEqualSPIRV(bool value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformVote;
            OpGroupNonUniformAllEqual $$bool result Subgroup $value
    }).x;
}

__generic<T : __BuiltinType>
__spirv_version(1.3)
[ForceInline] T __subgroupBroadcastSPIRV(T value, uint id)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBroadcast $$T result Subgroup $value $id
    });
}

__generic<T : __BuiltinType>
__spirv_version(1.3)
[ForceInline] T __subgroupBroadcastFirstSPIRV(T value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBroadcastFirst $$T result Subgroup $value
    });
}

__spirv_version(1.3)
[ForceInline] uvec4 __subgroupBallotSPIRV(bool value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallot $$uint4 result Subgroup $value
    });
}

__spirv_version(1.3)
[ForceInline] bool __subgroupInverseBallotSPIRV(uvec4 value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformInverseBallot $$bool result Subgroup $value
    });
}

__spirv_version(1.3)
[ForceInline] bool __subgroupBallotBitExtractSPIRV(uvec4 value, uint index)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitExtract $$bool result Subgroup $value $index
    });
}

__spirv_version(1.3)
[ForceInline] uint __subgroupBallotBitCountSPIRV(uvec4 value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup Reduce $value
    });
}

__spirv_version(1.3)
[ForceInline] uint __subgroupBallotInclusiveBitCountSPIRV(uvec4 value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup InclusiveScan $value
    });
}

__spirv_version(1.3)
    [ForceInline] uint __subgroupBallotExclusiveBitCountSPIRV(uvec4 value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotBitCount $$uint result Subgroup ExclusiveScan $value
    });
}

__spirv_version(1.3)
[ForceInline] uint __subgroupBallotFindLSBSPIRV(uvec4 value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotFindLSB $$uint result Subgroup $value
    });
}

__spirv_version(1.3)
[ForceInline] uint __subgroupBallotFindMSBSPIRV(uvec4 value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotFindMSB $$uint result Subgroup $value
    });
}

__spirv_version(1.3)
[ForceInline] uint __subgroupShuffleDownSPIRV(uvec4 value)
{
    return (spirv_asm {
            OpCapability GroupNonUniformBallot; 
            OpGroupNonUniformBallotFindMSB $$uint result Subgroup $value
    });
}

// GL_KHR_shader_subgroup_basic

public void subgroupBarrier();
public void subgroupMemoryBarrier();
public void subgroupMemoryBarrierBuffer();
public void subgroupMemoryBarrierImage();
public void subgroupMemoryBarrierShared();
public bool subgroupElect();

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
[ForceInline] void subgroupBarrier()
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
        __subgroupBarrierSPIRV();
        return;
    case cpp:
        // TODO: cpp
        return;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
[ForceInline] void subgroupMemoryBarrier()
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
        __subgroupMemoryBarrierSPIRV();
        return;
    case cpp:
        // TODO: cpp
        return;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
[ForceInline] void subgroupMemoryBarrierBuffer()
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
        __subgroupMemoryBarrierBufferSPIRV();
        return;
    case cpp:
        // TODO: cpp
        return;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
[ForceInline] void subgroupMemoryBarrierImage()
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
        __subgroupMemoryBarrierImageSPIRV();
        return;
    case cpp:
        // TODO: cpp
        return;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
[ForceInline] void subgroupMemoryBarrierShared()
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
        __subgroupMemoryBarrierSharedSPIRV();
        return;
    case cpp:
        // TODO: cpp
        return;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_basic)
__spirv_version(1.3)
[ForceInline] bool subgroupElect()
{
    __target_switch
    {
    case cuda:
        return __isMinimumWarpActiveInBranchCUDA();
    case glsl:
    case spirv:
    case hlsl:
        return WaveIsFirstLane();
    case cpp:
        // TODO: cpp
        return false;
    }
}

// ~GL_KHR_shader_subgroup_basic

// GL_KHR_shader_subgroup_vote

public bool subgroupAll(bool value);
public bool subgroupAny(bool value);
public bool subgroupAllEqual(bool value);

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
[ForceInline] bool subgroupAll(bool value)
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

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
[ForceInline] bool subgroupAny(bool value)
{
    return WaveMaskAnyTrue(WaveGetActiveMask(), value);
    
}

__glsl_extension(GL_KHR_shader_subgroup_vote)
__spirv_version(1.3)
[ForceInline] bool subgroupAllEqual(bool value)
{
    return WaveMaskAllEqual(WaveGetActiveMask(), value);
}

// ~GL_KHR_shader_subgroup_vote

// GL_KHR_shader_subgroup_ballot

__generic<T : __BuiltinType> public T subgroupBroadcast(T value, uint id);
__generic<T : __BuiltinType> public T subgroupBroadcastFirst(T value);
public uvec4 subgroupBallot(bool value);
public bool subgroupInverseBallot(uvec4 value);
public bool subgroupBallotBitExtract(uvec4 value, uint index);
public uint subgroupBallotBitCount(uvec4 value);
public uint subgroupBallotInclusiveBitCount(uvec4 value);
public uint subgroupBallotExclusiveBitCount(uvec4 value);
public uint subgroupBallotFindLSB(uvec4 value);
public uint subgroupBallotFindMSB(uvec4 value);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] T subgroupBroadcast(T value, uint id)
{
    return WaveMaskBroadcastLaneAt(WaveGetActiveMask(), value, id);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] T subgroupBroadcastFirst(T value)
{
    return WaveMaskReadLaneFirst(WaveGetActiveMask(), value);
}

// WaveMaskBallot is not the same; it force trunc's
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] uvec4 subgroupBallot(bool value)
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
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] bool subgroupInverseBallot(uvec4 value)
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
        return __subgroupInverseBallotSPIRV(value);
    case cpp:
        // TODO: cpp
        return false;
    }
    return false;
}

// same logic as subgroupInverseBallot
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] bool subgroupBallotBitExtract(uvec4 value, uint index)
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
        return __subgroupBallotBitExtractSPIRV(value, index);
    case cpp:
        // TODO: cpp
        return false;
    }
    return false;
}


// the count is only supposed to use uvec4 values within bottom bits of subgroup launched, not a simple countbits
__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] uint subgroupBallotBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotBitCount($0)";
    case spirv:
        return __subgroupBallotBitCountSPIRV(value);
    case hlsl:
    case cuda:
    case cpp:
        //TODO: implement
        return 0;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] uint subgroupBallotInclusiveBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotInclusiveBitCount($0)";
    case spirv:
        return __subgroupBallotInclusiveBitCountSPIRV(value);
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] uint subgroupBallotExclusiveBitCount(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotExclusiveBitCount($0)";
    case spirv:
        return __subgroupBallotExclusiveBitCountSPIRV(value);
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] uint subgroupBallotFindLSB(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotFindLSB($0)";
    case spirv:
        return __subgroupBallotFindLSBSPIRV(value);
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

__glsl_extension(GL_KHR_shader_subgroup_ballot)
__spirv_version(1.3)
[ForceInline] uint subgroupBallotFindMSB(uvec4 value)
{
    __target_switch
    {
    case glsl:
        __intrinsic_asm "subgroupBallotFindMSB($0)";
    case spirv:
        return __subgroupBallotFindMSBSPIRV(value);
    case hlsl:
    case cuda:
    case cpp:
        // TODO: implementation
        return 0;
    }
}

// ~GL_KHR_shader_subgroup_ballot

// GL_KHR_shader_subgroup_arithmetic

__generic<T : __BuiltinArithmeticType> 
public T subgroupAdd(T value);

__generic<T : __BuiltinArithmeticType> 
public T subgroupMul(T value);

__generic<T : __BuiltinArithmeticType> 
public T subgroupMin(T value);

__generic<T : __BuiltinArithmeticType> 
public T subgroupMax(T value);

__generic<T : __BuiltinIntegerType> 
public T subgroupAnd(T value);

__generic<T : __BuiltinIntegerType> 
public T subgroupOr(T value);

__generic<T : __BuiltinIntegerType> 
public T subgroupXor(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupInclusiveAdd(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupInclusiveMul(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupInclusiveMin(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupInclusiveMax(T value);

__generic<T : __BuiltinIntegerType>
public T subgroupInclusiveAnd(T value);

__generic<T : __BuiltinIntegerType>
public T subgroupInclusiveOr(T value);

__generic<T : __BuiltinIntegerType>
public T subgroupInclusiveXor(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupExclusiveAdd(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupExclusiveMul(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupExclusiveMin(T value);

__generic<T : __BuiltinArithmeticType>
public T subgroupExclusiveMax(T value);

__generic<T : __BuiltinIntegerType>
public T subgroupExclusiveAnd(T value);

__generic<T : __BuiltinIntegerType>
public T subgroupExclusiveOr(T value);

__generic<T : __BuiltinIntegerType>
public T subgroupExclusiveXor(T value);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupAdd(T value)
{
    return WaveMaskSum(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupMul(T value)
{
    return WaveMaskProduct(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupMin(T value)
{

    return WaveMaskMin(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupMax(T value)
{

    return WaveMaskMax(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupAnd(T value)
{
    return WaveMaskBitAnd(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupOr(T value)
{
    return WaveMaskBitOr(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinIntegerType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupXor(T value)
{
    return WaveMaskBitXor(WaveGetActiveMask(), value);
}

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupInclusiveAdd(T value)
{
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveAdd($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFAdd $$T result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $value;
                OpGroupNonUniformIAdd $$uint %mulResult Subgroup InclusiveScan %uvalue;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformIAdd $$T result Subgroup InclusiveScan $value};
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupInclusiveMul(T value)
{
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupInclusiveMul($0)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpGroupNonUniformFMul $$T result Subgroup InclusiveScan $value};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $value;
                OpGroupNonUniformIMul $$uint %mulResult Subgroup InclusiveScan %uvalue;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupInclusiveMin(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupInclusiveMax(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupInclusiveAnd(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupInclusiveOr(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupInclusiveXor(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupExclusiveAdd(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupExclusiveMul(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupExclusiveMin(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupExclusiveMax(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupExclusiveAnd(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupExclusiveOr(T value)
{
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
__glsl_extension(GL_KHR_shader_subgroup_arithmetic)
__spirv_version(1.3)
[ForceInline] T subgroupExclusiveXor(T value)
{
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

// ~GL_KHR_shader_subgroup_arithmetic


// GL_KHR_shader_subgroup_shuffle

__generic<T : __BuiltinType>
public T subgroupShuffle(T value, uint index);

__generic<T : __BuiltinType>
public T subgroupShuffleXor(T value, uint mask);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
[ForceInline] T subgroupShuffle(T value, uint index)
{
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
__glsl_extension(GL_KHR_shader_subgroup_shuffle)
__spirv_version(1.3)
[ForceInline] T subgroupShuffleXor(T value, uint mask)
{
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

// ~GL_KHR_shader_subgroup_shuffle

// GL_KHR_shader_subgroup_shuffle_relative

__generic<T : __BuiltinType>
public T subgroupShuffleUp(T value, uint delta);

__generic<T : __BuiltinType>
public T subgroupShuffleDown(T value, uint delta);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_shuffle_relative)
__spirv_version(1.3)
[ForceInline] T subgroupShuffleUp(T value, uint delta)
{
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
__glsl_extension(GL_KHR_shader_subgroup_shuffle_relative)
__spirv_version(1.3)
[ForceInline] T subgroupShuffleDown(T value, uint delta)
{
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

// ~GL_KHR_shader_subgroup_shuffle_relative

// GL_KHR_shader_subgroup_clustered

__generic<T : __BuiltinArithmeticType>
public T subgroupClusteredAdd(T value, uint clusterSize);

__generic<T : __BuiltinArithmeticType>
public T subgroupClusteredMul(T value, uint clusterSize);

__generic<T : __BuiltinArithmeticType>
public T subgroupClusteredMin(T value, uint clusterSize);

__generic<T : __BuiltinArithmeticType>
public T subgroupClusteredMax(T value, uint clusterSize);

__generic<T : __BuiltinIntegerType>
public T subgroupClusteredAnd(T value, uint clusterSize);

__generic<T : __BuiltinIntegerType>
public T subgroupClusteredOr(T value, uint clusterSize);

__generic<T : __BuiltinIntegerType>
public T subgroupClusteredXor(T value, uint clusterSize);

__generic<T : __BuiltinArithmeticType>
__glsl_extension(GL_KHR_shader_subgroup_clustered)
__spirv_version(1.3)
    [ForceInline] T subgroupClusteredAdd(T value, uint clusterSize)
{
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredAdd($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; 
            OpGroupNonUniformFAdd $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $value;
                OpGroupNonUniformIAdd $$uint %mulResult Subgroup ClusteredReduce %uvalue $clusterSize;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
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
__glsl_extension(GL_KHR_shader_subgroup_clustered)
__spirv_version(1.3)
[ForceInline] T subgroupClusteredMul(T value, uint clusterSize)
{
    __target_switch
    {
    case glsl:
        __target_intrinsic "subgroupClusteredMul($0, $1)";
    case spirv:
        if (__isFloat<T>())
            return spirv_asm {OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered; OpGroupNonUniformFMul $$T result Subgroup ClusteredReduce $value $clusterSize};
        else if (__isSignedInt<T>())
        {
            return spirv_asm
            {
                OpCapability GroupNonUniformArithmetic; OpCapability GroupNonUniformClustered;
                // TODO: use the correct integer width
                OpBitcast $$uint %uvalue $value;
                OpGroupNonUniformIMul $$uint %mulResult Subgroup ClusteredReduce %uvalue $clusterSize;
                OpBitcast $$T result %mulResult
            };
        }
        else if (__isUnsignedInt<T>())
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
__glsl_extension(GL_KHR_shader_subgroup_clustered)
__spirv_version(1.3)
[ForceInline] T subgroupClusteredMin(T value, uint clusterSize)
{
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
__glsl_extension(GL_KHR_shader_subgroup_clustered)
__spirv_version(1.3)
[ForceInline] T subgroupClusteredMax(T value, uint clusterSize)
{
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
__glsl_extension(GL_KHR_shader_subgroup_clustered)
__spirv_version(1.3)
[ForceInline] T subgroupClusteredAnd(T value, uint clusterSize)
{
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
__glsl_extension(GL_KHR_shader_subgroup_clustered)
__spirv_version(1.3)
[ForceInline] T subgroupClusteredOr(T value, uint clusterSize)
{
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
__glsl_extension(GL_KHR_shader_subgroup_clustered)
__spirv_version(1.3)
[ForceInline] T subgroupClusteredXor(T value, uint clusterSize)
{
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

// ~GL_KHR_shader_subgroup_clustered

// GL_KHR_shader_subgroup_quad

__generic<T : __BuiltinType>
public T subgroupQuadBroadcast(T value, uint id);

__generic<T : __BuiltinType>
public T subgroupQuadSwapHorizontal(T value);

__generic<T : __BuiltinType>
public T subgroupQuadSwapVertical(T value);

__generic<T : __BuiltinType>
public T subgroupQuadSwapDiagonal(T value);

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
[ForceInline] T subgroupQuadBroadcast(T value, uint id)
{
    return QuadReadLaneAt(value, id);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
[ForceInline] T subgroupQuadSwapHorizontal(T value)
{
    return QuadReadAcrossX(value);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
[ForceInline] T subgroupQuadSwapVertical(T value)
{
    return QuadReadAcrossY(value);
}

__generic<T : __BuiltinType>
__glsl_extension(GL_KHR_shader_subgroup_quad)
__spirv_version(1.3)
[ForceInline] T subgroupQuadSwapDiagonal(T value)
{
    return QuadReadAcrossDiagonal(value);
}

// ~GL_KHR_shader_subgroup_quad

/// ~GL_KHR_shader_subgroup Functions

//// ~GL_KHR_shader_subgroup
