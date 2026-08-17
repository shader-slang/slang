//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#varexpr-and-declrefexpr
//META: doc_section_digest=d41eb7e3da9d1c66fee3952dce01afeadec7647fa5526911668777fd6602e274
//META: purpose=A VarExpr whose name lookup finds nothing is rejected with an "undefined identifier" diagnostic.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// VarExpr starts life carrying only `name` and `scope`; the checker
// fills in `declRef` by lookup. When lookup finds no decl, the
// checker emits an "undefined identifier" diagnostic at the use
// site.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = undeclaredName;
//CHECK:    ^^^^^^^^^^^^^^ undefined identifier
//CHECK:    ^^^^^^^^^^^^^^ undefined identifier 'undeclaredName'.
// CHECK: E30015
}
