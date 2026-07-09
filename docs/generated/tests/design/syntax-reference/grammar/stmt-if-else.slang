//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=c4108d6661545bac260ff71845e7ba2fe35ca7f7587b33e25c895a81925cb89d
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
