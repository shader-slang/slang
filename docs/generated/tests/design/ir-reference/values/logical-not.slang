//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Prefix `!` on a boolean lowers to the `not` opcode.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that prefix `!` produces the logical `not` opcode, which is
// a different opcode from the bitwise `bitnot` behind `~`. Applying `!`
// to a `uniform bool` keeps the operand non-constant, so CHECK observes
// `not(%c)` rather than a folded literal.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool c;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    bool nc = !c;
    buf[0] = select(nc, 1, 0);
}

// IR-LABEL: func %main
// IR: not(%c)
