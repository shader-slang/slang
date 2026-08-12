//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Boundary: prefix `-` on a `uniform float` lowers to `neg(%fa)` — the float-typed sibling of the integer `neg` exercised in the smoke test.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that prefix `-` produces the same single `neg` opcode
// regardless of operand type. The smoke covers `int`; this boundary probe
// covers `float`. (Float negation handles ±0.0, ±inf, and NaN per
// IEEE-754; the IR opcode shape is the same.)

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = -fa;
}

// IR-LABEL: func %main
// IR: neg(%fa)
