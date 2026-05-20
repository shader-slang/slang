//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/llm-generated/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=72d10317b8f57d7be142f82b7bea6dbc422be9f78ab169aad0cfc0f9adfac32a
//META: purpose=A single-quoted character literal (`CharLiteral`) is recognized by the lexer and carries the character's integer code-point value.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that the char literal `'A'` evaluates to the ASCII code for
// 'A' (65). The claim under test is that the lexer recognizes the
// single-quoted form; the value 65 is the documented ASCII mapping.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    // CHECK: code=65
    printf("code=%d\n", int('A'));
}
