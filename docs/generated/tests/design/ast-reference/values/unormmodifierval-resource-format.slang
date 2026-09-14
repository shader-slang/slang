//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#modifier-values
//META: doc_section_digest=cc9b5af8e7e88defd3d066b93057c4ed4b13419aa0e459c9e09a634d498558b7
//META: purpose=A resource-format modifier written on a texture element type is converted into a value stored on the modified type, and that stored modifier survives to code generation, spelled back in the HLSL element position and rendered as the SPIR-V image format.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the `unorm` written inside the texture's type argument
// is not discarded once the type expression is checked: it becomes part
// of the texture's element type and travels with it all the way to
// codegen. HLSL asserts the emitted declaration still spells `unorm` in
// the element position, which it can only do if the modifier was
// carried on the type rather than on the declaration syntax.
//
// SPIR-V observes the same stored modifier through the other syntax an
// image type has for a format: without the `unorm` the image's format
// operand is `Unknown`, and the modifier is the only thing in this file
// that can turn it into `Rgba8`. That makes SPIR-V a real second
// observation of the claim rather than a restatement of the HLSL one.
//
// GLSL, Metal and WGSL cannot express the claim; see the bundle
// README's `## Untested claims` rows for this claim.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -stage compute -entry main
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage compute -entry main

Texture2D<unorm float4> tex;
RWStructuredBuffer<float4> outBuf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    outBuf[0] = tex.Load(int3(0, 0, 0));
}

// CHECK: Texture2D<unorm float4
//SPIRV: OpTypeImage %float 2D {{[0-9 ]+}}Rgba8
