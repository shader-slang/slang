//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#differentiate-family-expressions
//META: doc_section_digest=6c3b730bf3c80deec0b25345c74e483d9dc34ef0c1d176c3675315ff972a5c70
//META: purpose=A BackwardDifferentiateExpr `__bwd_diff(f)` selects the backward-mode derivative; calling it with dL/dy propagates a gradient back into the input differential.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names BackwardDifferentiateExpr as the backward-mode
// selector. For f(x) = x*x at x=3 with dL/dy=1, the resulting
// dL/dx is 2*x*1 = 6, stored in the .d slot of the diffPair.

//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float square(float x) { return x * x; }

void main()
{
    var dp = diffPair(3.0f, 0.0f);
    __bwd_diff(square)(dp, 1.0f);
    // CHECK: d=6
    printf("d=%d\n", int(dp.d));
}
