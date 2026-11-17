//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e9d21ff4a25410c2c41b9f2413bf081c7a9f699c2230818fe1a6fdf8979075a6
//META: purpose=Expansion: reverse 64-bit reinterpret `bit_cast<uint64_t>(double)` lowers to `bitCast(%da)` with `UInt64` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reverse direction of the 64-bit same-width reinterpret pair.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform double da;
uniform RWStructuredBuffer<uint64_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = bit_cast<uint64_t>(da);
}

// IR-LABEL: func %main
// IR: bitCast(%da)
