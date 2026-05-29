//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=Negative: `bit_cast` between differently-sized types is rejected with `E41202: bit_cast size mismatch` — no `bitCast` IR opcode is produced.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc describes `bitCast` as "reinterpret bits without
// changing them". Reinterpreting requires identical bit width;
// `uint` (4 bytes) into `uint64_t` (8 bytes) cannot be a bit
// reinterpret and Slang rejects it at semantic check.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<uint64_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bit_cast<uint64_t>(a);
//CHECK:                       ^ bit_cast size mismatch
// CHECK: E41202
}
