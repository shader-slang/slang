//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-3 `*`/`/`/`%` are left-associative: `16 / 4 / 2` groups as `(16/4)/2` = 2, not `16/(4/2)` = 8.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `*`/`/`/`%` at level 3 have left associativity.
// Left assoc: `a / b / c` = `(a/b)/c`.
// If right-associative, `16 / 4 / 2` would be `16 / (4/2)` = `16/2` = 8.
// The correct left-associative result is `(16/4)/2` = `4/2` = 2.

//TEST:INTERPRET(filecheck=CHECK):

int div3(int a, int b, int c) { return a / b / c; }

void main()
{
    int result = div3(16, 4, 2);
    // CHECK: mul-left-assoc: 2
    printf("mul-left-assoc: %d\n", result);
}
