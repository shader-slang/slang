//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=The doc example `a + b * c == d && e << f` groups as `((a+(b*c))==d) && (e<<f)` — verifying multiple precedence levels at once.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies four precedence levels interact correctly in one expression:
// `*` (level 3) > `+` (level 4) > `==` (level 7) > `&&` (level 11) > `<<` (level 5).
// With a=2, b=3, c=4, d=14, e=1, f=2 the grouping `((a+(b*c))==d) && (e<<f)`
// yields 1; changing d to 13 breaks the equality and yields 0, ruling out
// alternative groupings.

//TEST:INTERPRET(filecheck=CHECK):

int complex_expr(int a, int b, int c, int d, int e, int f)
{
    return (a + b * c == d && e << f) ? 1 : 0;
}

void main()
{
    // ((2 + 3*4) == 14) && (1 << 2) = (14==14) && 4 = true && true = 1
    int r1 = complex_expr(2, 3, 4, 14, 1, 2);
    // ((2 + 3*4) == 13) && (1 << 2) = (14==13) && 4 = false && true = 0
    int r2 = complex_expr(2, 3, 4, 13, 1, 2);
    // CHECK-DAG: complex-doc-example r1=1
    // CHECK-DAG: complex-doc-example r2=0
    printf("complex-doc-example r1=%d\n", r1);
    printf("complex-doc-example r2=%d\n", r2);
}
