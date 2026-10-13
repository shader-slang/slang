//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#content-tokens
//META: doc_section_digest=c2efc22fd6b393971554625ec6e5d1342ed8b02f96b9afdf61ab48806b3d700e
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
