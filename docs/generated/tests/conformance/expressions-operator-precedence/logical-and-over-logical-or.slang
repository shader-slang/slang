//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-11 `&&` binds more tightly than level-12 `||`: `false || true && false` groups as `false || (true && false)` = false.
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says `&&` is level 11, `||` is level 12.
// Level 11 is lower numeric = higher precedence, so `&&` binds before `||`.
// `false || true && false`:
//   `&&` first: `false || (true && false)` = `false || false` = false (0)
//   `||` first: `(false || true) && false` = `true && false` = false (same!)
// Need values where the two groupings differ:
// `true || false && false`:
//   `&&` first: `true || (false && false)` = `true || false` = true (1)
//   `||` first: `(true || false) && false` = `true && false` = false (0)

//TEST:INTERPRET(filecheck=CHECK):

int log_and_or(bool a, bool b, bool c) { return (a || b && c) ? 1 : 0; }

void main()
{
    // true || (false && false) = true || false = true = 1
    // (true || false) && false = true && false = false = 0
    int result = log_and_or(true, false, false);
    // CHECK: logical-and-over-or: 1
    printf("logical-and-over-or: %d\n", result);
}
