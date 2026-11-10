//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/name-resolution/visibility.md#edge-cases-and-failure-modes
//META: doc_section_digest=c8df5707a8529f8b54eb08117b70cc5d32a632cfac0bf45bd758aba4031e885f
//META: purpose=Marking a namespace `internal` is rejected with `invalid-visibility-modifier-on-type-of-decl` (36005); namespaces do not accept visibility modifiers.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `internal` on a namespace is an unsupported modifier
// placement: a NamespaceDecl is unconditionally Public, so the
// visibility keyword is rejected at the declaration site. The CHECK
// pins E36005 with carets at the modifier, covering the keyword axis
// the doc names explicitly.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

internal namespace Grouped
/*CHECK:
^^^^^^^^ visibility modifier not allowed
^^^^^^^^ visibility modifier is not allowed on 'namespace'.
*/
// CHECK: E36005
{
    int member() { return 2; }
}
