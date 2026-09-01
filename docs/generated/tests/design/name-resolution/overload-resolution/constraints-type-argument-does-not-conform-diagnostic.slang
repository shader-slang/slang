//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=6e6bd1f54cf8c6c1c7ea9efd59c3c27ea43dc0195aebd524dc7f1c903ca95457
//META: purpose=TryCheckOverloadCandidateConstraints rejects a generic candidate whose inferred type argument does not satisfy the declared interface conformance.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the constraint step. `describe` requires T : IShape; the call
// supplies an `int`, which has no IShape conformance, so the constraint check
// fails and the ForReal re-check reports it. CHECK pins the
// does-not-conform-to-interface error at the argument.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IShape { int kind(); }

int describe<T : IShape>(T s) { return s.kind(); }

void main()
{
    int r = describe(5);
//CHECK:            ^ type argument 'int' does not conform to the required interface 'IShape'
    printf("%d\n", r);
}
