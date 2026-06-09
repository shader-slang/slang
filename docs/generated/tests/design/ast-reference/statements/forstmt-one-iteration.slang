//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
//META: purpose=A ForStmt whose predicate is true exactly once executes the body exactly once and then exits; both initialStatement and sideEffectExpression run before the predicate fails on the second test.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: 1-iteration edge. The init runs once (i=5), the body
// runs once (hits=1), and then sideEffectExpression bumps i to 6,
// after which the predicate i<6 is false. A mis-lowering that
// dropped sideEffectExpression would run forever; one that swapped
// pre/post-test would run zero or two times.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int hits = 0;
    for (int i = 5; i < 6; i = i + 1)
    {
        hits = hits + 1;
    }
    // CHECK: hits=1
    printf("hits=%d\n", hits);
}
