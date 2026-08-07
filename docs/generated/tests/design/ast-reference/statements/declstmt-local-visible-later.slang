//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#declstmt-expressionstmt-and-emptystmt
//META: doc_section_digest=8cbc53779444646ce64a0d14a5a1f170a96bcb2eec03025e1840f339a2581e40
//META: purpose=A DeclStmt is a `DeclBase` used in statement position; the canonical use is a local variable declaration whose name is visible to subsequent statements in the same block.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says DeclStmt "lets any DeclBase appear in a statement
// position (the canonical use is a local-variable declaration)". The
// observable surface is that a `int x = ...;` placed mid-block is in
// scope for the rest of that block and its value is readable.

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
