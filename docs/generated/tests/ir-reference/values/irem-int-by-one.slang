//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
//META: purpose=Boundary: integer remainder by 1 (`a % 1`) lowers to `irem(%a, 1 : Int)` — the smallest non-zero divisor edge for the documented `irem` opcode.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `% 0` is rejected by semantic check; the smallest valid divisor
// is 1. The IR preserves the `irem` opcode with the divisor as
// the inline payload.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = a % 1;
}

// IR-LABEL: func %main
// IR: irem(%a, 1 : Int)
