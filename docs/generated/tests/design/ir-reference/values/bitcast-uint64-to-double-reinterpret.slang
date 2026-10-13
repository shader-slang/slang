//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=fd36e77aadaffc9bfb74e35c83d2c6e2a74c726d09922af2aa3a95ce43fcac0c
//META: purpose=Expansion: `bit_cast<double>(uint64_t)` 64-bit same-width reinterpret lowers to `bitCast(%u64a)` with `Double` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 64-bit same-width reinterpret: 64-bit unsigned int -> 64-bit
// float. Verifies the `bitCast` opcode is the same shape at the
// 64-bit width.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform uint64_t u64a;
uniform RWStructuredBuffer<double> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bit_cast<double>(u64a);
}

// IR-LABEL: func %main
// IR: bitCast(%u64a)
