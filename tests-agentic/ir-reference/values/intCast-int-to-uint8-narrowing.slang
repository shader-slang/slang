//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Boundary: maximum-narrowing `int(32-bit) -> uint8_t(8-bit)` lowers to `intCast(%a)` with result type `UInt8` — verifying the same opcode covers the widest narrowing step in the documented int ladder.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The existing smoke test covers `int -> int16_t` (one step). This
// boundary test exercises the widest-narrowing case in the doc's
// integer ladder, `int -> uint8_t` (signed-32 to unsigned-8), so
// the same `intCast` opcode is the single shape for both.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<uint8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint8_t(a);
}

// IR-LABEL: func %main
// IR: intCast(%a)
