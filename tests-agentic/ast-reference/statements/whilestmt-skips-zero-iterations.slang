//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
//META: purpose=A WhileStmt tests its predicate before each iteration; when the predicate is false at entry the body does not execute at all.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The loop-family section says WhileStmt is the head-tested loop
// (`while (cond) body`). Observable consequence: a false initial
// condition skips the body entirely. If the parser mis-bound `while`
// to `do-while` semantics the body would run once and the counter
// would be 1 instead of 0.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int count = 0;
    while (count > 0)
    {
        count = count - 1;
    }
    // CHECK: count=0
    printf("count=%d\n", count);
}
