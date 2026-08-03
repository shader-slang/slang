//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00Z
//META: source_commit=60db9042207670b39b0524118887efacb7a3d42d
//META: doc_ref=docs/generated/design/cross-cutting/core-module.md#core-module
//META: doc_section_digest=a1c3e1c21f5e7d5f5d6098ee8202f97eec56a5706e678fd2be1b3844ebe8539e
//META: purpose=Boundary: the HLSL meta-module's `length` intrinsic accepts a NaN vector; the HLSL backend preserves the `length(` call.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis: numeric edge — length of a NaN vector. Construct
// NaN from 0/0; multiply by tid-derived value so the front-end keeps
// the call instead of folding it away.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage compute

RWStructuredBuffer<float> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    float nan_val = (0.0 / 0.0) + float(tid.x) * 0.0;
    float3 nanvec = float3(nan_val, nan_val, nan_val);
    outBuf[tid.x] = length(nanvec);
}

// HLSL: length(
