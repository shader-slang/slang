//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#differentiability-attributes
//META: doc_section_digest=6699d8238c4128825e36dbe2a01de6dafc1b30965367cc9fdfea25f482ef206a
//META: purpose=A `[Differentiable]` function can be passed to `fwd_diff` and produces the correct derivative.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the marker attribute is what admits a function into the autodiff
// machinery. `square` is transformed by `fwd_diff` and evaluated at x=3 with
// dx=1; CHECK asserts the derivative component is 6, i.e. 2x -- the value only a
// real forward-mode transform of the body can produce.

//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float square(float x)
{
    return x * x;
}

void main()
{
    DifferentialPair<float> p = DifferentialPair<float>(3.0, 1.0);
    DifferentialPair<float> r = fwd_diff(square)(p);
    //CHECK: deriv=6
    printf("deriv=%d\n", int(r.d));
}
