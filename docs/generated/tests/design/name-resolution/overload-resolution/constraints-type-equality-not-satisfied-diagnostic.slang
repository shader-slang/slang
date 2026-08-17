//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#edge-cases-and-failure-modes
//META: doc_section_digest=7aaa4b55b90bf9dc8832d907095aad278bba32f9533b97e07d8e09e58134c060
//META: purpose=An unsatisfied type-equality constraint (where T == int) is reported as generic-constraint-not-satisfied naming the constraint.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the equality spelling of the constraint-failure arm. Inference
// binds T to float, which cannot satisfy `T == int`, so the candidate ends as
// a constraint failure rather than an argument type mismatch; CHECK pins the
// constraint message with the constraint text.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int only<T>(T v) where T == int { return 1; }

void main()
{
    float f = 1.0f;
    int tag = only(f);
//CHECK:          ^ could not satisfy the generic constraint 'T == int'
    printf("tag=%d\n", tag);
}
