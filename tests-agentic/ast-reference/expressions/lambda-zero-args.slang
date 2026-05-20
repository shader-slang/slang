//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#lambdaexpr-and-lambdadecl
//META: doc_section_digest=521b006586b3c2d016313bdd2e59350f44b59690f83752ed2aacfb0e0f8e9d5c
//META: purpose=Verifies that a LambdaExpr with zero parameters — `() => 42` — forms a callable that returns the body's constant.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Lower-edge boundary on LambdaExpr parameter count. The lambda has
// no parameters; invocation must still return the body's value.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    var get42 = () => 42;
    // CHECK: r=42
    printf("r=%d\n", get42());
}
