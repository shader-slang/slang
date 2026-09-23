//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=90d8366fb34b230a0bf9d002a41000fb52e4433f1d99750051158a6784953ce9
//META: purpose=TryCheckOverloadCandidateClassNewMatchUp rejects a plain constructor call whose candidate constructs a class, which must be initialized with new.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the mirror arm of the new/class pairing step: the candidate is a
// class constructor but the surface form is a plain call. The mismatch is only
// detected in the finalize re-check; CHECK pins the resulting diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

class C { int v; __init() { v = 7; } }

void make()
{
    C c = C();
//CHECK:   ^ a class can only be initialized by a `new` clause
    printf("%d\n", c.v);
}

void main() { make(); }
