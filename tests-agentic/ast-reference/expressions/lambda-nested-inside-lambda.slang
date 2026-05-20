//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=521b006586b3c2d016313bdd2e59350f44b59690f83752ed2aacfb0e0f8e9d5c
//META: purpose=Stresses LambdaExpr nesting: a curried `(int x) => (int y) => x + y` parses, captures `x` into the inner lambda, and yields the documented sum.
//META: intent=stress
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two LambdaExpr nodes nested. The inner lambda captures the outer
// lambda's parameter `x`. Currying `add(5)(7)` must yield 12.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var add = (int x) => (int y) => x + y;
    var addToFive = add(5);
    // CHECK: r=12
    printf("r=%d\n", addToFive(7));
}
