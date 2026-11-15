//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#aggregate-constructors
//META: doc_section_digest=81cb7db040259e3ef92a1ca250fb9ea0adfaa31cb83daa0c083596c98a915a16
//META: purpose=A `floatRxC(...)` matrix-constructor call with non-constant component values lowers to `makeMatrix(...)` producing a `Mat(Float, R : Int, C : Int, L : Int)` value.
//META: intent=functional
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The Aggregate constructors table lists `makeMatrix` for
// "matrix-constructor calls". A `float3x3(...)` constructor with
// nine uniform float operands produces the opcode.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float3x3> mbuf;

[numthreads(1, 1, 1)]
void main()
{
    mbuf[0] = float3x3(fa, fa, fa, fa, fa, fa, fa, fa, fa);
}

// IR-LABEL: func %main
// IR: makeMatrix(%fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa, %fa)
