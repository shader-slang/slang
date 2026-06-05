//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#varexpr-and-declrefexpr
//META: doc_section_digest=1048119061754018682d979f5542202272f2f067f0a41b1b806037278d497506
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
