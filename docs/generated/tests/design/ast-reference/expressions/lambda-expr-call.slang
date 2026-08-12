//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=44c1034c52fc65241dd638ce5a86a08822f7e7eef48bfb8b6c16e47820038dcf
//META: purpose=A LambdaExpr `(params) => { body }` forms a callable that returns the body's value when invoked.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `(params) => body` forms a callable value that returns the
// body's result when invoked. The lambda is bound to a local and then called
// with a concrete argument; the CHECK asserts the body's arithmetic ran.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var add = (int x, int y) => x + y;
    // CHECK: r=12
    printf("r=%d\n", add(5, 7));
}
