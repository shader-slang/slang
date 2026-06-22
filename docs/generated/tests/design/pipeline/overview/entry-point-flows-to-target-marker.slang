//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=74db89b9f77cdced9c4d0c47f377b38fffb9180b
//META: doc_ref=docs/generated/design/pipeline/overview.md#emit
//META: doc_section_digest=d7c5155aced5a19f1303c1d313c7cf3b21da58b38cefd801d92e52039ee02528
//META: purpose=The [shader("compute")] entry-point attribute survives every pipeline stage and emerges as the target's compute-kernel marker.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an entry-point attribute set in source survives the
// full pipeline and reaches the target backend in the right form.
// Each backend has a distinct way to spell "this is a compute
// kernel": HLSL keeps numthreads, GLSL emits the GLSL layout shape,
// SPIR-V uses OpEntryPoint with GLCompute execution model, Metal
// tags with kernel, WGSL uses @compute, and CUDA tags with
// __global__. Showing that all of these emerge from a single shared
// source proves the entry-point info is carried by the IR from
// front-end down to emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=CUDA):-target cuda -entry main -stage compute

[shader("compute")]
[numthreads(8, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    // HLSL: numthreads(8, 1, 1)
    // GLSL: local_size_x = 8
    // SPIRV: OpEntryPoint GLCompute
    // METAL: kernel
    // WGSL: @workgroup_size(8
    // CUDA: __global__
    return;
}
