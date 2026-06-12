//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T17:03:32+00:00
//META: source_commit=ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=da8363e39914cfb7152b84e17c34f7e9f5e6ae76aad2de17997f38aecabfc651
//META: purpose=`AtomExpr` accepts a parenthesized expression `'(' Expr ')'` and the parens override precedence.
//META: intent=functional
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the parenthesized `AtomExpr` production. Without parens,
// `2 + 3 * 4` is 14; with the parens regrouping, `(2 + 3) * 4` is
// 20 — confirming both that the `(Expr)` form parses and that it
// forces the inner grouping.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int r = (2 + 3) * 4;
    // CHECK: r=20
    printf("r=%d\n", r);
}
