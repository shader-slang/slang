//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#literal-scanning-and-value-extraction
//META: doc_section_digest=60402c8d086f96269e6fe5211e0043b74eb989c5d25aea5312e6a4ba5d5f8bf4
//META: purpose=An octal `\NNN` escape in a character literal decodes to the code point it names; `'\101'` yields 65.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies octal-escape decoding: `\101` is octal for 65 ('A').
// If the decoder read the digits as decimal (101) or mis-handled
// the octal sub-loop, the printed value would differ from 65.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: octal=65
    printf("octal=%d\n", int('\101'));
}
