//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#emit
//META: doc_section_digest=f5ca0be190b77cca517eee93b8fc03af868bd9b8d69abe8e20fadc934d5e4ab2
//META: purpose=A compute entry-point declaration survives every stage and reaches emit as the selected back-end's own compute-kernel marker.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the entry-point designation is carried through the whole
// pipeline as data (not re-derived at emit from the function name): the
// front end records the compute stage, the IR carries it, and the emitter
// chosen for each target spells it in that target's own way. Each CHECK
// pins the marker only a compute entry point can produce on that back-end
// -- `numthreads`, a GLSL `local_size` layout, a `GLCompute` execution
// model, Metal's `kernel` qualifier, WGSL's `@compute`, CUDA's
// `__global__`, and the exported C++ dispatch wrapper.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<uint> gOut;

[shader("compute")]
[numthreads(8, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOut[0] = tid.x;
}

// HLSL: numthreads

// GLSL: local_size_x

// SPIRV: OpEntryPoint GLCompute

// METAL: {{\[\[}}kernel{{\]\]}}

// WGSL: @compute

// CUDA: __global__

// CPP: computeMain_Thread
