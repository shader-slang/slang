//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/ast-reference/base.md#type-val
//META: doc_section_digest=15f58cdd668c51a527f2f464743928aa539eb7a9756515887b7ec8afc26e6407
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

void main() {}
