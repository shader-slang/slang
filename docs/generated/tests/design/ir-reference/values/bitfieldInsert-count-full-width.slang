//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
//META: purpose=Boundary: `bitfieldInsert(a, b, 0, 32)` (full-width replace on a 32-bit `uint`) lowers to `bitfieldInsert(%a, %b, 0 : UInt, 32 : UInt)` — the upper edge of the bitfield-width range.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The `bitfieldInsert` operand layout is `base, insert, offset,
// count`. This boundary probe takes the full-width replace case —
// the result is semantically identical to a plain copy of `b`
// into the result type, but the opcode still surfaces with the
// 32 count payload.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

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
