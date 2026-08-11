//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Boundary: float divide by positive-zero literal (`fa / 0.0`) lowers to `div(%a, 0 : Float)` — the `div` opcode survives even when the divisor is the IEEE-754 +zero, where integer divide-by-zero is rejected as a diagnostic.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `/` produces `div`, with the signed / unsigned /
// floating-point flavour keyed by operand type rather than by a
// separate opcode. On
// floats, divide-by-zero is allowed (produces +/-inf or NaN per
// IEEE-754) and the IR retains the `div` opcode with the zero
// payload directly on its operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = fa / 0.0;
}

// IR-LABEL: func %main
// IR: div(%fa, 0 : Float)
