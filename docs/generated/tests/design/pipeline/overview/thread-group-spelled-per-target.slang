//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-08T21:40:00Z
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/overview.md#emit
//META: doc_section_digest=f5ca0be190b77cca517eee93b8fc03af868bd9b8d69abe8e20fadc934d5e4ab2
//META: purpose=Each C-like source emitter emits for any compute entry point and spells the thread-group declaration in its own idiom: an HLSL attribute, a GLSL layout qualifier, a SPIR-V execution mode, a WGSL attribute, with the host-dispatched Metal and CUDA kernels carrying no 16, 1, 1.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the emit-dispatch claim that every text back-end emits for an
// ordinary entry point, each in its own idiom. A single `numthreads(16, 1, 1)`
// becomes an HLSL attribute, a GLSL `layout` qualifier, a SPIR-V
// `OpExecutionMode`, a WGSL `@workgroup_size`, and a C++ comment on the
// exported wrapper, while Metal and CUDA, whose runtimes supply the group
// size at dispatch, emit no `16, 1, 1`. (Per the ir-passes section this is
// emitter spelling, not pass-list branching; see pipeline-is-target-sensitive
// for the latter.)

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry computeMain -stage compute

RWStructuredBuffer<int> gOut;

[shader("compute")]
[numthreads(16, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    gOut[0] = int(tid.x);
}

// HLSL: numthreads(16, 1, 1)

// GLSL: local_size_x = 16, local_size_y = 1, local_size_z = 1

// SPIRV: OpExecutionMode {{.*}} LocalSize 16 1 1

// WGSL: @workgroup_size(16, 1, 1)

// C++ keeps the group size only as a comment above the exported wrapper.
// CPP: numthreads(16, 1, 1)

// Metal and CUDA take the thread-group size from the host dispatch call, so
// the pass list drops it before emit on those targets.
// METAL: {{\[\[}}kernel{{\]\]}} void computeMain(
// METAL-NOT: 16, 1, 1

// CUDA: __global__ void computeMain(
// CUDA-NOT: 16, 1, 1
