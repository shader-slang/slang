//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-13T00:00:00+00:00
//META: source_commit=c0e5ca5c55ff5ea6b210ac9418bac04728cc45e0
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=488c8698e5c5c5edba49863bb42e29a1d761c4be06fab55035626918cc32cbb8
//META: purpose=The single-expression lambda form wraps its expression in a synthesized return statement, so it builds the same shape as the explicit block form and the two are interchangeable.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `=> expr` is shorthand rather than a second kind of lambda.
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
