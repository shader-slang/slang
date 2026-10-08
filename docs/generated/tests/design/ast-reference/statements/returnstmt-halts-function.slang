//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/ast-reference/statements.md#returnstmt
//META: doc_section_digest=c25c0b38fe3f374b6c53bf5d39f45dbef8f00f160edf334a7927f62cea89fd70
//META: purpose=A ReturnStmt yields a value and halts the function; statements after a taken return are not executed.
//META: intent=functional
//META: pipeline_stage=check
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// ReturnStmt is documented as `return` (optionally with an expression).
// The expression carries the value; the halting is the other half of
// the role. We test both halves by writing a function that would have
// the wrong return value if statements after `return` were observed.

//TEST:INTERPRET(filecheck=CHECK):

int firstPositive(int a, int b, int c)
{
    if (a > 0) { return a; }
    if (b > 0) { return b; }
    return c;
}

void main()
{
    // First positive seen halts function: 5, not 9 or -3
    // CHECK: r=5
    printf("r=%d\n", firstPositive(5, 9, -3));
    // CHECK: s=9
    printf("s=%d\n", firstPositive(-1, 9, 100));
    // CHECK: t=-3
    printf("t=%d\n", firstPositive(-1, -2, -3));
}
