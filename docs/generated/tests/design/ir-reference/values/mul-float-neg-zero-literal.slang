//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=5e869d77d32b7f1e8abd286a5564168a9cb5d9a145cb0faea9dcc4c12e8e73e3
//META: purpose=Boundary: negative-zero `-0.0` literal survives as `-0 : Float` on a `mul` operand, distinct from positive `0` in the IR payload.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// IEEE-754 has two distinct zero bit patterns. The doc's "literal
// payload encoding" claim implies the dump differentiates them;
// the IR formatter prints `0` and `-0` separately for the two
// zero values on the operand of a `mul`.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<float> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = fa * -0.0;
}

// IR-LABEL: func %main
// IR: mul(%fa, -0 : Float)
