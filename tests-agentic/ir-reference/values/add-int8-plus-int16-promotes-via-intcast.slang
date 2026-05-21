//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
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
