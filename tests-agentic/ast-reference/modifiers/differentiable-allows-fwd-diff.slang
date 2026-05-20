//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/llm-generated/ast-reference/modifiers.md#differentiability-attributes
//META: doc_section_digest=4ba0842f116047916828c7e60c076affbd0939fd9e85537ac882355c33512aba
//META: purpose=A `[Differentiable]` function can be passed to `fwd_diff` and produces a sensible derivative.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `DifferentiableAttribute` marks a function as participating in the
// autodiff machinery. Calling `fwd_diff(f)` on it produces a pair
// (primal, derivative) according to the autodiff IR pipeline.

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
    // d/dx (x^2) at x=3 with dx=1 is 6.
    //CHECK: deriv=6
    printf("deriv=%d\n", int(r.d));
}
