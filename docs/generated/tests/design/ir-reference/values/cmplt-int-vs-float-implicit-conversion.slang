//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Expansion: comparing `int < float` triggers an implicit `castIntToFloat` on the int operand before `cmpLT`; verifies the documented conversion opcode bridges the type mismatch.
//META: intent=expansion
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Conversions table is the only place that names how mixed
// int/float operands meet a common type. The implicit conversion
// surfaces a `castIntToFloat` before the comparison.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int ia;
uniform float fa;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(ia < fa, 1, 0);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%ia)
// IR: cmpLT(%{{[0-9]+}}, %fa)
