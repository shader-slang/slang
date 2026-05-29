//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:31:07+00:00
//META: source_commit=019ec0e5ddac1c14fbad141a3f03d50cf8a5e022
//META: doc_ref=docs/generated/design/target-pipelines/hlsl.md#wrapstructuredbuffersofmatrices
//META: doc_section_digest=99399bec1655fc5eb538f2f70fe7d6d9093a89bbf1f86fbe5246b36cbfc141d8
//META: purpose=Boundary: a function parameter typed `inout row_major float4x4` carries the row_major qualifier through to the emitted HLSL function signature.
//META: intent=boundary
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: row_major on a function parameter (not in a
// cbuffer). The cbuffer-specific `_MatrixStorage_` rewrite does
// not apply here; the qualifier survives literally.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage compute

void rotate(inout row_major float4x4 m)
{
    m = transpose(m);
}

RWStructuredBuffer<float4> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    row_major float4x4 m = float4x4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1);
    rotate(m);
    outBuf[tid.x] = mul(m, float4(1, 2, 3, 4));
}

// CHECK: inout row_major float4x4
