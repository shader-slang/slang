//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=99399bec1655fc5eb538f2f70fe7d6d9093a89bbf1f86fbe5246b36cbfc141d8
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
