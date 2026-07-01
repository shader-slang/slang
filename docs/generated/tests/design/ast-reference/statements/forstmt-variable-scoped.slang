//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#loop-family
//META: doc_section_digest=27580c18b5b0d2c47692d85449b6144b11636a004d88adeefbe87418ecc0d12e
//META: purpose=A ForStmt is a ScopeStmt; the loop variable introduced by `initialStatement` is scoped to the body and does not leak into the surrounding scope.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The loop-family section says "ForStmt::statement is the body;
// initialStatement is parsed as a statement (not an expression) so
// that a DeclStmt can introduce loop variables", and that ForStmt is
// scoped (unlike its UnscopedForStmt sibling). We verify scoping by
// declaring a same-named outer variable and observing that it is
// preserved across the loop: the loop's `int i` shadows it but does
// not modify it.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int i = 100;
    int sum = 0;
    for (int i = 0; i < 4; i = i + 1)
    {
        sum = sum + i;
    }
    // CHECK: i=100
    printf("i=%d\n", i);
    // 0+1+2+3 = 6
    // CHECK: sum=6
    printf("sum=%d\n", sum);
}
