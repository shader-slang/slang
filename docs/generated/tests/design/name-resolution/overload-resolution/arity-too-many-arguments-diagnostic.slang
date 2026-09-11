//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=6e6bd1f54cf8c6c1c7ea9efd59c3c27ea43dc0195aebd524dc7f1c903ca95457
//META: purpose=TryCheckOverloadCandidateArity rejects a call above the allowed argument count and the finalize re-check emits too-many-arguments.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the upper edge of the arity step: a one-parameter callable invoked
// with three arguments, past `allowed` because there are no trailing defaults
// to pad with. CHECK pins the too-many-arguments message at the call site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int neg(int a) { return -a; }

void main()
{
    int v = neg(1, 2, 3);
//CHECK:       ^ too many arguments to call
    printf("%d\n", v);
}
