//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage compute -DNO_PARAMETER_BLOCK

// A combined texture-sampler nested inside a constant buffer, a parameter block, a struct or the
// uniform parameters of an entry point is split into a texture and a sampler, and each of them
// keeps the binding that reflection reports for the combined value. The texture and sampler
// before the nested ones make every expected index non-zero.
//
// Metal keeps a parameter block whole as an argument buffer instead of splitting it, so the Metal
// run leaves the parameter blocks out.

Texture2D<float4> pre;
SamplerState preS;

cbuffer C
{
    Sampler2D<float4> a;
    Sampler2D<float4> b;
    float4 c;
}

struct S
{
    Sampler2D<float4> t;
    float4 k;
}
ConstantBuffer<S> cb;
uniform S g;

struct In
{
    Sampler2D<float4> t;
}
struct Outer
{
    In i;
    In arr[2];
    float x;
}
ConstantBuffer<Outer> nest;

#ifndef NO_PARAMETER_BLOCK
ParameterBlock<S> pbs;
ParameterBlock<Sampler2D<float4>> pbt;
#endif

RWStructuredBuffer<float4> o;

// HLSL-DAG: Texture2D<float4 > C_b_texture_0 : register(t2);
// HLSL-DAG: SamplerState C_b_sampler_0 : register(s2);
// HLSL-DAG: Texture2D<float4 > cb_t_texture_0 : register(t3);
// HLSL-DAG: SamplerState cb_t_sampler_0 : register(s3);
// HLSL-DAG: Texture2D<float4 > globalParams_g_t_texture_0 : register(t4);
// HLSL-DAG: SamplerState globalParams_g_t_sampler_0 : register(s4);
// HLSL-DAG: Texture2D<float4 > nest_i_t_texture_0 : register(t5);
// HLSL-DAG: SamplerState nest_i_t_sampler_0 : register(s5);
// HLSL-DAG: Texture2D<float4 > nest_arr_t_texture_0[int(2)] : register(t6);
// HLSL-DAG: SamplerState nest_arr_t_sampler_0[int(2)] : register(s6);
// HLSL-DAG: Texture2D<float4 > pbs_t_texture_0 : register(t8);
// HLSL-DAG: SamplerState pbs_t_sampler_0 : register(s8);
// HLSL-DAG: Texture2D<float4 > pbt_texture_0 : register(t9);
// HLSL-DAG: SamplerState pbt_sampler_0 : register(s9);
// HLSL-DAG: Texture2D<float4 > entryPointParams_ept_texture_0 : register(t10);
// HLSL-DAG: SamplerState entryPointParams_ept_sampler_0 : register(s10);
// HLSL-DAG: Texture2D<float4 > entryPointParams_eps_t_texture_0 : register(t11);
// HLSL-DAG: SamplerState entryPointParams_eps_t_sampler_0 : register(s11);

// WGSL-DAG: @binding(6) @group(0) var C_b_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(7) @group(0) var C_b_sampler_0 : sampler;
// WGSL-DAG: @binding(9) @group(0) var cb_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(10) @group(0) var cb_t_sampler_0 : sampler;
// WGSL-DAG: @binding(11) @group(0) var globalParams_g_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(12) @group(0) var globalParams_g_t_sampler_0 : sampler;
// WGSL-DAG: @binding(14) @group(0) var nest_i_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(15) @group(0) var nest_i_t_sampler_0 : sampler;
// WGSL-DAG: @binding(16) @group(0) var nest_arr_t_texture_0 : array<texture_2d<f32>, i32(2)>;
// WGSL-DAG: @binding(17) @group(0) var nest_arr_t_sampler_0 : array<sampler, i32(2)>;
// WGSL-DAG: @binding(1) @group(1) var pbs_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(2) @group(1) var pbs_t_sampler_0 : sampler;
// WGSL-DAG: @binding(0) @group(2) var pbt_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(1) @group(2) var pbt_sampler_0 : sampler;
// WGSL-DAG: @binding(20) @group(0) var entryPointParams_ept_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(21) @group(0) var entryPointParams_ept_sampler_0 : sampler;
// WGSL-DAG: @binding(22) @group(0) var entryPointParams_eps_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(23) @group(0) var entryPointParams_eps_t_sampler_0 : sampler;

// METAL-DAG: C_b_texture_{{[0-9]+}} {{\[\[}}texture(2){{\]\]}}
// METAL-DAG: C_b_sampler_{{[0-9]+}} {{\[\[}}sampler(2){{\]\]}}
// METAL-DAG: cb_t_texture_{{[0-9]+}} {{\[\[}}texture(3){{\]\]}}
// METAL-DAG: cb_t_sampler_{{[0-9]+}} {{\[\[}}sampler(3){{\]\]}}
// METAL-DAG: globalParams_g_t_texture_{{[0-9]+}} {{\[\[}}texture(4){{\]\]}}
// METAL-DAG: globalParams_g_t_sampler_{{[0-9]+}} {{\[\[}}sampler(4){{\]\]}}
// METAL-DAG: nest_i_t_texture_{{[0-9]+}} {{\[\[}}texture(5){{\]\]}}
// METAL-DAG: nest_i_t_sampler_{{[0-9]+}} {{\[\[}}sampler(5){{\]\]}}
// METAL-DAG: nest_arr_t_texture_{{[0-9]+}} {{\[\[}}texture(6){{\]\]}}
// METAL-DAG: nest_arr_t_sampler_{{[0-9]+}} {{\[\[}}sampler(6){{\]\]}}
// METAL-DAG: entryPointParams_ept_texture_{{[0-9]+}} {{\[\[}}texture(8){{\]\]}}
// METAL-DAG: entryPointParams_ept_sampler_{{[0-9]+}} {{\[\[}}sampler(8){{\]\]}}
// METAL-DAG: entryPointParams_eps_t_texture_{{[0-9]+}} {{\[\[}}texture(9){{\]\]}}
// METAL-DAG: entryPointParams_eps_t_sampler_{{[0-9]+}} {{\[\[}}sampler(9){{\]\]}}

[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID, uniform Sampler2D<float4> ept, uniform S eps)
{
    float2 uv = float2(id.xy);
    float4 r = pre.SampleLevel(preS, uv, 0);
    r += b.SampleLevel(uv, 0) + b.Load(int3(id.xy, 0));
    r += cb.t.SampleLevel(uv, 0) + cb.t.Load(int3(0));
    r += g.t.SampleLevel(uv, 0) + g.t.Load(int3(0));
    r += nest.i.t.SampleLevel(uv, 0) + nest.arr[id.x].t.SampleLevel(uv, 0);
    r += nest.arr[1].t.Load(int3(0));
#ifndef NO_PARAMETER_BLOCK
    r += pbs.t.SampleLevel(uv, 0) + pbt.SampleLevel(uv, 0) + pbt.Load(int3(0));
#endif
    r += ept.SampleLevel(uv, 0) + eps.t.SampleLevel(uv, 0) + eps.t.Load(int3(0));
    o[0] = r + c + cb.k + nest.x;
}
