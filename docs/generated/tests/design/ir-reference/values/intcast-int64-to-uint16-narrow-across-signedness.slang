//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Expansion: `int64_t -> uint16_t` narrowing across signedness lowers to a single `intCast(%i64a)` with `UInt16` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Combines a 48-bit width drop with a signed-to-unsigned flip.
// Verifies that lowering keeps both steps under one `intCast` opcode keyed by
// the result type.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int64_t i64a;
uniform RWStructuredBuffer<uint16_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = uint16_t(i64a);
}

// IR-LABEL: func %main
// IR: intCast(%i64a)
