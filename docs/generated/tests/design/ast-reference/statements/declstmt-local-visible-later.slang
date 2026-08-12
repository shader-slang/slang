//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=e05101999cc6555f3e7214b2fb06744dbaf16788905b25807e20594acfdd10d6
//META: purpose=A DeclStmt is a `DeclBase` used in statement position; the canonical use is a local variable declaration whose name is visible to subsequent statements in the same block.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies a local variable declaration is legal in statement position
// and that its name is visible to the statements that follow it in the
// same block. The CHECK asserts a sum computed from the earlier
// declaration, which is only computable if the name resolved.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int seed = 5;
    int local = seed * 2 + 1;
    int sum = seed + local;
    // 5 + 11 = 16
    // CHECK: sum=16
    printf("sum=%d\n", sum);
}
