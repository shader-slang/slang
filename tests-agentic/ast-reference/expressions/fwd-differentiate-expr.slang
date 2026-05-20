//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:30:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#differentiate-family-expressions
//META: doc_section_digest=6c3b730bf3c80deec0b25345c74e483d9dc34ef0c1d176c3675315ff972a5c70
//META: purpose=A ForwardDifferentiateExpr `__fwd_diff(f)` selects the forward-mode derivative of a [Differentiable] function.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc names ForwardDifferentiateExpr as a HigherOrderInvokeExpr
// that selects the forward-mode derivative of its callable argument.
// For f(x) = x*x, the forward-mode derivative at x=3 with dx=1 is
// (9, 6).

//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float square(float x) { return x * x; }

void main()
{
    DifferentialPair<float> r = __fwd_diff(square)(diffPair(3.0f, 1.0f));
    // CHECK: p=9
    printf("p=%d\n", int(r.p));
    // CHECK: d=6
    printf("d=%d\n", int(r.d));
}
