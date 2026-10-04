//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#returnstmt
//META: doc_section_digest=c25c0b38fe3f374b6c53bf5d39f45dbef8f00f160edf334a7927f62cea89fd70
//META: purpose=A ReturnStmt in a non-void function is required to carry an expression; `return;` is rejected with `'return' should have an expression`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the returned expression is matched against the enclosing
// function's declared return type, so a non-void function cannot use a
// bare `return;`. The CHECK pins the E30006 diagnostic on the `return`
// token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int f()
{
    return;
/*CHECK:
    ^^^^^^ 'return' should have an expression
    ^^^^^^ 'return' should have an expression.
*/
// CHECK: E30006
}
