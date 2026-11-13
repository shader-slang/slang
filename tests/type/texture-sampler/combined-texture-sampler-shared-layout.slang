//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL): -target metal -entry main -stage compute -DNO_PARAMETER_BLOCK

// IR layouts are deduplicated, so the layout of the global `x` below (t0/s0) is the same
// instruction as the layout of each first field (relative t0/s0) of the constant buffers and
// structs after it. Lowering `x` must leave that shared layout intact for the nested fields.

struct S
{
    Sampler2D<float4> t;
    float4 k;
}

Sampler2D<float4> x;
cbuffer C
{
    Sampler2D<float4> a;
    float4 c;
}
ConstantBuffer<S> cb;
uniform S g;
#ifndef NO_PARAMETER_BLOCK
ParameterBlock<S> pb;
#endif
RWStructuredBuffer<float4> o;

// HLSL-DAG: Texture2D<float4 > C_a_texture_0 : register(t1);
// HLSL-DAG: SamplerState C_a_sampler_0 : register(s1);
// HLSL-DAG: Texture2D<float4 > cb_t_texture_0 : register(t2);
// HLSL-DAG: SamplerState cb_t_sampler_0 : register(s2);
// HLSL-DAG: Texture2D<float4 > globalParams_g_t_texture_0 : register(t3);
// HLSL-DAG: SamplerState globalParams_g_t_sampler_0 : register(s3);
// HLSL-DAG: Texture2D<float4 > pb_t_texture_0 : register(t4);
// HLSL-DAG: SamplerState pb_t_sampler_0 : register(s4);

// WGSL-DAG: @binding(4) @group(0) var C_a_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(5) @group(0) var C_a_sampler_0 : sampler;
// WGSL-DAG: @binding(7) @group(0) var cb_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(8) @group(0) var cb_t_sampler_0 : sampler;
// WGSL-DAG: @binding(9) @group(0) var globalParams_g_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(10) @group(0) var globalParams_g_t_sampler_0 : sampler;
// WGSL-DAG: @binding(1) @group(1) var pb_t_texture_0 : texture_2d<f32>;
// WGSL-DAG: @binding(2) @group(1) var pb_t_sampler_0 : sampler;

// METAL-DAG: C_a_texture_{{[0-9]+}} {{\[\[}}texture(1){{\]\]}}
// METAL-DAG: C_a_sampler_{{[0-9]+}} {{\[\[}}sampler(1){{\]\]}}
// METAL-DAG: cb_t_texture_{{[0-9]+}} {{\[\[}}texture(2){{\]\]}}
// METAL-DAG: cb_t_sampler_{{[0-9]+}} {{\[\[}}sampler(2){{\]\]}}
// METAL-DAG: globalParams_g_t_texture_{{[0-9]+}} {{\[\[}}texture(3){{\]\]}}
// METAL-DAG: globalParams_g_t_sampler_{{[0-9]+}} {{\[\[}}sampler(3){{\]\]}}

[numthreads(1, 1, 1)]
void main(uint3 id: SV_DispatchThreadID)
{
    float2 uv = float2(id.xy);
    float4 r = x.SampleLevel(uv, 0) + a.SampleLevel(uv, 0) + a.Load(int3(0)) + c;
    r += cb.t.SampleLevel(uv, 0) + cb.k + g.t.SampleLevel(uv, 0) + g.k;
#ifndef NO_PARAMETER_BLOCK
    r += pb.t.SampleLevel(uv, 0) + pb.k;
#endif
    o[0] = r;
}
