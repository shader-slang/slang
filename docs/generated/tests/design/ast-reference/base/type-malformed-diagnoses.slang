//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/base.md#type-val
//META: doc_section_digest=617a194ed6ec24148aa785801ab591efc2b4d803a1ca1e6680b6f00d76499121
//META: purpose=A type expression must resolve to a type value before a declaration can use it, so an unresolvable spelling in a parameter's type slot is diagnosed.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the negative side of the type-as-value claim: the name in a
// parameter's type position resolves to nothing, so no type value can
// be built for the parameter and the declaration is rejected. CHECK
// pins E30015 with the caret under the type spelling itself.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void f(NotARealTypeName x) {}
/*CHECK:
       ^^^^^^^^^^^^^^^^ undefined identifier
       ^^^^^^^^^^^^^^^^ undefined identifier 'NotARealTypeName'.
*/
// CHECK: E30015

void main() {}
