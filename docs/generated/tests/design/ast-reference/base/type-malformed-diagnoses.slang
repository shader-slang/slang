//META: generated=true
//META: model=claude-opus-4-8
//META: generated_at=2026-07-02T00:00:00+00:00
//META: source_commit=8e2a63cbf22c85332796702fd02a875466486a23
//META: doc_ref=docs/generated/design/ast-reference/base.md#type-val
//META: doc_section_digest=617a194ed6ec24148aa785801ab591efc2b4d803a1ca1e6680b6f00d76499121
//META: purpose=Every type the front end works with is a Type-as-Val; an ill-formed type expression at a declaration site cannot resolve to any Type and is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an unresolved type identifier in a parameter slot
// (where a Type-as-Val is required) is reported. The name
// `NotARealTypeName` is not a Type, so no Val can be built and the
// checker emits an "undefined identifier" diagnostic at the type
// expression's source location.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f(NotARealTypeName x) {}
/*CHECK:
       ^^^^^^^^^^^^^^^^ undefined identifier
       ^^^^^^^^^^^^^^^^ undefined identifier 'NotARealTypeName'.
*/
// CHECK: E30015

void main() {}
