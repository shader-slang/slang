//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T18:00:00+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=942497f445c2102c7a435609ccb26e1bc901622765b619863b12aa14a39e7af0
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
