//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:00:00+00:00
//META: source_commit=1e0d460c1cb410005c4f775ba11fbc803cc8c16d
//META: doc_ref=docs/llm-generated/ast-reference/statements.md#nodes
//META: doc_section_digest=9f32f2463c3b3150d88ac333b9a732766e6f477e8e541627f6f4db0faa939a24
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
