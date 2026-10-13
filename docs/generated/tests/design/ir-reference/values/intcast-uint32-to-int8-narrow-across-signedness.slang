//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=fd36e77aadaffc9bfb74e35c83d2c6e2a74c726d09922af2aa3a95ce43fcac0c
//META: purpose=Expansion: `uint32 -> int8_t` narrowing across signedness lowers to a single `intCast(%a)` with `Int8` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Unsigned-to-signed narrow: 32-bit unsigned to 8-bit signed.
// Verifies that lowering keeps this under the same `intCast` shape as the
// signed-narrow and same-rank-flip cases.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform uint a;
uniform RWStructuredBuffer<int8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int8_t(a);
}

// IR-LABEL: func %main
// IR: intCast(%a)
