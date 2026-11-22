//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=bdab7c96cac9a55381381f37622841c41f387f80bb1577daf4c1cc5d5aa46b5d
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
