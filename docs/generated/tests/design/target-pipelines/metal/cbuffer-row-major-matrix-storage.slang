//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00Z
//META: source_commit=781ed4aac159d764c5350d082058d68d4f91b785
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=2a4fe67f3530d551446e5a52ff530daaceb47c541972fbd77fc4832e5e0aa6a6
//META: purpose=Boundary: a `row_major float4x4` inside a cbuffer is passed through `wrapCBufferElementsForMetal` and emits as a bare `matrix<float,int(4),int(4)>` field inside the `SLANG_ParameterGroup_M_0` wrapper (no MatrixStorage wrapper layer in this orientation path).
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: matrix layout = row-major. With `row_major` the
// cbuffer wrapper still emerges as `SLANG_ParameterGroup_M_0`,
// but the inner element is `matrix<float,int(4),int(4)>` directly
// — contrast with the default column-major path which inserts a
// `_MatrixStorage_…natural_<N>` wrapper.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

cbuffer M
{
    row_major float4x4 mat;
}
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(mat, float4(float(tid.x), 0, 0, 1));
}

// CHECK: struct SLANG_ParameterGroup_M_0
// CHECK: matrix<float,int(4),int(4)>
