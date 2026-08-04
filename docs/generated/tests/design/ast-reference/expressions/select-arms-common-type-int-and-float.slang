//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/generated/design/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
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
