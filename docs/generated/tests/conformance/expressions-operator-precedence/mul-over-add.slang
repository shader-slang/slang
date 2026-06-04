//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-3 `*` binds more tightly than level-4 `+`: `2 + 3 * 4` groups as `2 + (3 * 4)` = 14, not 20.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `*`/`/`/`%` are level 3 and `+`/`-` are level 4.
// Lower numeric level = higher precedence (tighter binding).
// So `*` should bind before `+`, grouping `2 + 3 * 4` as `2 + (3*4)` = 14.
// Wrong grouping `(2+3)*4` would yield 20 — the test distinguishes the two.

//TEST:INTERPRET(filecheck=CHECK):

int add_mul(int a, int b, int c) { return a + b * c; }

void main()
{
    int result = add_mul(2, 3, 4);
    // CHECK: mul-over-add: 14
    printf("mul-over-add: %d\n", result);
}
