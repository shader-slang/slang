//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#arithmetic-and-bitwise
//META: doc_section_digest=1d2f08dfecd3e2ccb26b50652f1294ebe4c1c5182a6aab140a589ccecd16faa1
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
