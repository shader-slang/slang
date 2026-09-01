//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=6e6bd1f54cf8c6c1c7ea9efd59c3c27ea43dc0195aebd524dc7f1c903ca95457
//META: purpose=TryCheckOverloadCandidateVisibility drops a candidate that isDeclVisibleFromScope rejects, and the finalize re-check emits decl-is-not-visible.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the visibility filter step. `secret` is the only candidate and it is
// private to S, so the visibility step rejects it from a call site outside S.
// CHECK pins the not-accessible diagnostic on the member name.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S
{
    private int secret() { return 42; }
}

void main()
{
    S s;
    int v = s.secret();
//CHECK:      ^^^^^^ 'secret' is not accessible from the current context.
    printf("%d\n", v);
}
