//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: the HLSL meta-module's `mul(matrix, vector)` accepts an all-zero matrix and the HLSL backend preserves the `mul(` call.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: matrix value — `mul` with an all-zero 3x3 matrix.
// Pair with a tid-derived vector so the call is preserved by the
// HLSL emitter rather than folded to a zero literal.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float3> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float3x3 m = float3x3(0, 0, 0,
                          0, 0, 0,
                          0, 0, 0);
    float3 v = float3(float(tid.x), float(tid.y), float(tid.z));
    outBuf[tid.x] = mul(m, v);
}

// HLSL: mul(
