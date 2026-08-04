//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-5 `<<`/`>>` are left-associative: `1 << 2 << 1` groups as `(1<<2)<<1` = 8, not `1<<(2<<1)` = 16.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `<<`/`>>` at level 5 have left associativity.
// Left assoc: `a << b << c` = `(a<<b)<<c`.
// `(1<<2)<<1` = `4<<1` = 8.
// Right-associative `1<<(2<<1)` = `1<<4` = 16 — wrong.

//TEST:INTERPRET(filecheck=CHECK):

int shift_left2(int a, int b, int c) { return a << b << c; }

void main()
{
    int result = shift_left2(1, 2, 1);
    // CHECK: shift-left-assoc: 8
    printf("shift-left-assoc: %d\n", result);
}
