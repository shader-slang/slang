//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=c4108d6661545bac260ff71845e7ba2fe35ca7f7587b33e25c895a81925cb89d
//META: purpose=`SwitchStmt` parses with `SwitchCase` arms — `'case' Expr ':'` and `'default' ':'`.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the SwitchStmt production and the SwitchCase variants.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 2;
    int y = 0;
    switch (x)
    {
    case 1: y = 100; break;
    case 2: y = 200; break;
    default: y = 999; break;
    }
    // CHECK: y=200
    printf("y=%d\n", y);
}
