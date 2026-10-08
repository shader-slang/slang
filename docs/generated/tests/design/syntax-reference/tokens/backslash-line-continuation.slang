//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=d3d4c02ac4dafd74393b443c556304f894229442554014fb013287b9cae88960
//META: purpose=A backslash immediately before a newline is consumed and folded out; the two halves of an identifier split across the continuation become a single token.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the line-continuation rule by splitting an identifier
// across two physical lines with a `\<newline>` between them. After
// folding, the identifier reads as `foobar` and resolves to the
// declared constant.

//TEST:INTERPRET(filecheck=CHECK):

static const int foobar = 11;

void main()
{
    int v = foo\
bar;
    // CHECK: v=11
    printf("v=%d\n", v);
}
