//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T00:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
//META: purpose=A WhileStmt with a constant-true predicate iterates indefinitely; a body-internal BreakStmt terminates the LoopStmt at the documented iteration.
//META: intent=boundary
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary: WhileStmt with a constant-true predicate. The doc says
// WhileStmt is head-tested with `predicate: Expr*`; a literal `true`
// must still re-check before each iteration but never falsify. A
// break inside the body must short-circuit the loop.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int n = 0;
    while (true)
    {
        n = n + 1;
        if (n == 4) { break; }
    }
    // CHECK: n=4
    printf("n=%d\n", n);
}
