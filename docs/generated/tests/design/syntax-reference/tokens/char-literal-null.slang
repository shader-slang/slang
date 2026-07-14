//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=0e1e5b6a05be01d6e2a0fbe1d41469c918569522adb5871ac6efa8d2a0811ba4
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
