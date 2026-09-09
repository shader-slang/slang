//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=0c9bef4794f61ac3fead35d16b0cd0603f6881658db01263a9dfdf11e63c7f46
//META: purpose=A ThrowStmt is only valid inside a function declared to throw the matching error type; an uncaught `throw` in a non-throwing function is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `throw` is only admissible where the enclosing function is
// declared to propagate an error type. A plain `void` function has no
// such declaration, so the throw is uncaught; the CHECK pins the
// E30115 diagnostic on the `throw` token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum MyError { Fail }

void f()
{
    throw MyError.Fail;
/*CHECK:
    ^^^^^ uncaught 'throw' in non-throwing function
    ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
*/
// CHECK: E30115
}
