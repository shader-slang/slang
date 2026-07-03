//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
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
