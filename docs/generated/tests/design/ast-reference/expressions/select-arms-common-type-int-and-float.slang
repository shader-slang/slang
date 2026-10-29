//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=0b0dfab9f13850a71e13b9ce6f8fb9a433ddb72267a526bf6454583638151a5b
//META: purpose=Verifies that a SelectExpr with arms of different but convertible types (`int` and `float`) is accepted and yields the chosen arm in the common type (float).
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary on SelectExpr arm-type matching: the two arms have
// distinct types, but a common type emerges via implicit cast. The
// false branch's 2.5f appears as 2.5 when the assignment is float.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool c = false;
    float v = c ? 1 : 2.5f;
    // CHECK: v=2.5
    printf("v=%.1f\n", v);
}
