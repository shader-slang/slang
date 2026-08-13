//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=667410b964ed99fbf0c70015e2ab77e7656e9f8728c17d66aa0a1cad3ef6f6b5
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
