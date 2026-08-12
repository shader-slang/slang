//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Boundary: shift by 0 (`a << 0`) lowers to `shl(%a, 0 : Int)` — the lower edge of the shift-amount axis.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `<<` produces `shl`. This boundary probe is the
// shift-by-zero case (lower edge of the amount axis); the
// opcode still surfaces with the literal `0 : Int` amount.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a << 0;
}

// IR-LABEL: func %main
// IR: shl(%a, 0 : Int)
