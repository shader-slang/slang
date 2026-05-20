//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
//META: purpose=The `bitfieldExtract(v, off, count)` and `bitfieldInsert(b, ins, off, count)` builtins lower to the IR opcodes of the same names.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Arithmetic and bitwise table lists `bitfieldExtract` and
// `bitfieldInsert` with AST origins "`BitfieldExtract` intrinsic"
// and "`BitfieldInsert` intrinsic". Both opcodes carry their
// offset/count operands inline as `<N> : UInt` payloads.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

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
