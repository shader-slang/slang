//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/values.md#intval-family
//META: doc_section_digest=7dc220c4c4dd5c2b21296a00da8c0b8bb7caf5c69af13f6b032f0ae0e4047ecd
//META: purpose=An error placeholder integer lets checking continue when a compile-time integer cannot be computed, so an undeclared name in an array bound produces only the root-cause diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the recovery role of the error-placeholder integer value.
// The array bound names an identifier that does not exist, so no
// integer can be computed; CHECK pins the single undefined-identifier
// diagnostic (E30015) at the bound. No follow-on "array size must be a
// constant" style error is annotated, which is what makes this a
// cascade-suppression observation rather than a plain name-lookup one.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int a[undeclaredN];
//CHECK:  ^^^^^^^^^^^ undefined identifier 'undeclaredN'.
// CHECK: E30015
}
