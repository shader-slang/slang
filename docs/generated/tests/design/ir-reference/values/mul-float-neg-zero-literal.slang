//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=f6737af21fc0ce62bf35ca601aa6d8bc483fabcafaedad874ca80c1323a37207
//META: purpose=Boundary: negative-zero `-0.0` literal survives as `-0 : Float` on a `mul` operand, distinct from positive `0` in the IR payload.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// IEEE-754 has two distinct zero bit patterns. Verifies that the literal
// payload encoding rule implies the dump differentiates them;
// the IR formatter prints `0` and `-0` separately for the two
// zero values on the operand of a `mul`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = fa * -0.0;
}

// IR-LABEL: func %main
// IR: mul(%fa, -0 : Float)
