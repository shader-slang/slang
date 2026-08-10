//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=f194f1b075af80ee23e030c52a1aedd7ee43bc2072f0ea3308cbffd439f98c56
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
