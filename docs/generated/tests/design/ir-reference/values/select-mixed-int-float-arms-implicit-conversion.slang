//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=d86bedaed855595e10c45be817dcd07ae903a5905ada08a17d56c55e59daa232
//META: purpose=Expansion: `select(c, intVal, floatVal)` implicitly converts the int arm via `castIntToFloat` so both arms meet a common float type; the `select` opcode then takes two `Float` operands.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc requires that both `select` result operands share a
// common type. When the two arms are int and float, the int arm
// is promoted via `castIntToFloat` before `select`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform bool c;
uniform int ia;
uniform float fb;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(c, ia, fb);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%ia)
// IR: select(%c, %{{[0-9]+}}, %fb)
