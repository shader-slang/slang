//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#capability--target-attributes
//META: doc_section_digest=ce8106cbb762f8a84c2c1448718332b6001d57168b92318fbbf9d952624061de
//META: purpose=The `[vk::image_format("...")]` spelling is the alternate form of the format attribute and selects the same emitted image format.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second documented spelling resolves to the same attribute as
// `[format(...)]`. A different format (`rgba16f`) is requested through the
// `vk::`-qualified name and CHECK asserts it reaches the emitted `OpTypeImage`,
// so the alternate spelling is shown to be equivalent rather than ignored.

// The GLSL back end reads the same attribute and turns it into the image
// uniform's `layout(rgba16f)` qualifier, so the GLSL prefix shows the alternate
// spelling is resolved before target selection rather than in a SPIR-V-only
// path.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage compute

[vk::image_format("rgba16f")]
RWTexture2D<float4> tex;

[numthreads(1,1,1)]
void main()
{
    tex[uint2(0, 0)] = float4(1, 0, 0, 1);
}

//CHECK: OpTypeImage {{.*}}Rgba16f

//GLSL: layout(rgba16f)
