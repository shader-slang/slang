//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#differentiate-family-expressions
//META: doc_section_digest=d977436f6dd59b4e5e73cb194cb787b23a56ac21de6d48a23ddc96e540a36a6e
//META: purpose=A ForwardDifferentiateExpr `__fwd_diff(f)` selects the forward-mode derivative of a [Differentiable] function.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `__fwd_diff(f)` selects the forward-mode derivative of a
// [Differentiable] function. f(x) = x*x evaluated at x = 3 with dx = 1 must
// yield the primal 9 and the differential 6; the CHECKs read both halves of
// the returned pair.

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
