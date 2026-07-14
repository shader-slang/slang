//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#interpolation-modes
//META: doc_section_digest=24e7271a64c5842b122da8c1f81403c6afa1794c0903bde19c22179fa5e608b4
//META: purpose=`nointerpolation` on a pixel-shader varying input survives into emitted HLSL.
//META: intent=functional
//META: pipeline_stage=emit
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `HLSLNoInterpolationModifier` is the modifier behind the
// `nointerpolation` keyword. A varying-input field carrying
// `nointerpolation` must emit as `nointerpolation` in HLSL.

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
