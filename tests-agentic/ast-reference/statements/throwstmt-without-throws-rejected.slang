//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#throwstmt-and-catchstmt
//META: doc_section_digest=4beb3cdf4f4fc264b184a0269fd084212ac38cb61fe979ed8cba8421dba3f523
//META: purpose=A ThrowStmt is only valid inside a function declared to throw the matching error type; an uncaught `throw` in a non-throwing function is rejected.
//META: intent=negative
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names ThrowStmt as "`throw e;` for errorable functions".
// The "errorable" qualifier means the enclosing function must declare
// `throws E`. A non-throwing function with a bare `throw` must be
// rejected so the type system stays sound.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

enum MyError { Fail }

void f()
{
    throw MyError.Fail;
/*CHECK:
    ^^^^^ uncaught 'throw' in non-throwing function
    ^^^^^ the current function or environment is not declared to throw any errors, but contains an uncaught 'throw' statement.
*/
}
