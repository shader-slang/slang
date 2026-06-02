//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
//META: purpose=Boundary - a CharLiteral whose content is an escape sequence (`'\\'`, `'\n'`) lexes as one token and decodes to the escaped code point.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: an escape sequence inside single quotes is a
// single CharLiteral. `'\\'` is backslash (ASCII 92); `'\n'` is
// newline (ASCII 10).

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: bs=92
    printf("bs=%d\n", int('\\'));
    // CHECK: nl=10
    printf("nl=%d\n", int('\n'));
}
