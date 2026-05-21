//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=99399bec1655fc5eb538f2f70fe7d6d9093a89bbf1f86fbe5246b36cbfc141d8
//META: purpose=Boundary: a 2x2 matrix element type in a structured buffer is still wrapped into a single-field struct, distinct from the 4x4 baseline.
//META: intent=boundary
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: smallest documented matrix dim (2x2). The
// wrapStructuredBuffersOfMatrices pass must wrap any matrix
// element regardless of dim, so the 2x2 case still appears as
// `struct _S<N> { float2x2 ... }` in HLSL emit.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

RWStructuredBuffer<float2x2> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = float2x2(1.0, 2.0, 3.0, 4.0);
}

// CHECK: struct _S{{[0-9]+}}
// CHECK: float2x2 _S{{[0-9]+}};
// CHECK: RWStructuredBuffer<_S{{[0-9]+}}{{.*}}> outBuf_{{[0-9]+}}
