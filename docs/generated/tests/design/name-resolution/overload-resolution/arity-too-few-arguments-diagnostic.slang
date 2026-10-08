//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=8893ade9a7efa04929eeabd0929b6ce5ed75204189b492f7d284d2f366d938f2
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
