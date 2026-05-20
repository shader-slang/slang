//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=300bbe3c39d84a3dd7997f5a048fb4bd6e896b95057fa5fb8b7116c32840a6b7
//META: purpose=Boundary: float divide by positive-zero literal (`fa / 0.0`) lowers to `div(%a, 0 : Float)` — the `div` opcode survives even when the divisor is the IEEE-754 +zero, where integer divide-by-zero is rejected as a diagnostic.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc lists `div` for `/` and notes the choice between
// signed/unsigned/floating-point is keyed by operand types. On
// floats, divide-by-zero is allowed (produces +/-inf or NaN per
// IEEE-754) and the IR retains the `div` opcode with the zero
// payload directly on its operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = fa / 0.0;
}

// IR-LABEL: func %main
// IR: div(%fa, 0 : Float)
