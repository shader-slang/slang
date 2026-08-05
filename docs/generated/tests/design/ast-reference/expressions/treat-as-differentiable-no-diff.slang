//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=b4aac875016d352a73eaff212c9886fb3c6956320e3406200a947b5679edef57
//META: purpose=Verifies that `no_diff(call)` marks the wrapped call non-differentiable and is accepted inside a [Differentiable] function.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The differentiability marker wraps a call rather than an arbitrary
// expression, and the wrapped call still contributes its primal value. A
// [Differentiable] function adds an ordinary (non-differentiable) helper call
// under `no_diff`; the CHECK asserts the combined primal result, which is only
// produced if the marker was accepted and the wrapped call ran.

//TEST:INTERPRET(filecheck=CHECK):

float helper(float x) { return x + 1.0; }

[Differentiable]
float f(float x)
{
    return x * x + no_diff(helper(x));
}

void main()
{
    // CHECK: v=13
    printf("v=%d\n", int(f(3.0)));
}
