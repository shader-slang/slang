//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=521b006586b3c2d016313bdd2e59350f44b59690f83752ed2aacfb0e0f8e9d5c
//META: purpose=Verifies that a LambdaExpr capturing a single enclosing local variable reads its value through the synthesized LambdaDecl environment struct.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on LambdaDecl's captured-environment count: one capture.
// The doc says LambdaDecl carries the captured environment as a
// struct; the lambda must read the captured value at call time.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int delta = 10;
    var addDelta = (int x) => x + delta;
    // CHECK: r=17
    printf("r=%d\n", addDelta(7));
}
