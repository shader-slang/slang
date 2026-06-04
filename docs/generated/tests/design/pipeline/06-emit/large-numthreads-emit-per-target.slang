//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=Boundary: a large numthreads value (1024, 1, 1) round-trips through emit as the target's local-workgroup-size marker — HLSL `numthreads(1024,1,1)`, GLSL `local_size_x = 1024`, SPIR-V `LocalSize 1024 1 1`, WGSL `@workgroup_size(1024,1,1)`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the upper edge of the typical compute-workgroup
// range: D3D/Vulkan/Metal/WGSL all allow workgroup sizes up to 1024.
// The emit must not truncate, reorder, or rewrite the literal.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1024, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: numthreads(1024, 1, 1)
// GLSL: local_size_x = 1024
// SPIRV: LocalSize 1024 1 1
// METAL: kernel
// WGSL: @workgroup_size(1024, 1, 1)
