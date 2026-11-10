//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/05-ir-passes.md#type-and-value-legalization
//META: doc_section_digest=b9ff903836e65c228e0e2f2b3aec5862dda6d78f824babeb56251f657789e6bf
//META: purpose=The lower-enum-type pass renders a Slang enum constant as a plain integer in the HLSL emit; no enum type name remains.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The "Lower enum type" row of the type-and-value legalization
// table (`slang-ir-lower-enum-type.cpp`) lowers Slang enums to
// their underlying integer values. The HLSL emit shows the raw
// integer where the enum constant was used.

//TEST:SIMPLE(filecheck=HLSL):-target hlsl -stage compute -entry main

enum Color { Red = 0, Green = 1, Blue = 2 };

uniform RWStructuredBuffer<Color> buf;

[numthreads(1, 1, 1)]
void main(uint3 tid : SV_DispatchThreadID)
{
    buf[0] = Color.Green;
}

// HLSL: void main
// HLSL: int(1)
