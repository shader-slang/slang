//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=b2029736d63943de091f689a9a0ed7f6ade82400bc257a00a34b24d27b223f09
//META: purpose=`private` on a top-level decl (no enclosing aggregate or namespace) is rejected with `invalid-use-of-private-visibility`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Edge case: `private` only makes sense inside a container. A
// top-level struct declared `private` fires diagnostic 30603.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

private struct TopLevel
/*CHECK:
^^^^^^^ invalid private visibility
^^^^^^^ 'TopLevel' cannot have private visibility because it is not a member of a type.
*/
// CHECK: E30603
{}
