//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=a41eec816a55db301ccafcfa867bab048ae6ed92cc99c5f791c2e8474302d15d
//META: purpose=Boundary: `fa != fa` on a `uniform float` surfaces `cmpNE(%fa, %fa)` — the NaN-detection idiom is preserved through IR lowering.
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The self-compare-not-equal form is the IEEE-754 NaN predicate.
// Verifies that `!=` lowers to `cmpNE`; both operands being the
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
