//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#statements
//META: doc_section_digest=53b6c93f0af2e9c3190efabb8af92d9e31b25ecd69db4a1c48dd5dfbd607aaa8
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
