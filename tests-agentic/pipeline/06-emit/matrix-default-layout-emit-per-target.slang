//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/pipeline/06-emit.md#shared-c-like-base
//META: doc_section_digest=9bd773e205e5aa520ee4875c2a6aebc4a9d9aabba8cfac9d3b66489e9488c043
//META: purpose=Boundary: a default-layout float4x4 in a cbuffer is emitted with a target-specific matrix marker — HLSL `#pragma pack_matrix(column_major)`, GLSL `layout(row_major)`, SPIR-V `RowMajor` member decoration, Metal/WGSL `_MatrixStorage_float4x4_ColMajor...`.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: a `float4x4` in a `cbuffer` exercises the matrix
// layout legalization path. The Slang default is column-major in
// memory, but each backend spells the layout invariant in its own
// way (a #pragma, a layout() qualifier, an OpDecorate, or a wrapper
// struct name suffix).

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage compute
//TEST:SIMPLE(filecheck=WGSL):-target wgsl -entry main -stage compute

cbuffer C
{
    float4x4 m;
}
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = m[0];
}

// HLSL: pack_matrix(column_major)
// GLSL: layout(row_major)
// SPIRV-DAG: RowMajor
// METAL: _MatrixStorage_float4x4_ColMajor
// WGSL: _MatrixStorage_float4x4_ColMajor
