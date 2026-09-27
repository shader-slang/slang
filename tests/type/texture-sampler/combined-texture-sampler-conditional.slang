//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry fsMain -stage fragment

// `Conditional<Sampler2D, ...>` shader parameters, both at global scope
// (gh-10117) and as struct fields (gh-13272), keep their bindings once the
// combined texture-sampler is split into a texture and a sampler.

extern const static bool testBool = true;
Conditional<Sampler2D, testBool> testSampler;

struct S
{
    Conditional<Sampler2D, true> unused;
    Conditional<Sampler2D, true> used;
};
S g;

// METAL-DAG: testSampler_texture_{{[0-9]+}} {{\[\[}}texture(0)]]
// METAL-DAG: testSampler_sampler_{{[0-9]+}} {{\[\[}}sampler(0)]]
// METAL-DAG: g_used_texture_{{[0-9]+}} {{\[\[}}texture(2)]]
// METAL-DAG: g_used_sampler_{{[0-9]+}} {{\[\[}}sampler(2)]]

// HLSL-DAG: Texture2D<float4 > testSampler_texture_{{[0-9]+}} : register(t0);
// HLSL-DAG: SamplerState testSampler_sampler_{{[0-9]+}} : register(s0);
// HLSL-DAG: Texture2D<float4 > g_used_texture_{{[0-9]+}} : register(t2);
// HLSL-DAG: SamplerState g_used_sampler_{{[0-9]+}} : register(s2);

// WGSL-DAG: @binding(0) @group(0) var testSampler_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(1) @group(0) var testSampler_sampler_{{[0-9]+}} : sampler;
// WGSL-DAG: @binding(4) @group(0) var g_used_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(5) @group(0) var g_used_sampler_{{[0-9]+}} : sampler;

[shader("fragment")]
float4 fsMain(float2 uv : TEXCOORD0) : SV_Target
{
    float4 result = g.used.get().value.Sample(uv);
    if (testSampler.get().hasValue)
        result += testSampler.get().value.Sample(uv);
    return result;
}
