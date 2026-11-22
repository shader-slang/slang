//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=6c5b24137d8a00d45340c81231f003fe2cc4825dcc7a3203d181feeb7da8b977
//META: purpose=Negative probe: a `\u` escape with fewer than 4 hex digits is diagnosed as an invalid Unicode string escape (E10008).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the fixed-width rule of the `\u` escape form: the escape
// below provides only two hex digits before a non-hex character, so
// the ill-formed-Unicode-escape diagnostic fires, pinned to code
// E10008 with the digit-count requirement in its message.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    String s = "ab\u12xy";
    //CHECK: \u must be followed by 4 hex digits
    // CHECK: E10008
}
