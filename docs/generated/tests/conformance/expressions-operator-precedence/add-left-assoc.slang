//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-4 `+`/`-` are left-associative: `10 - 3 - 2` groups as `(10-3)-2` = 5, not `10-(3-2)` = 9.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `+`/`-` at level 4 have left associativity.
// Left assoc: `a - b - c` = `(a-b)-c`.
// If right-associative, `10 - 3 - 2` would be `10 - (3-2)` = 9.
// The correct left-associative result is `(10-3)-2` = 5.

//TEST:INTERPRET(filecheck=CHECK):

int sub3(int a, int b, int c) { return a - b - c; }

void main()
{
    int result = sub3(10, 3, 2);
    // CHECK: add-left-assoc: 5
    printf("add-left-assoc: %d\n", result);
}
