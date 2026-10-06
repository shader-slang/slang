//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=e6dd79fac380ce187829abe90002109d7b13f470e191affc4c82a5813eb6298b
//META: purpose=An unsatisfied non-empty-pack constraint (where nonempty(T)) is reported as generic-constraint-not-satisfied naming the constraint.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the non-empty-pack spelling of the constraint-failure arm. The call
// supplies no arguments, so the inferred pack is empty and `nonempty(T)` fails
// during inference; CHECK pins the constraint message with the constraint text.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int cnt<each T>(expand each T args) where nonempty(T) { return 1; }

void main()
{
    int tag = cnt();
//CHECK:         ^ could not satisfy the generic constraint 'nonempty(T)'
    printf("tag=%d\n", tag);
}
