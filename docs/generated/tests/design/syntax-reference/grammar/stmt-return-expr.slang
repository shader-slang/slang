//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#statements
//META: doc_section_digest=c4108d6661545bac260ff71845e7ba2fe35ca7f7587b33e25c895a81925cb89d
//META: purpose=`ReturnStmt` parses as `'return' Expr? ';'` and yields the expression value.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the ReturnStmt production with an Expr operand.

//TEST:INTERPRET(filecheck=CHECK):

int answer() { return 42; }

void main()
{
    // CHECK: r=42
    printf("r=%d\n", answer());
}
