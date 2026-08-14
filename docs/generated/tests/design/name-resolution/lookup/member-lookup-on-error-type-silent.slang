//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/lookup.md#edge-cases-and-failure-modes
//META: doc_section_digest=4c78ab08cf659f53d04e87356fa441615410f33a23152c70e9fb93b377efd998
//META: purpose=Member lookup on the error type matches no arm of the type dispatch and returns empty silently, so a member access on an unresolved name does not cascade a second diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the no-cascade property. The base of the member access is an
// undefined name, so its type is the error type and the following
// `.field` reaches member lookup on it. The test runs in exhaustive
// diagnostic mode and annotates only the undefined-identifier
// diagnostic; a follow-on member-not-found would leave an unmatched
// diagnostic and fail the run, which is what makes the silence
// observable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int n = noSuchVar.field;
//CHECK:    ^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^ undefined identifier 'noSuchVar'.
// CHECK: E30015
}
