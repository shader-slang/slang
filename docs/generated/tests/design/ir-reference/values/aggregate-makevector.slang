//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=136fa0f20043e0a8db805b7f018d292446250e6117047722c77e7aa7efca0675
//META: purpose=A `floatN(...)` vector-constructor call with non-constant component values lowers to `makeVector(%a, %b, ...)` of type `Vec(Float, N : Int)`.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Aggregate constructors table lists `makeVector` for
// "vector-constructor calls". Building a `float3` and a `float4`
// from `uniform` operands and writing each to a vector buffer
// surfaces two `makeVector(...)` lines in the IR dump.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform float fb;
uniform float fc;
uniform float fd;
uniform RWStructuredBuffer<float3> vbuf;
uniform RWStructuredBuffer<float4> v4buf;

[numthreads(1, 1, 1)]
void main()
{
    vbuf[0] = float3(fa, fb, fc);
    v4buf[0] = float4(fa, fb, fc, fd);
}

// IR-LABEL: func %main
// IR: makeVector(%fa, %fb, %fc)
// IR: makeVector(%fa, %fb, %fc, %fd)
