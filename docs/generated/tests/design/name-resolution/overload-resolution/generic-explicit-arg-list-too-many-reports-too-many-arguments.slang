//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=6e6bd1f54cf8c6c1c7ea9efd59c3c27ea43dc0195aebd524dc7f1c903ca95457
//META: purpose=An explicit generic-argument list with too many arguments is rejected by the arity step with too-many-arguments rather than by the generic-argument-list arity diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the doc's worked example for which step catches an oversized
// explicit generic-argument list. The arity step leaves `allowed` at the
// declared generic parameter count, so three arguments to a two-parameter
// generic are rejected there; CHECK pins the ordinary arity message and its
// got/expected counts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

int g<T, U>(T a, U b) { return 1; }

void main()
{
    int tag = g<int, float, int>(1, 2.0f);
//CHECK:      ^ too many arguments to call (got 3, expected 2)
    printf("tag=%d\n", tag);
}
