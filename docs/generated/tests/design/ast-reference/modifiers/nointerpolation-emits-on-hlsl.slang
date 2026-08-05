//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#interpolation-modes
//META: doc_section_digest=b00757f585ffb1d581749d3144f4d738cfa61d44114ac021c2e6dc15ad86eb5e
//META: purpose=`nointerpolation` on a fragment varying input survives into emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the `nointerpolation` interpolation mode reaches codegen rather
// than being erased with the rest of the varying plumbing. The annotated field
// is returned from the fragment shader so it stays live, and CHECK asserts the
// emitted HLSL parameter still carries the qualifier.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry main -stage fragment

struct VsOut
{
    float4 pos     : SV_Position;
    nointerpolation float4 flat : COLOR0;
};

float4 main(VsOut v) : SV_Target
{
    return v.flat;
}

//CHECK: nointerpolation
