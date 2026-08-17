//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=Negative: `bit_cast` between differently-sized types is rejected with `E41202: bit_cast size mismatch` — no `bitCast` IR opcode is produced.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `bitCast` as "reinterpret bits without
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
