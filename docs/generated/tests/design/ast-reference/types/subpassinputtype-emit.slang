//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource-and-texture-type-families
//META: doc_section_digest=0036091ab5f0cd50c2bada4820b7c7d7db668e207cf8e5052d6b074f59b4585f
//META: purpose=A SubpassInputType `SubpassInput<T>` is a Vulkan subpass-input resource type that each backend lowers to its own attachment form.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the subpass-input resource type is not lowered like an
// ordinary texture: each backend has its own attachment form. A
// fragment entry point loads the attachment and returns it, and the
// per-target CHECKs pin HLSL's SubpassInput spelling, Metal's colour
// attachment, and SPIR-V's SubpassData image.

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
