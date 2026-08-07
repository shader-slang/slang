//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/overload-resolution.md#finalize-phase-completeoverloadcandidate
//META: doc_section_digest=a916857bbe07c220fc4b183b2a1794d39a799be9e41fa992dc6a49d37be94635
//META: purpose=TryCheckOverloadCandidateClassNewMatchUp runs only in the finalize re-check and rejects a new expression whose candidate does not construct a class.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the new/class pairing step. The struct's constructor is a perfectly
// applicable candidate, so it wins the probe phase; only the ForReal re-check
// notices the surface form is `new` while the constructed type is a struct.
// CHECK pins that rejection.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

struct S { int v; }

void make()
{
    S s = new S();
//CHECK:  ^^^ `new` can only be used to initialize a class
    printf("%d\n", s.v);
}

void main() { make(); }
