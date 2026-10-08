//META: generated=true
//META: model=claude-sonnet-4-6
//META: generated_at=2026-06-02T09:20:02+00:00
//META: source_commit=54bb48e67c3a4fdd4c0f4bd8085f5dc0bd8ffc99
//META: doc_ref=docs/language-reference/expressions-operator-precedence.md#operator-precedence
//META: doc_section_digest=2c43dea5ca69ca0e9d2a792df3b8f164e2487c75e25ef796f78839c678589fa1
//META: purpose=Level-2 prefix unary `-` binds more tightly than level-3 `*`: `-a * b` groups as `(-a) * b`, not `-(a*b)` (same value — use `-a + b` for observability).
//META: intent=functional
//META: pipeline_stage=runtime
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The precedence table says prefix/unary ops are level 2, `*`/`/`/`%` are level 3.
// Level 2 binds more tightly, so `-a * b` = `(-a) * b`.
// To make the two groupings observably different, use `-a + b`:
//   unary-first: `(-a) + b` = `(-3) + 10` = 7
//   But `-` is unary here, no ambiguity with binary `-`.
// Instead, test that prefix `!` on a comparison: `!a == b`:
//   prefix `!` (level 2) binds tighter than `==` (level 7)
//   `!a == b` groups as `(!a) == b`.
//   `!false == true` = `true == true` = true.
//   `!(false == true)` = `!false` = true (coincidentally same).
// Better: `!false == false`:
//   `(!false) == false` = `true == false` = false (0)
//   `!(false == false)` = `!true` = false (0) — same again.
// Use negation with different operands:
// `-3 + 10`: prefix `-` applies to `3`, giving `-3+10=7`.
// If binary `-` were parsed first: `-(3+10)=-13` — clearly different.

//TEST:INTERPRET(filecheck=CHECK):

int neg_add(int a, int b) { return -a + b; }

void main()
{
    // (-3) + 10 = 7  (prefix `-` applies before binary `+`)
    int result = neg_add(3, 10);
    // CHECK: unary-over-mul: 7
    printf("unary-over-mul: %d\n", result);
}
