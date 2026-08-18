//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5f27d4bb5fbd1429627db68bec594389689c236998e7182eaaf350ae3ceda2ee
//META: purpose=Boundary: positive `0.0` literal survives as `0 : Float` on a `mul` operand (the `add` identity collapses `+0`, so `mul` is used to keep the literal observable).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Float zero — the documented "smallest" magnitude. `mul` is used
// because `add(%fa, 0.0)` collapses to `%fa` via the additive
// identity before the IR dump captures it; `mul(%fa, 0.0)` does
// not, so the literal payload remains observable.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = fa * 0.0;
}

// IR-LABEL: func %main
// IR: mul(%fa, 0 : Float)
