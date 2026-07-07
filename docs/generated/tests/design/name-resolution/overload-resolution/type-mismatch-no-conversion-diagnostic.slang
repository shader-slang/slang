//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#probe-phase-trycheckoverloadcandidate
//META: doc_section_digest=f9c7becda59ea2cafbc68bded1f9fb4c90f4bada02e2725c3fe5655439f0c486
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
