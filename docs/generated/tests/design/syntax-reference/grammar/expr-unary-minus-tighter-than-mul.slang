//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=da8363e39914cfb7152b84e17c34f7e9f5e6ae76aad2de17997f38aecabfc651
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
