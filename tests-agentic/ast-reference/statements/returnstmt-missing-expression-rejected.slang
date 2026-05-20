//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
//META: purpose=A ReturnStmt in a non-void function is required to carry an expression; `return;` is rejected with `'return' should have an expression`.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc spells ReturnStmt as "`return` (optionally with an
// expression)". The optionality is conditional on the function's
// return type: `void` allows `return;` and a non-void function
// requires the expression. We test the non-void-function side.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

int f()
{
    return;
/*CHECK:
    ^^^^^^ 'return' should have an expression
    ^^^^^^ 'return' should have an expression.
*/
}
