//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=b9a1edc11e7f4ef12c141b50f73b3a36a9eda5811402d0c118a09dc03161e42b
//META: purpose=`DoWhileStmt` parses as `'do' Stmt 'while' '(' Expr ')' ';'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the DoWhileStmt production with the body executing at least
// once before the condition is checked.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 5;
    int count = 0;
    do { ++count; --n; } while (n > 0);
    // CHECK: count=5
    printf("count=%d\n", count);
}
