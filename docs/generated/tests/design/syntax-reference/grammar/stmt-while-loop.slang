//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=11f48bb43fb7900ef7dd3c2f4785da1cab7aaae9b90d2da44ecf5e8cf8e950b1
//META: purpose=`WhileStmt` parses as `'while' '(' Expr ')' Stmt`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the WhileStmt production.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 0;
    while (n < 3) ++n;
    // CHECK: n=3
    printf("n=%d\n", n);
}
