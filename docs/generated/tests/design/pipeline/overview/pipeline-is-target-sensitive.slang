//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/overview.md#ir-passes
//META: doc_section_digest=34a1a93797103f8be5a87169d293f14c96530b6a0d321516d44af0828be23f85
//META: purpose=The IR transformation sequence between lowering and emit is target-sensitive, so one thread-group declaration is materialized in a different form on each target and dropped entirely on the targets that take it at dispatch time.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the pass sequence branches on the target rather than
// producing one neutral shape that emit merely re-spells. A single
// `numthreads(16, 1, 1)` becomes an HLSL attribute, a GLSL `layout`
// qualifier, a SPIR-V `OpExecutionMode`, a WGSL `@workgroup_size`, and a
// C++ comment on the exported wrapper -- while Metal and CUDA, whose
// runtimes supply the group size at dispatch, emit no `16, 1, 1` at all.
// The CHECKs assert the five materialized forms and the two omissions, a
// divergence only a target-sensitive pass list can produce.

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
