//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=62d41f734409e853620e442a2938448e5d7be850d748cdbabc3e948d9882271a
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
