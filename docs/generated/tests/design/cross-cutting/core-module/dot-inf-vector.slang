//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: the HLSL meta-module's `dot` intrinsic accepts an (inf,inf,inf) argument and the HLSL backend preserves the `dot(` call.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: numeric edge — dot on an infinity vector. Build inf
// from 1.0/0.0 so the front-end accepts a non-finite operand, and
// pair with a tid-derived runtime vector to keep the intrinsic call
// observable in the emit.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float inf = 1.0 / 0.0;
    float3 infvec = float3(inf, inf, inf);
    float3 b = float3(float(tid.x), float(tid.y), float(tid.z));
    outBuf[tid.x] = dot(infvec, b);
}

// HLSL: dot(
