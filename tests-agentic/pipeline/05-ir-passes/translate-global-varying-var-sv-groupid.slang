//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1655c2bf8d3567fa220a5226769ef5e3917d55e8
//META: doc_ref=docs/llm-generated/pipeline/05-ir-passes.md#layout-and-binding
//META: doc_section_digest=06fba085c49c1b3a2b0f9fcc4941f5b7e0c382078660bd5b19c43e0aa5b68350
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
