//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=6e6bd1f54cf8c6c1c7ea9efd59c3c27ea43dc0195aebd524dc7f1c903ca95457
//META: purpose=A too-short explicit generic-argument list on a generic whose inner declaration is not a callable cannot be partially applied and reaches the generic-argument-list arity diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the carve-out that decides where a short explicit generic-argument
// list lands. The generic here wraps a struct rather than a callable, so no
// partial application is possible and the arity mismatch is reported against
// the generic's parameter list; CHECK pins that message with both counts. The
// declaration is indented so the reported caret column clears the CHECK prefix.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct Pair<T, U> { T a; U b; }

void main()
{
          Pair<int> p;
//CHECK:  ^^^^ wrong number of generic arguments
//CHECK:  ^^^^ generic 'Pair' expects 2 generic argument(s) but 1 were provided
    printf("tag=%d\n", 0);
}
