//META: generated=true
//META: model=claude-opus-4-8[1m]
//META: generated_at=2026-06-11T00:00:00+00:00
//META: source_commit=1ee81dae21143fb69ccb9999f0fd013b4db2d922
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource--texture--sampler-types
//META: doc_section_digest=1fea6aaeec3d54a63a589bef03489a7dd7676c39ed93549751670b987cd46065
//META: purpose=A SubpassInputType is the AST type for Vulkan `SubpassInput<T>`; the GLSL backend renders the binding as an `input_attachment_index` decoration in a fragment entry point.
//META: intent=expansion
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Fans the SubpassInputType emission claim out to the HLSL, Metal, and
// SPIR-V text targets the sibling subpass-input-fragment.slang (GLSL)
// does not cover. The subpass-input handle lowers to a target-specific
// attachment form: HLSL keeps the `SubpassInput<...>` spelling on a
// `register(t0)` binding, Metal lowers it to a `color(0)` fragment-input
// attachment, SPIR-V to an `OpTypeImage ... SubpassData`. (WGSL has no
// subpass-input feature; see Untested claims unsupported-on-target wgsl.)
// Each CHECK pins the token that identifies that target's subpass form,
// copied verbatim from slangc output.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage fragment -entry main
//TEST:SIMPLE(filecheck=METAL):-target metal -stage fragment -entry main
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -stage fragment -entry main

[[vk::input_attachment_index(0)]] SubpassInput<float4> inputColor;

//HLSL: SubpassInput<float4> inputColor
//METAL: inputColor{{.*}}color(0)
//SPV: OpTypeImage %float SubpassData
float4 main() : SV_Target
{
    return inputColor.SubpassLoad();
}
