//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#differentiate-family-expressions
//META: doc_section_digest=d977436f6dd59b4e5e73cb194cb787b23a56ac21de6d48a23ddc96e540a36a6e
//META: purpose=A BackwardDifferentiateExpr `__bwd_diff(f)` selects the backward-mode derivative; calling it with dL/dy propagates a gradient back into the input differential.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__bwd_diff(f)` selects the backward-mode derivative of a
// [Differentiable] function. For f(x) = x*x the gradient is 2*x, so seeding
// dL/dy = 1 at x = 3 must leave 6 in the differential slot of the pair; the
// CHECK reads that slot back.

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
