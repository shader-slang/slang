//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#end-markers-and-special
//META: doc_section_digest=7d027eaef54f8d01019f7c8500338fd2356fbe2e30aea34768af0343dfee8c53
//META: purpose=Negative - a character matching no lexer dispatch arm yields an `Invalid` token and the printable-character form of the illegal-character diagnostic (E10000).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fall-through arm of the per-character dispatch. A backtick is
// ASCII-printable and is not part of any Slang token spelling, so the lexer
// must select the printable spelling of the diagnostic (quoting the character)
// rather than the hex spelling. The CHECK pins the character inside the message
// so a switch to the hex form would be caught.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    int x = 1; `
//CHECK:       ^ error
// CHECK: E10000
    printf("%d\n", x);
}
