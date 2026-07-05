//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:50:00Z
//META: source_commit=2aa9f69f5e2e75f6e2f4231a451a1a022818e18b
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=2a4fe67f3530d551446e5a52ff530daaceb47c541972fbd77fc4832e5e0aa6a6
//META: purpose=wrapCBufferElementsForMetal wraps a float4x4 cbuffer element in a _MatrixStorage_..._natural_<N> struct so the emitted MSL is valid.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Metal cannot reference a bare `float4x4` element of a constant
// buffer; `wrapCBufferElementsForMetal` rewrites the element as a
// `_MatrixStorage_…natural_<N>` struct holding the matrix as an
// `array<float4, int(4)>`. The cbuffer struct wrapper is
// `SLANG_ParameterGroup_<name>_natural_<M>`.

//TEST:SIMPLE(filecheck=CHECK):-target metal -entry main -stage compute

cbuffer M
{
    float4x4 mat;
}
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(mat, float4(float(tid.x), 0, 0, 1));
}

// CHECK: struct _MatrixStorage_{{.*}}natural_{{[0-9]+}}
// CHECK: struct SLANG_ParameterGroup_M_{{.*}}
