//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=19b077f47f71a1feb81adb9dea6ff4919edb188c235b3634dce08c45cf359e3c
//META: purpose=Boundary: shift by 32 (`a << 32`) on a 32-bit `uint` still lowers to `shl(%a, 32 : Int)` (one past the documented valid range) — the opcode appears in IR even though semantic-check emits a left-shift-overflow warning. Verifies that lowering proceeds past the warning, leaving the literal payload intact.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// One-past-edge on the shift-amount axis. The compiler emits a
// warning `E41030 left shift overflow` because 32 is not within
// the valid range for the 32-bit operand, but the IR still
// contains the `shl(%a, 32 : Int)` instruction. The boundary test
// asserts that the opcode shape is unchanged in IR by the warning
// path.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a << 32;
}

// IR-LABEL: func %main
// IR: shl(%a, 32 : Int)
