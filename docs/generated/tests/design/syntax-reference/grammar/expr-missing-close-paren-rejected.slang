//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/grammar.md#expressions
//META: doc_section_digest=4e9de34fbc8a8877c6b73c6110db4d1ebef2d0ddd10da2bdfd2742413c1d6015
//META: purpose=A parenthesized expression atom requires a matching `)`; a missing close-paren is rejected.
//META: intent=negative
//META: pipeline_stage=parse
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `( Expr )` requires the closing paren. Omitting it
// causes a parse-stage diagnostic.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

void main()
{
    int x = (1 + 2;
    //CHECK: error
    //CHECK: E20001
    printf("x=%d\n", x);
}
