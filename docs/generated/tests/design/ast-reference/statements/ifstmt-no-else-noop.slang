//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#ifstmt
//META: doc_section_digest=fa628db8ade3560b33a46074f32ce4b163adb95e07f7892450f1b1f9c7ee55dc
//META: purpose=An IfStmt's negativeStatement slot is null for an `if` without an `else`; when the predicate is false the program continues with no side effects from the missing else branch.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies an `if` written without an `else` simply does nothing when
// the predicate is false — there is no alternative branch to run. The
// CHECKs assert the variable keeps its prior value after a false
// predicate and is updated after a true one.

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
