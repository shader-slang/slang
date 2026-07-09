//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=04ce765c6615b6d4afcb57668ffc53ea9c74c835ea05c4c4d58b93fe0cbf4541
//META: purpose=Boundary: `==` against the negative-zero literal `-0.0` lowers to `cmpEQ(%fa, -0 : Float)`, with the sign-distinct payload preserved on the operand.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The cmpEQ payload for `-0.0` is distinct from `0.0` in the IR
// formatter (matches the IEEE-754 distinct bit pattern), even
// though `+0.0 == -0.0` is true at the value level.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(fa == -0.0, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpEQ(%fa, -0 : Float)
