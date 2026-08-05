//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#capability--target-attributes
//META: doc_section_digest=3a5e5ccd3c56586ff535ba4ba12aa25b1d20542d755d9561cb4e0be92dc6ca73
//META: purpose=The `[format("...")]` attribute selects the image format recorded on the emitted SPIR-V image type.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the attribute's format argument is carried into layout and lands on
// the image type, not just parsed. The texture asks for `rgba32f` and CHECK
// asserts the emitted `OpTypeImage` names `Rgba32f`; without the attribute the
// image type would carry the unknown format instead.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute

[format("rgba32f")]
RWTexture2D<float4> tex;

[numthreads(1,1,1)]
void main()
{
    tex[uint2(0, 0)] = float4(1, 0, 0, 1);
}

//CHECK: OpTypeImage {{.*}}Rgba32f
