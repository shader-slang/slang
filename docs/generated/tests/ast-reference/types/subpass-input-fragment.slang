//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-22T00:00:00+00:00
//META: source_commit=6e923e2c0fe3cae4e7cf40e25a96569df5b9f009
//META: doc_ref=docs/generated/design/ast-reference/types.md#resource--texture--sampler-types
//META: doc_section_digest=1fea6aaeec3d54a63a589bef03489a7dd7676c39ed93549751670b987cd46065
//META: purpose=A SubpassInputType is the AST type for Vulkan `SubpassInput<T>`; the GLSL backend renders the binding as an `input_attachment_index` decoration in a fragment entry point.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `SubpassInputType` is the `BuiltinType` leaf for the Vulkan
// `SubpassInput<T>` user spelling. It is observable via GLSL emit
// text from a fragment entry point: the `input_attachment_index`
// layout token appears and a subpass-load intrinsic is generated.

//TEST:SIMPLE(filecheck=CHECK):-target glsl -stage fragment -entry main

[[vk::input_attachment_index(0)]] SubpassInput<float4> inputColor;

//CHECK: input_attachment_index
//CHECK: subpassInput
//CHECK: subpassLoad
float4 main() : SV_Target
{
    return inputColor.SubpassLoad();
}
