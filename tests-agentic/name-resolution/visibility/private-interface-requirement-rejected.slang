//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=942497f445c2102c7a435609ccb26e1bc901622765b619863b12aa14a39e7af0
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
