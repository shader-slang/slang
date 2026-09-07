//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#interpolation-modes
//META: doc_section_digest=b00757f585ffb1d581749d3144f4d738cfa61d44114ac021c2e6dc15ad86eb5e
//META: purpose=The `sample` interpolation modifier is preserved through emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `sample` -- the per-sample sibling of `nointerpolation` in the same
// interpolation-mode family -- is likewise preserved to the back end. The
// annotated varying is returned so it survives DCE, and CHECK asserts the
// qualifier is present in the emitted HLSL.

// The per-sample request has a spelling on each of the other shader back ends
// too -- GLSL `sample`, Metal `[[sample_perspective]]`, SPIR-V a `Sample`
// decoration -- so each prefix pins that back end's own form of the rate.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=GLSL):-target glsl -entry main -stage fragment
//TEST:SIMPLE(filecheck=METAL):-target metal -entry main -stage fragment
//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -entry main -stage fragment

struct VsOut
{
    float4 pos              : SV_Position;
    sample float4 perSample : COLOR0;
};

float4 main(VsOut v) : SV_Target
{
    return v.perSample;
}

//CHECK: sample

//GLSL: sample layout(location = 0)
//METAL: {{\[\[}}sample_perspective{{\]\]}} float4 perSample_
//SPIRV: OpDecorate %v_perSample Sample
