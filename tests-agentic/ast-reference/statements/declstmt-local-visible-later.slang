//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#declstmt-and-expressionstmt
//META: doc_section_digest=5421163f30d37859ccbac142213ea7e98961f1ad0e8a11d41b9da2055e015242
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
