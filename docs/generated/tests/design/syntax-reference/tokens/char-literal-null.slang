//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=98ff1b59e664e45b3e67fde56e5d2482e7d7845bb990f58cbca3a8c2edfcdd84
//META: purpose=Boundary - the null escape `'\0'` lexes as a CharLiteral and decodes to integer 0.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the lowest-value CharLiteral (`'\0'`) is a valid
// single-character literal whose code point is 0.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: code=0
    printf("code=%d\n", int('\0'));
}
