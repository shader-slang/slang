//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#definition-forms
//META: doc_section_digest=2c0608f6d4897cdbfc247f90cd3fba9250321d1c82341829952363fd26bccf89
//META: purpose=A stage atom (`compute`) populates the `stage` keyhole; a target atom (`hlsl`, `glsl`, `spirv`, ...) populates the `target` keyhole. Because they populate different keyholes a `compute` entry point is compatible with every text-emit target.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an unpopulated keyhole stays compatible with any key
// atom of that keyhole: a `compute` entry point populates only the
// `stage` keyhole, leaving `target` free, so the same shader is
// admissible under every target atom. Each per-target CHECK asserts
// the literal 42 reaches that backend's emitted text, which happens
// only if the stage/target conjunction was accepted for that target.

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
