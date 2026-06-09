//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=e98041c33a94ddbbb2f967b2078765fb561c2b5e826bdd9ca046a44763e172fe
//META: purpose=A precision-changing float-to-float cast lowers to `floatCast`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Conversions table lists `floatCast` as the precision-change
// operator. Casting a `float` to a `half` (narrowing) or to a
// `double` (widening) surfaces it.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<half> hbuf;
uniform RWStructuredBuffer<double> dbuf;

[numthreads(1, 1, 1)]
void main()
{
    hbuf[0] = half(fa);
    dbuf[0] = double(fa);
}

// IR-LABEL: func %main
// IR: floatCast(%fa)
// IR: floatCast(%fa)
