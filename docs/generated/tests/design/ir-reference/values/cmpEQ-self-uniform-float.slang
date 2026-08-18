//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ir-reference/values.md#comparison
//META: doc_section_digest=a41eec816a55db301ccafcfa867bab048ae6ed92cc99c5f791c2e8474302d15d
//META: purpose=Boundary: `fa == fa` on a `uniform float` is not constant-folded away — both operands are the same SSA value, so the comparison surfaces `cmpEQ(%fa, %fa)`. This is the canonical NaN-detection idiom (NaN != NaN per IEEE-754).
//META: intent=boundary
//META: pipeline_stage=lower
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The IEEE-754 NaN-ordering rule is `NaN != NaN`. The doc's
// Comparison table claims `==` lowers to `cmpEQ`; this test
// verifies that even the self-compare form (used as a NaN
// predicate at runtime) is not folded out before IR.

//TEST:SIMPLE(filecheck=IR):-target spirv-asm -dump-ir -o /dev/null -stage compute -entry main

uniform float fa;
uniform RWStructuredBuffer<int> buf;

[numthreads(1, 1, 1)]
void main()
{
    buf[0] = select(fa == fa, 1, 0);
}

// IR-LABEL: func %main
// IR: cmpEQ(%fa, %fa)
