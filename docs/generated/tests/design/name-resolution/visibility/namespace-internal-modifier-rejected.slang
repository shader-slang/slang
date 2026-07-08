//META: generated=true
//META: model=claude-fable-5
//META: generated_at=2026-07-08T15:25:40Z
//META: source_commit=6b490bc8cb3cf11b356ddf70168c5609efa46341
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=f0fb19c83c1fe744cfc27441df1ce1770502bc22a362ebfb8484f2adaed2edb9
//META: purpose=Marking a namespace `internal` is rejected with `invalid-visibility-modifier-on-type-of-decl` (36005); namespaces do not accept visibility modifiers.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the second documented keyword shape of the same rejection:
// `internal` on a namespace is an unsupported placement just like
// `private`, since a NamespaceDecl is unconditionally Public. The CHECK
// pins E36005 at the declaration, covering the keyword axis the doc
// names explicitly (`private` or `internal`).

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

internal namespace Grouped
// CHECK: E36005
{
    int member() { return 2; }
}
