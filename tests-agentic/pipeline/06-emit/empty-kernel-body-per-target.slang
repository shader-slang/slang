//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=Boundary: a compute kernel with an empty body still emits all required per-target entry-point markers — every backend round-trips the minimum shape (numthreads/local_size_x/@workgroup_size/OpExecutionMode LocalSize/[[kernel]]).
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Empty-body boundary: a [shader("compute")][numthreads(1,1,1)] kernel
// with no statements still has to emit the target-specific compute
// markers. If any backend lost its entry-point shape when the body is
// trivially empty, the corresponding sub-test fails.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute
//TEST:SIMPLE(filecheck=CPP):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
}

// HLSL: numthreads(1, 1, 1)
// GLSL: layout(local_size_x = 1, local_size_y = 1, local_size_z = 1)
// SPIRV: OpExecutionMode %main{{.*}} LocalSize 1 1 1
// METAL: kernel
// WGSL: @workgroup_size(1, 1, 1)
// CUDA: __global__
// CPP: SLANG_PRELUDE_EXPORT
