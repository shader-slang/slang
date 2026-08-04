//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9f3194ddc2631c231ae0863cc8c3961c0a5eef6db41560bb17b1114e6c0cf6aa
//META: purpose=Boundary: an explicit `row_major float4x4` in a cbuffer changes the per-target emit marker — GLSL drops the `_ColMajor` wrapper-struct suffix; SPIR-V emits `ColMajor` member decoration (because SPIR-V matrix layouts are transposed relative to source); WGSL drops the `_ColMajor` infix.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe pair to `matrix-default-layout-emit-per-target`:
// flipping the matrix-layout qualifier to `row_major` toggles the
// per-target marker. The default-layout test pins the `_ColMajor`
// (Metal/WGSL) and `RowMajor` (SPIR-V) shapes; this one pins the
// row_major equivalents.

//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

cbuffer C
{
    row_major float4x4 m;
}
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = m[0];
}

// GLSL: _MatrixStorage_float4x4natural
// SPIRV-DAG: ColMajor
// WGSL: _MatrixStorage_float4x4std140
