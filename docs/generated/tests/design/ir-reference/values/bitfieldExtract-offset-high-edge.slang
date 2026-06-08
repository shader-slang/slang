//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
//META: purpose=Boundary: `bitfieldExtract(a, 31, 1)` (top-bit extract on a 32-bit `uint`) lowers to `bitfieldExtract(%a, 31 : UInt, 1 : UInt)` — the high-edge of the offset axis.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The high-edge of the offset axis: offset = width - 1, count = 1.
// This is the canonical "extract the sign bit" pattern on a
// 32-bit unsigned value.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bitfieldExtract(a, 31, 1);
}

// IR-LABEL: func %main
// IR: bitfieldExtract(%a, 31 : UInt, 1 : UInt)
