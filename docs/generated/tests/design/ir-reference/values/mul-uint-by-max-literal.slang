//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=156280c3625197a4ed3b683ed5771a1ea94cd998308b23695241f7f96edbb6a7
//META: purpose=Boundary: `a * 4294967295u` (multiplication by `uint` MAX) lowers to `mul(%a, 4294967295 : UInt)` with the MAX literal preserved as the inline payload.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Multiplication by the unsigned MAX is the documented modular-
// wrap case for unsigned arithmetic. The opcode is unchanged
// and carries the literal MAX as the second operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a * 4294967295u;
}

// IR-LABEL: func %main
// IR: mul(%a, 4294967295 : UInt)
