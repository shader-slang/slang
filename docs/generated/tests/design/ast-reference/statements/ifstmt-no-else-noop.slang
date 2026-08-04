//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/generated/design/ast-reference/statements.md#ifstmt
//META: doc_section_digest=10062280ed5cf8f826912bbfaa3a2c63188ff4d4c316ffda45574aab2cc4dc2c
//META: purpose=An IfStmt's negativeStatement slot is null for an `if` without an `else`; when the predicate is false the program continues with no side effects from the missing else branch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The IfStmt section says "the negativeStatement slot is null for an
// `if` without an `else`". Observable consequence: with a false
// predicate and no `else`, the variable keeps its prior value and the
// program does not crash; with a true predicate the positive arm
// fires.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 7;
    if (1 == 2) { x = 99; }
    // CHECK: x-after-false=7
    printf("x-after-false=%d\n", x);

    if (1 == 1) { x = 42; }
    // CHECK: x-after-true=42
    printf("x-after-true=%d\n", x);
}
