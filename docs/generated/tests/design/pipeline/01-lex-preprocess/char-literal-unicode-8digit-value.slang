//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=6c5b24137d8a00d45340c81231f003fe2cc4825dcc7a3203d181feeb7da8b977
//META: purpose=A `\UNNNNNNNN` escape (exactly 8 hex digits) in a character literal decodes to the named code point; `'\U00000041'` yields 65.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the 8-hex-digit `\U` Unicode escape form on the char
// value-extraction path. `\U00000041` names U+0041; the decoded
// code point printed as an integer must be 65, proving all eight
// digits were consumed as one escape.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: u8=65
    printf("u8=%d\n", int('\U00000041'));
}
