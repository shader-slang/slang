//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=5acb4ce614573d6b46a7a78f0ba6dac2fcbcebec94dcc7e7e1fc9c7cbb5bc1fd
//META: purpose=Boundary: a matrix declared in a cbuffer without a row_major/column_major qualifier emits as plain `float4x4` under the default column_major pragma.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: default matrix layout (no qualifier). The
// emitter relies on `#pragma pack_matrix(column_major)` to set
// the convention and does not need a wrapper struct.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params {
    float4x4 m;
};
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(m, float4(1, 2, 3, 4));
}

// CHECK: float4x4 m_{{[0-9]+}};
// CHECK: cbuffer Params_{{[0-9]+}} : register(b{{[0-9]+}})
