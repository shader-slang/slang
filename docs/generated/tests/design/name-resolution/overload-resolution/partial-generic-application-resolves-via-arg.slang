//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#partial-generic-application
//META: doc_section_digest=b26e753d2e537fba4eb99322e5bdf496b8dbb6792283415ba45b24c59a14fc6e
//META: purpose=A generic candidate's TryCheckGenericOverloadCandidateTypes infers the type parameter from the argument's type when the call does not supply explicit generic arguments; the resolved call is callable and returns the typed result.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "When the call expression supplies arguments that pin down some
// but not all of a generic's parameters, TryCheckGenericOverloadCandidate
// Types may produce a candidate that is partially specialized."
// Here T is inferred from the int argument so the call resolves to
// `identity<int>` without any explicit generic application.

//TEST:INTERPRET(filecheck=CHECK):

T identity<T>(T x) { return x; }

void main()
{
    int v = identity(42);
    // CHECK: v=42
    printf("v=%d\n", v);
}
