//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=15a849f7cabcf06b95570121d08d30f735912628e8a357779f17fa39de0aca01
//META: purpose=Expansion: `int8_t + int16_t` promotes the int8 operand via `intCast` to int16 before `add` — surfaces the same `intCast` opcode used for explicit constructor casts.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Mixed-rank integer arithmetic surfaces the documented `intCast`
// opcode for the implicit promotion to a common type before
// `add` runs.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int8_t i8a;
uniform int16_t i16a;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = int(i8a + i16a);
}

// IR-LABEL: func %main
// IR: intCast(%i8a)
// IR: add(%{{[0-9]+}}, %i16a)
