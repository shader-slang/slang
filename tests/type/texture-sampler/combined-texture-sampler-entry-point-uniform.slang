//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry fsMain -stage fragment

// An ordinary `uniform` entry-point parameter makes the gathered
// `EntryPointParams` a constant buffer. The combined texture-samplers that sit
// next to it must still keep their bindings once they are split (gh-13272).

// METAL-DAG: {{\[\[}}buffer(0)]]
// METAL-DAG: entryPointParams_used_texture_{{[0-9]+}} {{\[\[}}texture(1)]]
// METAL-DAG: entryPointParams_used_sampler_{{[0-9]+}} {{\[\[}}sampler(1)]]

// HLSL-DAG: Texture2D<float4 > entryPointParams_used_texture_{{[0-9]+}} : register(t1);
// HLSL-DAG: SamplerState entryPointParams_used_sampler_{{[0-9]+}} : register(s1);

// WGSL-DAG: @binding(0) @group(0) var<uniform> entryPointParams_{{[0-9]+}}
// WGSL-DAG: @binding(3) @group(0) var entryPointParams_used_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(4) @group(0) var entryPointParams_used_sampler_{{[0-9]+}} : sampler;

[shader("fragment")]
float4 fsMain(float2 uv : TEXCOORD0, uniform float4 scale, uniform Sampler2D unused, uniform Sampler2D used) : SV_Target
{
    return used.Sample(uv) * scale;
}
