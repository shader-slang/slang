//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=f194f1b075af80ee23e030c52a1aedd7ee43bc2072f0ea3308cbffd439f98c56
//META: purpose=`IfStmt` parses as `'if' '(' Expr ')' Stmt ('else' Stmt)?`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the IfStmt production including the optional `else` arm.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 5;
    int y;
    if (x > 0)
        y = 1;
    else
        y = -1;
    // CHECK: y=1
    printf("y=%d\n", y);
}
