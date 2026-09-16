//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/cross-cutting/targets.md#profiles
//META: doc_section_digest=469973558ae9131458179def57cec815ce506922c2ef82e0e161360ad76f32c5
//META: purpose=A `Profile` pins a `Stage` plus a feature-level `Version`; `-profile sm_6_0` parses and its capability set lets a compute kernel compile on HLSL, and `-profile glsl_450` does the same on GLSL.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a profile name pins a stage plus a feature-level
// version and that the resulting capability set admits a plain
// compute kernel. Two spellings from different version families —
// Shader Model 6.0 and GLSL 450 — are each paired with the matching
// text target; the CHECKs assert the compute entry point survives to
// emit (`numthreads` / `local_size_x`), which only happens if the
// profile's atoms were admitted rather than rejected.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -profile sm_6_0 -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -profile glsl_450 -entry main -stage compute

RWStructuredBuffer<int> outBuf;

[shader("compute")]
[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[tid.x] = 1;
}

// HLSL: numthreads(1, 1, 1)
// GLSL: #version 4{{[0-9]+}}
// GLSL: local_size_x = 1
