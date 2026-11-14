//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=d3d4c02ac4dafd74393b443c556304f894229442554014fb013287b9cae88960
//META: purpose=A `\'` inside a character literal is stepped over while lexing, so `'\''` is one CharLiteral that decodes to the apostrophe code point 39.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the closing-quote search skips an escaped quote instead of ending
// the token at it. If the escape were not stepped over, `'\''` would close
// after the backslash and the trailing quote would start a new, unterminated
// literal. Printing the decoded value proves both that one token was produced
// and that it decoded to the apostrophe.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: quote=39
    printf("quote=%d\n", int('\''));
}
