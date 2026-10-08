//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-4 `+` binds more tightly than level-5 `<<`: `2 + 3 << 1` groups as `(2+3) << 1` = 10, not `2 + (3<<1)` = 8.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `+`/`-` are level 4, `<<`/`>>` are level 5.
// Level 4 is lower numeric = higher precedence, so `+` binds before `<<`.
// `2 + 3 << 1` groups as `(2+3) << 1` = `5 << 1` = 10.
// Wrong grouping `2 + (3<<1)` = `2 + 6` = 8.

//TEST:INTERPRET(filecheck=CHECK):

int add_then_shift(int a, int b, int c) { return a + b << c; }

void main()
{
    int result = add_then_shift(2, 3, 1);
    // CHECK: add-over-shift: 10
    printf("add-over-shift: %d\n", result);
}
