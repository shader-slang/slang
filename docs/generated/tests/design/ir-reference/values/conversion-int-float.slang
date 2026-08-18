//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e048ee63e8d3331531d544e35ff8c335c38d075b516814b79f5d10d0602ac72a
//META: purpose=Constructor-style `float(intVal)` lowers to `castIntToFloat`; `int(floatVal)` lowers to `castFloatToInt`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Conversions table lists `castIntToFloat` (int-to-float
// conversion) and `castFloatToInt` (float-to-int truncation).
// Constructor-style casts on uniform operands surface each.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform int a;
uniform float fa;
uniform RWStructuredBuffer<int> buf;
uniform RWStructuredBuffer<float> fbuf;

[numthreads(1, 1, 1)]
void main()
{
    fbuf[0] = float(a);
    buf[0] = int(fa);
}

// IR-LABEL: func %main
// IR: castIntToFloat(%a)
// IR: castFloatToInt(%fa)
