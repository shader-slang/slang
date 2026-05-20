//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/cross-cutting/targets.md#vocabulary
//META: doc_section_digest=e6ee8d6a0ad9b6be0dfa1e2bef7fa142d6866067cb581476b3dadc1f13bb47e4
//META: purpose=A stage atom (`compute`) populates the `stage` keyhole; a target atom (`hlsl`, `glsl`, `spirv`, ...) populates the `target` keyhole. Because they populate different keyholes a `compute` entry point is compatible with every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's Vocabulary section establishes that atoms populate
// keyholes and that two atoms populating different keyholes are
// compatible (the `vertex + glsl` example). `compute` (stage) and
// `hlsl`/`glsl`/`spirv`/`metal`/`wgsl`/`cuda`/`cpp` (target) are
// in distinct keyholes, so a single `[shader("compute")]` entry
// point validates against every text-emit target. This test is a
// positive proof: the same shader compiles under each backend.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(4, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 42;
}

// HLSL: 42
// GLSL: 42
// SPIRV: OpConstant {{.*}} 42
// METAL: 42
// WGSL: 42
// CUDA: 42
// CPP: 42
