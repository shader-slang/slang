//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T15:00:00+00:00
//META: source_commit=ef1068b5485e09b3a7afadba2e25f9541e29af42
//META: doc_ref=docs/generated/design/target-pipelines/spirv.md#option-set-toggles
//META: doc_section_digest=730d07f4cad01d2edbbbf03523fae836c3254cc356ffd1ac578d0a21bad19381
//META: purpose=Verifies -fvk-bind-globals N set places the $Globals cbuffer at SPIR-V Binding N / DescriptorSet set instead of the default 0/0.
//META: intent=characterization
//META: covers=source/slang/slang-options.cpp
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Characterization of OptionKind::VulkanBindGlobals (-fvk-bind-globals <N> <set>),
// a two-operand CLI option. A module-scope `uniform` collects into the implicit
// $Globals cbuffer (emitted as %globalParams). The BOUND directive pins the
// chosen Binding 7 / DescriptorSet 3 decorations; the DEFAULT directive (no flag)
// pins the default Binding 0 / DescriptorSet 0, confirming the option's operands
// flow through to the SPIR-V binding decorations.

//TEST:SIMPLE(filecheck=BOUND):-target spirv-asm -fvk-bind-globals 7 3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=DEFAULT):-target spirv-asm -entry main -stage fragment

uniform float4 gColor;

[shader("fragment")]
float4 main() : SV_Target
{
    return gColor;
}

//BOUND-DAG: OpDecorate %globalParams Binding 7
//BOUND-DAG: OpDecorate %globalParams DescriptorSet 3
//DEFAULT-DAG: OpDecorate %globalParams Binding 0
//DEFAULT-DAG: OpDecorate %globalParams DescriptorSet 0
