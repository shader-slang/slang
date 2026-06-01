//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Stress: eight back-to-back `dot` intrinsic calls in one expression all survive HLSL emit.
//META: intent=stress
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Stress axis: many uses of the same meta-module intrinsic in one
// expression — probes the intrinsic-lowering path under a deep
// expression tree.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3 v = float3(float(tid.x), float(tid.y), float(tid.z));
    float3 a = float3(1.0, 2.0, 3.0);
    float r = dot(v, a)
            + dot(v, a + 1.0)
            + dot(v, a + 2.0)
            + dot(v, a + 3.0)
            + dot(v, a + 4.0)
            + dot(v, a + 5.0)
            + dot(v, a + 6.0)
            + dot(v, a + 7.0);
    outBuf[tid.x] = r;
}

// HLSL: dot(
// HLSL: dot(
// HLSL: dot(
// HLSL: dot(
// HLSL: dot(
// HLSL: dot(
// HLSL: dot(
// HLSL: dot(
