//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=488c8698e5c5c5edba49863bb42e29a1d761c4be06fab55035626918cc32cbb8
//META: purpose=Verifies that a LambdaExpr capturing a single enclosing local variable reads its value through the synthesized LambdaDecl environment struct.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a lambda body can read an enclosing local, i.e. that the
// capture is carried into the closure. The captured local is set before the
// lambda is formed and the lambda is invoked afterwards; the CHECK asserts the
// captured value reached the body.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int delta = 10;
    var addDelta = (int x) => x + delta;
    // CHECK: r=17
    printf("r=%d\n", addDelta(7));
}
