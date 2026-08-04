//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=68e6f8fe2ef26c5eb509b197e00cfd6b92f20ff7baca61aabbd1d7d712edf820
//META: purpose=Boundary: a column_major matrix in a cbuffer does NOT need a _MatrixStorage_ wrapper because it matches the prelude pack_matrix(column_major) default.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: column_major (matches the default) — the emit
// should keep the plain `float4x4` spelling in the parameter-group
// struct, not introduce a `_MatrixStorage_` wrapper.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params {
    column_major float4x4 m;
};
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(m, float4(1, 2, 3, 4));
}

// CHECK: struct SLANG_ParameterGroup_Params{{.*}}
// CHECK: float4x4 m_{{[0-9]+}};
// CHECK: cbuffer Params_{{[0-9]+}} : register(b{{[0-9]+}})
