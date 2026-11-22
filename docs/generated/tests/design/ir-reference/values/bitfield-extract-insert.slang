//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=156280c3625197a4ed3b683ed5771a1ea94cd998308b23695241f7f96edbb6a7
//META: purpose=The `bitfieldExtract(v, off, count)` and `bitfieldInsert(b, ins, off, count)` builtins lower to the IR opcodes of the same names.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Arithmetic and bitwise table lists `bitfieldExtract` and
// `bitfieldInsert` with AST origins "`BitfieldExtract` intrinsic"
// and "`BitfieldInsert` intrinsic". Both opcodes carry their
// offset/count operands inline as `<N> : UInt` payloads.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform uint a;
uniform uint b;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bitfieldExtract(a, 4, 8);
    buf[1] = bitfieldInsert(a, b, 4, 8);
}

// IR-LABEL: func %main
// IR: bitfieldExtract(%a, 4 : UInt, 8 : UInt)
// IR: bitfieldInsert(%a, %b, 4 : UInt, 8 : UInt)
