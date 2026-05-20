//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T21:00:00+00:00
//META: source_commit=3250005059a2746ebc504a9d3f71ed112f1f2b94
//META: doc_ref=docs/llm-generated/ast-reference/expressions.md#nodes
//META: doc_section_digest=5cae24401ed06bd3567242934951f344da434f10804637bdd679d55ab6b5bb2b
//META: purpose=Verifies that the InfixExpr compound assignment `*=` updates the l-value in place, parallel to the documented `+=` case.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary axis for compound assignment: cover a second operator
// flavor (`*=`) beyond the smoke-test `+=`, to confirm the family
// is uniform.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 5;
    x *= 3;
    // CHECK: x=15
    printf("x=%d\n", x);
}
