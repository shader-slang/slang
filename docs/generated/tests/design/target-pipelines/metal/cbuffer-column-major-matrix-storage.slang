//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=2a4fe67f3530d551446e5a52ff530daaceb47c541972fbd77fc4832e5e0aa6a6
//META: purpose=Boundary: a `column_major float4x4` inside a cbuffer triggers the `_MatrixStorage_float4x4_ColMajornatural_<N>` wrapper inside `SLANG_ParameterGroup_M_natural_<M>`.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: matrix layout = column-major (default). The
// emitter inserts the `_MatrixStorage_..._ColMajornatural` wrapper
// because Metal cannot reference a column-major `float4x4`
// element directly. This is the orientation pair to the row-major
// boundary test.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

cbuffer M
{
    column_major float4x4 mat;
}
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(mat, float4(float(tid.x), 0, 0, 1));
}

// CHECK: _MatrixStorage_float4x4_ColMajornatural
// CHECK: SLANG_ParameterGroup_M_natural
