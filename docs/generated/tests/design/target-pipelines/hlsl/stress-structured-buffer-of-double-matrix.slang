//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=68e6f8fe2ef26c5eb509b197e00cfd6b92f20ff7baca61aabbd1d7d712edf820
//META: purpose=Stress: a StructuredBuffer<double4x4> (largest documented matrix dim, 64-bit element) still triggers the single-field wrapper struct rewrite.
//META: intent=stress
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: largest documented matrix dim (4x4) over 64-bit
// element type. The wrapStructuredBuffersOfMatrices pass still
// introduces a single-field wrapper struct so the column_major
// pragma applies.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

StructuredBuffer<double4x4> inBuf;
RWStructuredBuffer<double4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(inBuf[tid.x], double4(1, 2, 3, 4));
}

// CHECK: struct _S{{[0-9]+}}
// CHECK: matrix<double{{.*}}> _S{{[0-9]+}};
// CHECK: StructuredBuffer<_S{{[0-9]+}}{{.*}}> inBuf_{{[0-9]+}} : register(t{{[0-9]+}})
