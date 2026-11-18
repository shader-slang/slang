//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=bf81ae64bcb3ebf34033f80837df03e474b90159b95904ac3be2fb4e350793c2
//META: purpose=Boundary: `==` against the float literal `0.0` lowers to `cmpEQ(%fa, 0 : Float)`, preserving the literal payload on the comparison operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Comparison against the positive-zero edge surfaces the `cmpEQ`
// opcode with the literal payload directly on the operand.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o - -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(fa == 0.0, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpEQ(%fa, 0 : Float)
