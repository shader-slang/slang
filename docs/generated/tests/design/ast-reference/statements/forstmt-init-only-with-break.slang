//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
//META: purpose=A ForStmt with empty predicateExpression and empty sideEffectExpression (`for (init; ; )`) loops indefinitely; a body-internal break terminates it after the documented number of iterations.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: ForStmt with null `predicateExpression` and null
// `sideEffectExpression` slots, only the initialStatement is present.
// The documented role is `for (init; cond; step) body`; missing
// `cond` is treated as constant-true. A break inside the body must
// still exit the LoopStmt.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int hits = 0;
    for (int i = 0; ; )
    {
        hits = hits + 1;
        if (hits >= 3) { break; }
        i = i + 1;
    }
    // CHECK: hits=3
    printf("hits=%d\n", hits);
}
