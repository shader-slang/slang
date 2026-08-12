//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=f6737af21fc0ce62bf35ca601aa6d8bc483fabcafaedad874ca80c1323a37207
//META: purpose=Boundary: smallest positive normal `float` (~1.17549e-38) survives lowering as `1.17549e-38 : Float` on an `add` operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-positive-normal edge of the single-precision range. The
// IR dump formats it in exponent notation.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = fa + 1.175494351e-38;
}

// IR-LABEL: func %main
// IR: add(%fa, 1.17549e-38 : Float)
