//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Parenthesized subexpressions override precedence (level 0, highest): `(2+3)*4` = 20 rather than the natural `2+(3*4)` = 14.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc's "Explicit subexpression grouping" section says:
// `(a + b) * c  // parenthesized subexpressions have highest precedence`
// Level 0 is atoms including parenthesized expressions — the highest precedence.
// Without parens: `2 + 3 * 4 = 14`.
// With parens:    `(2 + 3) * 4 = 20`.

//TEST:INTERPRET(filecheck=CHECK):

int parens_mul(int a, int b, int c) { return (a + b) * c; }
int no_parens(int a, int b, int c) { return a + b * c; }

void main()
{
    int with_parens = parens_mul(2, 3, 4);
    int without_parens = no_parens(2, 3, 4);
    // CHECK-DAG: parens: 20
    // CHECK-DAG: no-parens: 14
    printf("parens: %d\n", with_parens);
    printf("no-parens: %d\n", without_parens);
}
