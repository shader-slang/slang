//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/llm-generated/cross-cutting/core-module.md#core-module
//META: doc_section_digest=2bfd479afe0dc76144283c14d90b22a6ac0fd8d03be7907d3412c1e2f3944a4b
//META: purpose=Boundary: the HLSL meta-module's `length` intrinsic accepts a (0,0,0) vector; the HLSL backend preserves the `length(` call.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: numeric edge — length of the zero vector. Route the
// zero through a single tid-derived scale so the call survives
// constant folding in the HLSL emitter.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float zero_scale = float(tid.x) * 0.0;
    float3 zerovec = float3(zero_scale, zero_scale, zero_scale);
    outBuf[tid.x] = length(zerovec);
}

// HLSL: length(
