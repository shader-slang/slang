//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Stress: four chained `mul(matrix, vector)` calls all survive HLSL emit.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: chained meta-module `mul` calls — feeding the
// previous result into the next mul to probe lowering under nesting.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3x3 m = float3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);
    float3 v = float3(float(tid.x), float(tid.y), float(tid.z));
    v = mul(m, v);
    v = mul(m, v);
    v = mul(m, v);
    v = mul(m, v);
    outBuf[tid.x] = v;
}

// HLSL: mul(
// HLSL: mul(
// HLSL: mul(
// HLSL: mul(
