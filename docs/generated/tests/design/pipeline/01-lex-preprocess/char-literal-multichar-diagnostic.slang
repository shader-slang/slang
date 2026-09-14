//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=Negative probe: a character literal holding more than one character is diagnosed at decode time as an illegal character literal (E10001).
//META: intent=negative
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "decode left unconsumed input" arm of the same
// one-character rule: the scanner happily records `'ab'` as one token,
// and only the value decode notices that a second character remains
// after the first was consumed. E10001 is pinned to prove the check
// fires, and the caret spans the whole literal token.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

void main()
{
    uint c = 'ab';
//CHECK:     ^^^^ illegal character literal
// CHECK: E10001
}
