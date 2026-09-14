//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=647b640bd681c2f430950851d8d396048217ee710b1310283de6eb1e46d0cb36
//META: purpose=Bare non-ASCII source bytes in a character literal are decoded as UTF-8 into a single code point; `'é'` yields 233.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a character literal written with raw UTF-8 source
// bytes (no escape) decodes to one code point: 'é' is the two-byte
// sequence C3 A9 in this file, and UTF-8 decoding must yield
// U+00E9 = 233. If the bytes were taken individually, the literal
// would instead be rejected as multi-character.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: cp=233
    printf("cp=%d\n", int('é'));
}
