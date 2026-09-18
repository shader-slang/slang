//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=b2029736d63943de091f689a9a0ed7f6ade82400bc257a00a34b24d27b223f09
//META: purpose=`private` on an interface requirement is rejected; the doc's "private inside the declaring container" rule does not extend to interface requirements which must be visible to implementers.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge case: an interface requirement cannot be `private` because
// every conforming implementer must be able to override it.
// The same `invalid-use-of-private-visibility` diagnostic fires.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

interface IThing
{
    private int hidden();
/*CHECK:
    ^^^^^^^ invalid private visibility
    ^^^^^^^ 'hidden' cannot have private visibility because it is not a member of a type.
*/
// CHECK: E30603
}
