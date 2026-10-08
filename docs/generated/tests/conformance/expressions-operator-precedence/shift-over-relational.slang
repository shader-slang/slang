//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-5 `<<` binds more tightly than level-6 `<`: `1 << 3 < 10` groups as `(1<<3) < 10` = true (8<10), not `1 << (3<10)` = 2.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `<<`/`>>` are level 5, `<`/`<=`/`>`/`>=` are level 6.
// Level 5 is lower numeric = higher precedence, so `<<` binds before `<`.
// `1 << 3 < 10` groups as `(1<<3) < 10` = `8 < 10` = true.
// Wrong grouping `1 << (3<10)` = `1 << 1` = 2 (integer, not bool).

//TEST:INTERPRET(filecheck=CHECK):

int shift_lt(int a, int b, int c) { return (a << b < c) ? 1 : 0; }

void main()
{
    int result = shift_lt(1, 3, 10);
    // CHECK: shift-over-relational: 1
    printf("shift-over-relational: %d\n", result);
}
