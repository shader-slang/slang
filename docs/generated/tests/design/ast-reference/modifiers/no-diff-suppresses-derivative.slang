//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T16:50:00+00:00
//META: source_commit=bbd84dc65e58598bfa71fafe72764b4076b0869b
//META: doc_ref=docs/generated/design/ast-reference/modifiers.md#type-modifiers-wrapping-the-type-rather-than-the-declaration
//META: doc_section_digest=eb1bd482f15f6231d9a0ec5d997ee4c0c0a88b66dd6fa7e77a1c1b4da32d9254
//META: purpose=`no_diff` type modifier suppresses differentiation through a parameter.
//META: intent=functional
//META: pipeline_stage=ir-pass
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `NoDiffModifier` is the type modifier behind the `no_diff` keyword.
// A parameter declared as `no_diff T` does not participate in the
// derivative; only the parameter without `no_diff` carries through.

//TEST:INTERPRET(filecheck=CHECK):

[Differentiable]
float scaledSum(float x, no_diff float scale)
{
    return x * scale;
}

void main()
{
    DifferentialPair<float> px = DifferentialPair<float>(2.0, 1.0);
    // The `no_diff` parameter takes only the primal value.
    DifferentialPair<float> r = fwd_diff(scaledSum)(px, 5.0);
    // d/dx(x*scale)|x=2,scale=5 with dx=1 is 5.
    //CHECK: deriv=5
    printf("deriv=%d\n", int(r.d));
}
