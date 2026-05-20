//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
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
