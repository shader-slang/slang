//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-05T00:00:00+00:00
//META: source_commit=5634a0ea1b
//META: purpose=Characterizes the WGSL system-value table for a fragment entry point, mapping SV_Position/SV_IsFrontFace/SV_SampleIndex to @builtin inputs and SV_Depth/SV_Coverage to @builtin members of the returned struct while SV_Target becomes a @location.
//META: intent=characterization
//META: covers=source/slang/slang-ir-legalize-varying-params.cpp
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Pins `LegalizeWGSLEntryPointContext::getSystemValueInfo`
// (slang-ir-legalize-varying-params.cpp:4790) for the fragment stage. WGSL
// spells varying parameters as `@builtin(name)` / `@location(n)` attributes,
// and the interesting rows here are the ones whose WGSL name is not the HLSL
// one: `SV_Depth` is `frag_depth`, `SV_Coverage` is `sample_mask` (a scalar
// `u32`, unlike GLSL where it is an array), and `SV_Target` is not a builtin
// at all but an ordinary `@location(0)`. CHECK asserts the full signature and
// the output struct members so the input mappings and the output mappings are
// pinned together.
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry fragMain -stage fragment

struct FIn
{
    float4 pos : SV_Position;
    bool front : SV_IsFrontFace;
    uint sampleId : SV_SampleIndex;
}
struct FOut
{
    float4 color : SV_Target;
    float depth : SV_Depth;
    uint cover : SV_Coverage;
}

[shader("fragment")]
FOut fragMain(FIn i)
{
    FOut o;
    o.color = i.pos + float(i.front) + float(i.sampleId);
    o.depth = 0.5;
    o.cover = 1;
    return o;
}

// SV_Target is a plain location; the two other outputs are builtins.
//WGSL-DAG: @location(0) color_0 : vec4<f32>,
//WGSL-DAG: @builtin(frag_depth) depth_0 : f32,
//WGSL-DAG: @builtin(sample_mask) cover_0 : u32,
// The input struct is flattened into one attributed parameter per field.
//WGSL: fn fragMain(@builtin(position) pos_0 : vec4<f32>, @builtin(front_facing) front_0 : bool, @builtin(sample_index) sampleId_0 : u32) -> FOut_0
