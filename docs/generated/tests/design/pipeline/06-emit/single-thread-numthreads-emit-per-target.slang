//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#backends
//META: doc_section_digest=d646ec1bfe54ca64d7afa27f7a26e793849f961a190aeb5f3c004156a5cb2217
//META: purpose=Boundary: the smallest documented numthreads value (1, 1, 1) is emitted as `numthreads(1,1,1)` / `local_size_x=1` / `LocalSize 1 1 1` / `@workgroup_size(1,1,1)` — no backend collapses or omits the single-thread workgroup attribute.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe at the lower edge of the numthreads/local-size
// space: (1, 1, 1) is the smallest valid workgroup. Some backends
// could conceivably treat the trivial workgroup specially (omit the
// attribute, or collapse to a scalar). This test pins the explicit
// triplet on every target.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = int(tid.x);
}

// HLSL: numthreads(1, 1, 1)
// GLSL: local_size_x = 1, local_size_y = 1, local_size_z = 1
// SPIRV: LocalSize 1 1 1
// WGSL: @workgroup_size(1, 1, 1)
