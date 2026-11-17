//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=bdab7c96cac9a55381381f37622841c41f387f80bb1577daf4c1cc5d5aa46b5d
//META: purpose=Boundary: an array-of-row_major-matrix field in a cbuffer still produces the standard SLANG_ParameterGroup struct shape.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: row_major matrices stored in an array inside a
// cbuffer. The cbuffer is still hoisted into a parameter-group
// struct and bound to a b-register.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

cbuffer Params {
    row_major float4x4 mats[3];
};
RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = mul(mats[tid.x % 3], float4(1, 2, 3, 4));
}

// CHECK: struct SLANG_ParameterGroup_Params{{.*}}
// CHECK: mats_{{[0-9]+}}
// CHECK: cbuffer Params_{{[0-9]+}} : register(b{{[0-9]+}})
