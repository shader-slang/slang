//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
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
