//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=6c5b24137d8a00d45340c81231f003fe2cc4825dcc7a3203d181feeb7da8b977
//META: purpose=A `\uNNNN` escape (exactly 4 hex digits) in a character literal decodes to the named code point; `'\u0041'` yields 65.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the 4-hex-digit `\u` Unicode escape form on the char
// value-extraction path. `\u0041` names U+0041; the decoded 32-bit
// code point printed as an integer must be 65.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: u4=65
    printf("u4=%d\n", int('\u0041'));
}
