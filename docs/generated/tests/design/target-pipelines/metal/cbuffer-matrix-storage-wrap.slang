//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/metal.md#wrapcbufferelementsformetal
//META: doc_section_digest=794e6b917e6a05e1bb5a790707b746f03faed4ff03cdce751a3d870c7ccc928f
//META: purpose=`wrapCBufferElementsForMetal` rewrites a `float4x4` cbuffer element into a `_MatrixStorage_..._natural_<N>` struct inside a `SLANG_ParameterGroup_<name>` wrapper.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the pass that makes constant-buffer elements expressible in
// MSL: Metal restricts what may appear as a `constant T*` element type, so
// the matrix is re-expressed as a struct holding an array of row vectors
// and the cbuffer itself is wrapped in a parameter-group struct. The
// kernel multiplies through the matrix so the field is live; CHECK asserts
// both wrapper struct names appear, with wildcards for the mangled
// suffixes.

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
