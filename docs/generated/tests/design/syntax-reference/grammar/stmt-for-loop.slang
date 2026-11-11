//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=b9a1edc11e7f4ef12c141b50f73b3a36a9eda5811402d0c118a09dc03161e42b
//META: purpose=`ForStmt` parses with `DeclStmt` init, `Expr` condition, and `Expr` increment.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ForStmt production: `'for' '(' DeclStmt Expr ';' Expr ')' Stmt`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int sum = 0;
    for (int i = 1; i <= 4; ++i) sum += i;
    // CHECK: sum=10
    printf("sum=%d\n", sum);
}
