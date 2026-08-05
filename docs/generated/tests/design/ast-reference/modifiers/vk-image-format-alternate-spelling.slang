//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#capability--target-attributes
//META: doc_section_digest=3a5e5ccd3c56586ff535ba4ba12aa25b1d20542d755d9561cb4e0be92dc6ca73
//META: purpose=The `[vk::image_format("...")]` spelling is the alternate form of the format attribute and selects the same emitted image format.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second documented spelling resolves to the same attribute as
// `[format(...)]`. A different format (`rgba16f`) is requested through the
// `vk::`-qualified name and CHECK asserts it reaches the emitted `OpTypeImage`,
// so the alternate spelling is shown to be equivalent rather than ignored.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

[vk::image_format("rgba16f")]
RWTexture2D<float4> tex;

[numthreads(1,1,1)]
void main()
{
    tex[uint2(0, 0)] = float4(1, 0, 0, 1);
}

//CHECK: OpTypeImage {{.*}}Rgba16f
