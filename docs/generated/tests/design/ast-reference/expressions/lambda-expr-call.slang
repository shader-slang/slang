//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=521b006586b3c2d016313bdd2e59350f44b59690f83752ed2aacfb0e0f8e9d5c
//META: purpose=A LambdaExpr `(params) => { body }` forms a callable that returns the body's value when invoked.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says a LambdaExpr parses into a synthesized LambdaDecl and
// a body Stmt. We define a lambda, store it in a `var`, and call it;
// the returned value must match the body's expression.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var add = (int x, int y) => x + y;
    // CHECK: r=12
    printf("r=%d\n", add(5, 7));
}
