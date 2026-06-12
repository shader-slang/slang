//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#partial-generic-application
//META: doc_section_digest=2653a18d403f024fba71e245d5bf2fc8cd9e7d1b5cc2b2a59cc47fac18f87e89
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
