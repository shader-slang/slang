//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#differentiability-attributes
//META: doc_section_digest=6699d8238c4128825e36dbe2a01de6dafc1b30965367cc9fdfea25f482ef206a
//META: purpose=`[ForwardDerivative(fn)]` binds an explicit derivative function that `fwd_diff` calls instead of synthesising one.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the explicit-derivative form of the family actually redirects
// `fwd_diff` to the named function rather than merely marking `f` as
// differentiable. The user derivative deliberately returns 100 * dx, a value the
// synthesised derivative of `x * x` could never produce, so CHECK asserting 100
// proves the bound function was the one that ran.

//TEST:INTERPRET(filecheck=CHECK):

DifferentialPair<float> myDeriv(DifferentialPair<float> p)
{
    return diffPair(p.p * p.p, 100.0 * p.d);
}

[ForwardDerivative(myDeriv)]
float f(float x)
{
    return x * x;
}

void main()
{
    DifferentialPair<float> r = fwd_diff(f)(diffPair(3.0, 1.0));
    //CHECK: deriv=100
    printf("deriv=%d\n", int(r.d));
}
