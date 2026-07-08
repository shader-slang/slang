//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:40Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=f0fb19c83c1fe744cfc27441df1ce1770502bc22a362ebfb8484f2adaed2edb9
//META: purpose=Marking a namespace `private` is rejected with `invalid-visibility-modifier-on-type-of-decl` (36005); namespaces do not accept visibility modifiers.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a namespace cannot carry a visibility keyword: namespaces are
// unconditionally Public, so `private namespace ...` is an unsupported
// node kind for the modifier. The CHECK pins diagnostic code E36005 at
// the namespace declaration, proving the rejection happens at the
// declaration site rather than silently classifying the namespace.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

private namespace Hidden
// CHECK: E36005
{
    int member() { return 1; }
}
