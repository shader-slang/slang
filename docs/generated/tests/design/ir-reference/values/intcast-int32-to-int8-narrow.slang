//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=Expansion: `int(32-bit) -> int8_t` narrowing across the full int ladder lowers to `intCast(%a)` with `Int8` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that one `intCast` opcode covers integer-to-integer
// conversions, with sign or zero extension chosen by the types. This probe drives the
// signed-to-signed narrowing edge from 32-bit down to 8-bit while
// preserving signedness — a different shape from the existing
// int -> uint8_t narrowing test.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform RWStructuredBuffer<int8_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int8_t(a);
}

// IR-LABEL: func %main
// IR: intCast(%a)
