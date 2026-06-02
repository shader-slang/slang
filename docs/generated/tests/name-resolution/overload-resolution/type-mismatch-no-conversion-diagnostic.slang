//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:48:41Z
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=c71eed362b7090c94c5447396e3ebd7ff5d3d15480a793a8f05cdaa4015e935b
//META: purpose=When TryCheckOverloadCandidateTypes cannot find any implicit conversion for an argument (canConvertImplicitly rejects), the only candidate is dropped and the user sees a no-applicable-overload diagnostic at the call site.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// "When canConvertImplicitly rejects an argument's cost, the
// candidate is dropped (silently in JustTrying, with a conversion-
// error diagnostic in ForReal)." Passing a non-convertible struct
// to an int parameter triggers this.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct Custom { int v; }

void receive(int x) {}

void main()
{
    Custom c;
    receive(c);
//CHECK:    ^ type mismatch in expression
// CHECK: E30019
}
