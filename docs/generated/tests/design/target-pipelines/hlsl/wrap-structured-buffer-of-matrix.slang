//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=8041b8729de3f94a856ac55d2d8461ddffd491518349fcd47c2422a784e25a3a
//META: purpose=wrapStructuredBuffersOfMatrices wraps a matrix-typed StructuredBuffer element in a single-field struct so pack_matrix applies.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// HLSL emit replaces RWStructuredBuffer<float4x4> with a buffer
// whose element type is an anonymous struct (_S<N>) containing
// the matrix. The wave-6 lesson: the literal `row_major` does NOT
// survive as a token on HLSL; the struct-wrapping is the
// observable form.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float4x4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float4x4(1.0);
}

// CHECK: struct _S{{[0-9]+}}
// CHECK: float4x4 _S{{[0-9]+}};
// CHECK: RWStructuredBuffer<_S{{[0-9]+}}{{.*}}> outBuf_{{[0-9]+}} : register(u{{[0-9]+}})
