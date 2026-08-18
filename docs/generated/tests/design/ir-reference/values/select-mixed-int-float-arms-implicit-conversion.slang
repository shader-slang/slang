//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Expansion: `select(c, intVal, floatVal)` implicitly converts the int arm via `castIntToFloat` so both arms meet a common float type; the `select` opcode then takes two `Float` operands.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that both `select` result operands share a
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
