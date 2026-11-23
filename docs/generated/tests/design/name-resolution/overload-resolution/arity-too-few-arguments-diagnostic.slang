//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=471a235af8be78d2eb5c62c0f69c7e4037797818d1630eb02a08fcdaae148425
//META: purpose=TryCheckOverloadCandidateArity rejects a call below the required argument count and the finalize re-check emits not-enough-arguments.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the lower edge of the arity step: a two-parameter callable invoked
// with one argument. The single candidate is still kept as the least-bad one,
// so the ForReal re-check reports the arity failure; CHECK pins that message
// at the call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int add(int a, int b) { return a + b; }

void main()
{
    int v = add(7);
//CHECK:       ^ not enough arguments to call
    printf("%d\n", v);
}
