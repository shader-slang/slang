//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Expansion: `int8_t -> int16_t` one-step signed widening lowers to a single `intCast(%i8a)` with `Int16` result.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Smallest documented signed widening step: 8-bit to 16-bit. The
// doc places this under the same `intCast` opcode that handles all
// other int-int reshapes.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t i8a;
uniform RWStructuredBuffer<int16_t> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int16_t(i8a);
}

// IR-LABEL: func %main
// IR: intCast(%i8a)
