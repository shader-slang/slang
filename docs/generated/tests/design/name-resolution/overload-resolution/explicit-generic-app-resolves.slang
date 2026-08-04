//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=f9c7becda59ea2cafbc68bded1f9fb4c90f4bada02e2725c3fe5655439f0c486
//META: purpose=An explicit generic application G<A> binds the generic's type parameter directly, satisfying TryCheckOverloadCandidateConstraints; the call resolves and returns the expected value.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "TryCheckOverloadCandidateConstraints ... for an explicit generic
// application G<A, B>, this step is where the where-clauses on G's
// parameters are validated against the inferred substitutions."
// With an explicit `<int>` and no where-clauses, the generic
// candidate survives the pipeline and the call returns a known
// value.

//TEST:INTERPRET(filecheck=CHECK):

int makeZeroLike<T>(T x) { return 0; }

void main()
{
    int v = makeZeroLike<int>(42);
    // CHECK: v=0
    printf("v=%d\n", v);
}
