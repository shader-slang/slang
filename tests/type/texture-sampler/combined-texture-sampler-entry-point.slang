//TEST:SIMPLE(filecheck=METAL): -target metal -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -entry fsMain -stage fragment
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry fsMain -stage fragment
//TEST:REFLECTION(filecheck=REFLECT): -target metal -entry fsMain -stage fragment

// Combined texture-samplers declared as entry-point `uniform` parameters are
// gathered into an `EntryPointParams` struct before they are split into a
// texture and a sampler. The split resources must keep the bindings that
// reflection reports for them (gh-13272).

// REFLECT: "name": "unused"
// REFLECT-NOT: }
// REFLECT: {"kind": "shaderResource", "index": 0}
// REFLECT-NOT: }
// REFLECT: {"kind": "samplerState", "index": 0}
// REFLECT: "name": "used"
// REFLECT-NOT: }
// REFLECT: {"kind": "shaderResource", "index": 1}
// REFLECT-NOT: }
// REFLECT: {"kind": "samplerState", "index": 1}

// METAL-DAG: entryPointParams_used_texture_{{[0-9]+}} {{\[\[}}texture(1)]]
// METAL-DAG: entryPointParams_used_sampler_{{[0-9]+}} {{\[\[}}sampler(1)]]

// HLSL-DAG: Texture2D<float4 > entryPointParams_used_texture_{{[0-9]+}} : register(t1);
// HLSL-DAG: SamplerState entryPointParams_used_sampler_{{[0-9]+}} : register(s1);

// WGSL-DAG: @binding(2) @group(0) var entryPointParams_used_texture_{{[0-9]+}} : texture_2d<f32>;
// WGSL-DAG: @binding(3) @group(0) var entryPointParams_used_sampler_{{[0-9]+}} : sampler;

[shader("fragment")]
float4 fsMain(float2 uv : TEXCOORD0, uniform Sampler2D unused, uniform Sampler2D used) : SV_Target
{
    return used.Sample(uv);
}
