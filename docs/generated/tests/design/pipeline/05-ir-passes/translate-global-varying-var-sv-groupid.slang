//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=7c4335ef62cc708fc063656220fb481b4e3fdbe6592973946fe16a93c217487e
//META: purpose=Expansion: translate-global-varying-var routes `SV_GroupID` and `SV_GroupThreadID` into per-target stage-input markers on HLSL and GLSL (alternative semantic shapes to SV_DispatchThreadID).
//META: intent=expansion
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Parameter-shape variation on translate-global-varying-var: an
// entry point that consumes the alternative built-in semantics
// `SV_GroupID` and `SV_GroupThreadID`. Each routes through the
// per-target stage-input shape.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=GLSL):-target glsl -stage compute -entry main

uniform RWStructuredBuffer<uint> buf;

[numthreads(4, 1, 1)]
void main(uint3 gid : SV_GroupID, uint3 gtid : SV_GroupThreadID)
{
    buf[0] = gid.x + gtid.x;
}

// HLSL-DAG: SV_GroupID
// HLSL-DAG: SV_GroupThreadID
// GLSL-DAG: gl_WorkGroupID
// GLSL-DAG: gl_LocalInvocationID
