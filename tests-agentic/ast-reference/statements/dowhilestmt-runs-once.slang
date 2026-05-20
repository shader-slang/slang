//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
//META: purpose=A DoWhileStmt tests its predicate after each iteration; the body executes at least once even when the predicate is false at entry.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The loop-family section names DoWhileStmt as the `do body while
// (cond);` tail-tested form. The distinguishing observable from
// WhileStmt is that the body runs at least once even when the
// predicate is false at entry. A `while` mis-bound to `do-while` would
// also pass this test, but the partner whilestmt test rules that out.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 0;
    do
    {
        n = n + 1;
    } while (n < 0);
    // CHECK: n=1
    printf("n=%d\n", n);
}
