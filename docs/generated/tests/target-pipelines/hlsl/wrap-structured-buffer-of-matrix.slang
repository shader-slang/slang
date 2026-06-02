//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:16:11+00:00
//META: source_commit=ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=99399bec1655fc5eb538f2f70fe7d6d9093a89bbf1f86fbe5246b36cbfc141d8
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
