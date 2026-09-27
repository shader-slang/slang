//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry fsMain -stage fragment

// Combined texture-samplers nested in a struct, an array field or a constant
// buffer keep the bindings that the enclosing parameter's layout assigns to
// them once they are split into a texture and a sampler (gh-13272).

struct S
{
    Sampler2D unused;
    Sampler2D used;
    Sampler2D usedArray[2];
};

struct T
{
    float4 scale;
    Sampler2D unused;
    Sampler2D used;
};

S g;
ConstantBuffer<T> cb;

// METAL-DAG: g_used_texture_{{[0-9]+}} {{\[\[}}texture(1)]]
// METAL-DAG: g_used_sampler_{{[0-9]+}} {{\[\[}}sampler(1)]]
// METAL-DAG: cb_used_texture_{{[0-9]+}} {{\[\[}}texture(5)]]
// METAL-DAG: cb_used_sampler_{{[0-9]+}} {{\[\[}}sampler(5)]]

// HLSL-DAG: Texture2D<float4 > g_used_texture_{{[0-9]+}} : register(t1);
// HLSL-DAG: SamplerState g_used_sampler_{{[0-9]+}} : register(s1);
// HLSL-DAG: Texture2D<float4 > g_usedArray_texture_{{[0-9]+}}[int(2)] : register(t2);
// HLSL-DAG: SamplerState g_usedArray_sampler_{{[0-9]+}}[int(2)] : register(s2);
// HLSL-DAG: Texture2D<float4 > cb_used_texture_{{[0-9]+}} : register(t5);
// HLSL-DAG: SamplerState cb_used_sampler_{{[0-9]+}} : register(s5);

// WGSL-DAG: @binding(2) @group(0) var g_used_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(3) @group(0) var g_used_sampler_{{[0-9]+}} : sampler;
// WGSL-DAG: @binding(4) @group(0) var g_usedArray_texture_{{[0-9]+}} : array<texture_2d<f32>, i32(2)>;
// WGSL-DAG: @binding(5) @group(0) var g_usedArray_sampler_{{[0-9]+}} : array<sampler, i32(2)>;
// WGSL-DAG: @binding(6) @group(0) var<uniform> cb_{{[0-9]+}}
// WGSL-DAG: @binding(9) @group(0) var cb_used_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(10) @group(0) var cb_used_sampler_{{[0-9]+}} : sampler;

[shader("fragment")]
float4 fsMain(float2 uv : TEXCOORD0) : SV_Target
{
    return g.used.Sample(uv) + g.usedArray[1].Sample(uv) + cb.used.Sample(uv) * cb.scale;
}
