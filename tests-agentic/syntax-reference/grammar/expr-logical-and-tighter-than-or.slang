//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/llm-generated/syntax-reference/grammar.md#expressions
//META: doc_section_digest=da8363e39914cfb7152b84e17c34f7e9f5e6ae76aad2de17997f38aecabfc651
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
