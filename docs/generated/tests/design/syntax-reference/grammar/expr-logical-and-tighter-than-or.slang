//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=667410b964ed99fbf0c70015e2ab77e7656e9f8728c17d66aa0a1cad3ef6f6b5
//META: purpose=Precedence: `&&` (level 11) binds tighter than `||` (level 12).
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `false || true && false` parses as
// `false || (true && false) == false`. If `||` bound tighter, it would
// parse as `(false || true) && false == false` — but the chosen
// operands distinguish: `true || false && false` is `true` either way,
// while `false || true && true` is `true` both ways. We use
// `true || false && false`: with documented precedence it's
// `true || (false && false) == true || false == true`; with reversed
// precedence it's `(true || false) && false == false`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    bool r = true || false && false;
    // CHECK: r=1
    printf("r=%d\n", int(r));
}
