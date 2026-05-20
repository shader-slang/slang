//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
//META: doc_ref=docs/llm-generated/ir-reference/values.md#comparison
//META: doc_section_digest=03e312fcc9f534bb1522ca14baac23dce5576764a035ba6d311c03be3b39ed14
//META: purpose=Boundary: `fa != fa` on a `uniform float` surfaces `cmpNE(%fa, %fa)` — the NaN-detection idiom is preserved through IR lowering.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The self-compare-not-equal form is the IEEE-754 NaN predicate.
// The doc says `!=` lowers to `cmpNE`; both operands being the
// same SSA value does not collapse the instruction.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(fa != fa, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpNE(%fa, %fa)
