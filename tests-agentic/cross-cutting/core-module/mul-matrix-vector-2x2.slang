//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: the HLSL meta-module's `mul(matrix, vector)` accepts a 2x2 matrix (smallest practical square dim) and lowers per target.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: matrix dimension — `mul` on the smallest practical
// square matrix (2x2). Mirrors the functional `mul` test but at the
// small-dim edge.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage compute

RWStructuredBuffer<float2> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float2x2 m = float2x2(1, 2, 3, 4);
    float2 v = float2(float(tid.x), float(tid.y));
    outBuf[tid.x] = mul(m, v);
}

// HLSL: mul(
// SPIRV: OpVectorTimesMatrix
