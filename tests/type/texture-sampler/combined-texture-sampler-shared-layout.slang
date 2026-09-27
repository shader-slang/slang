//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry fsMain -stage fragment

// Layouts are deduplicated, so the layouts of `top0` and `top1` (offsets 0
// and 1) are the same instructions as the field layouts of `g.a` and `g.b`
// (offsets 0 and 1 within `S`). Lowering the top-level parameters must leave
// those shared layouts intact, or `g.a` and `g.b` end up with the same
// bindings (gh-13272).

struct S
{
    Sampler2D a;
    Sampler2D b;
};

Sampler2D top0;
Sampler2D top1;
S g;

// METAL-DAG: top0_texture_{{[0-9]+}} {{\[\[}}texture(0)]]
// METAL-DAG: top1_texture_{{[0-9]+}} {{\[\[}}texture(1)]]
// METAL-DAG: g_a_texture_{{[0-9]+}} {{\[\[}}texture(2)]]
// METAL-DAG: g_a_sampler_{{[0-9]+}} {{\[\[}}sampler(2)]]
// METAL-DAG: g_b_texture_{{[0-9]+}} {{\[\[}}texture(3)]]
// METAL-DAG: g_b_sampler_{{[0-9]+}} {{\[\[}}sampler(3)]]

// HLSL-DAG: Texture2D<float4 > g_a_texture_{{[0-9]+}} : register(t2);
// HLSL-DAG: SamplerState g_a_sampler_{{[0-9]+}} : register(s2);
// HLSL-DAG: Texture2D<float4 > g_b_texture_{{[0-9]+}} : register(t3);
// HLSL-DAG: SamplerState g_b_sampler_{{[0-9]+}} : register(s3);

// WGSL-DAG: @binding(4) @group(0) var g_a_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(5) @group(0) var g_a_sampler_{{[0-9]+}} : sampler;
// WGSL-DAG: @binding(6) @group(0) var g_b_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(7) @group(0) var g_b_sampler_{{[0-9]+}} : sampler;

[shader("fragment")]
float4 fsMain(float2 uv : TEXCOORD0) : SV_Target
{
    return top0.Sample(uv) + top1.Sample(uv) + g.a.Sample(uv) + g.b.Sample(uv);
}
