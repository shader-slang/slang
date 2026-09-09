//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#type-modifiers-wrapping-the-type-rather-than-the-declaration
//META: doc_section_digest=3ce0d13cb0cdf095e32648137a27df629ce5030246d3c0ecfe8279e160be8599
//META: purpose=The `no_diff` type modifier suppresses differentiation through a parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `no_diff` attaches to the parameter's type and removes it from the
// differentiable set. `scaledSum(x, scale)` is differentiated with respect to `x`
// only; the second argument is passed as a plain value, which is legal only
// because `no_diff` took it out of the derivative signature. CHECK asserts the
// derivative is 5 (the scale), the value that d/dx of `x * scale` must produce.

//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float scaledSum(float x, no_diff float scale)
{
    return x * scale;
}

void main()
{
    DifferentialPair<float> px = DifferentialPair<float>(2.0, 1.0);
    DifferentialPair<float> r = fwd_diff(scaledSum)(px, 5.0);
    //CHECK: deriv=5
    printf("deriv=%d\n", int(r.d));
}
