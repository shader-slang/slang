//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=88158e254118bbcaa14ae66079e66a43621c70534d5a6d5c30c9c48586b3aa21
//META: purpose=The single-expression lambda form wraps its expression in a synthesized return statement, so it shares the outer LambdaExpr shape with the explicit block form and the two behave identically when called.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `=> expr` is shorthand rather than a second kind of lambda (the
// two forms differ only in whether bodyStmt is the parsed block or a bare
// synthesized return).
// The two spellings are written side by side over the same parameter list and
// body expression and passed through the same IFunc-constrained helper; if the
// expression form built anything other than a body ending in a return of that
// expression, one of the two calls would give a different answer or fail to
// satisfy the constraint. CHECK asserts both produce 6.

//TEST:INTERPRET(filecheck=CHECK):

int callWith(IFunc<int, int> f, int x)
{
    return f(x);
}

void main()
{
    let blockForm = (int x) => { return x + 1; };
    let exprForm = (int x) => x + 1;
    printf("block=%d expr=%d\n", callWith(blockForm, 5), callWith(exprForm, 5));
}

//CHECK: block=6 expr=6
