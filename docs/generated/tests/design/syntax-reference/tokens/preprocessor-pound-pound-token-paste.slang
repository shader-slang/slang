//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#preprocessor-markers
//META: doc_section_digest=201ff9242516697daa268085dbc9f99b91d0fc710e1da33e7eb81daa78517f72
//META: purpose=The two-character spelling `##` lexes as a single `PoundPound` token that the preprocessor uses to paste adjacent macro arguments into one identifier.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies `##` is one token rather than two `Pound` tokens. Two separate
// `Pound` tokens in a macro body would be directive prefixes, not a paste
// operator, and the expansion would not name an existing function. The pasted
// name is defined elsewhere in the file, so a successful call proves the paste.

//TEST:INTERPRET(filecheck=CHECK):

#define JOIN(a, b) a##b

int fooBar() { return 5; }

void main()
{
    // CHECK: paste=5
    printf("paste=%d\n", JOIN(foo, Bar)());
}
