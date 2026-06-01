//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/generated/design/ir-reference/values.md#literal-payload-encoding
//META: doc_section_digest=04423d40739a5430fb339507cc0da277b4a8a29153c176db3ec3d56a42a4070b
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
