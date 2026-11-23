//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=156280c3625197a4ed3b683ed5771a1ea94cd998308b23695241f7f96edbb6a7
//META: purpose=Boundary: `bitfieldInsert(a, b, 0, 32)` (full-width replace on a 32-bit `uint`) lowers to `bitfieldInsert(%a, %b, 0 : UInt, 32 : UInt)` — the upper edge of the bitfield-width range.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `bitfieldInsert` operand layout is `base, insert, offset,
// count`. This boundary probe takes the full-width replace case —
// the result is semantically identical to a plain copy of `b`
// into the result type, but the opcode still surfaces with the
// 32 count payload.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform uint a;
uniform uint b;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bitfieldInsert(a, b, 0, 32);
}

// IR-LABEL: func %main
// IR: bitfieldInsert(%a, %b, 0 : UInt, 32 : UInt)
