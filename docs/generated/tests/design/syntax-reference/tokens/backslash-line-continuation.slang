//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
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
