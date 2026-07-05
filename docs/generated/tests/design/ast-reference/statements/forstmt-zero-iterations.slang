//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
//META: purpose=A ForStmt whose predicateExpression is false at entry executes zero iterations; the loop body does not run at all.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: lower-edge iteration count for ForStmt. The doc says
// ForStmt has `initialStatement; predicateExpression; sideEffectExpression`.
// A predicate that is false at entry means zero body executions.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int hits = 0;
    for (int i = 0; i < 0; i = i + 1)
    {
        hits = hits + 1;
    }
    // CHECK: hits=0
    printf("hits=%d\n", hits);
}
