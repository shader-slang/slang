//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=667410b964ed99fbf0c70015e2ab77e7656e9f8728c17d66aa0a1cad3ef6f6b5
//META: purpose=Precedence: unary `-` (level 2) binds tighter than `*` (level 3).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `-2 * 3` parses as `(-2) * 3 == -6`, not `-(2 * 3)`.
// Both happen to yield -6 in arithmetic, so we distinguish with
// `-2 * -3 == 6` instead; a different grouping would not change the
// result either. Use a side-effecting form: `2 * -3 + 1` parses as
// `(2 * (-3)) + 1 == -5`. If unary `-` were looser, parsing would
// fail (no `2 * -` production at level 3).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int r = 2 * -3 + 1;
    // CHECK: r=-5
    printf("r=%d\n", r);
}
