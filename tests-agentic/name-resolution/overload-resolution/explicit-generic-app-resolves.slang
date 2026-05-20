//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=c71eed362b7090c94c5447396e3ebd7ff5d3d15480a793a8f05cdaa4015e935b
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
