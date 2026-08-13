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

// Emission fan-out: although the flag is spelled `-fvk-*`, the binding it
// assigns is a *layout* decision made before back-end emission, so every text
// target renders the same chosen slot in its own spelling — GLSL
// `layout(binding = 7, set = 3)`, HLSL `register(b7, space3)`, Metal
// `[[buffer(7)]]`, WGSL `@binding(7) @group(3)`. Each target therefore gets a
// flag/no-flag pair pinning its own form of the delta.

//TEST:SIMPLE(filecheck=BOUND):-target spirv-asm -fvk-bind-globals 7 3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=DEFAULT):-target spirv-asm -entry main -stage fragment
//TEST:SIMPLE(filecheck=GBIND):-target glsl -fvk-bind-globals 7 3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=GDEF):-target glsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=HBIND):-target hlsl -fvk-bind-globals 7 3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=HDEF):-target hlsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=MBIND):-target metal -fvk-bind-globals 7 3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=MDEF):-target metal -entry main -stage fragment
//TEST:SIMPLE(filecheck=WBIND):-target wgsl -fvk-bind-globals 7 3 -entry main -stage fragment
//TEST:SIMPLE(filecheck=WDEF):-target wgsl -entry main -stage fragment

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

//GBIND: layout(binding = 7, set = 3)
//GDEF: layout(binding = 0)

//HBIND: cbuffer globalParams_{{[0-9]+}} : register(b7, space3)
//HDEF: cbuffer globalParams_{{[0-9]+}} : register(b0)

//MBIND: constant* globalParams_{{[0-9]+}} {{\[\[}}buffer(7){{\]\]}}
//MDEF: constant* globalParams_{{[0-9]+}} {{\[\[}}buffer(0){{\]\]}}

//WBIND: @binding(7) @group(3) var<uniform> globalParams_{{[0-9]+}}
//WDEF: @binding(0) @group(0) var<uniform> globalParams_{{[0-9]+}}
