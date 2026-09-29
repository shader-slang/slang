//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#conversions
//META: doc_section_digest=fd36e77aadaffc9bfb74e35c83d2c6e2a74c726d09922af2aa3a95ce43fcac0c
//META: purpose=A precision-changing float-to-float cast lowers to `floatCast`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Conversions table lists `floatCast` as the precision-change
// operator. Casting a `float` to a `half` (narrowing) or to a
// `double` (widening) surfaces it.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

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
