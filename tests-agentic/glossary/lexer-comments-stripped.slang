//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:30:00+00:00
//META: source_commit=330c9a8d807b9f9352e4754f466d1244ae681cff
//META: doc_ref=docs/llm-generated/glossary.md#terms
//META: doc_section_digest=c590da2fa78de86b023f0aa1d0df2f46a79bb2de597705bc7ed2fee90af2869b
//META: purpose=lexer: the lexer converts a source character stream into a token stream, stripping `//` line comments and `/* ... */` block comments as trivia so they are not visible to later stages.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies the "lexer" glossary entry: an interleaved comment is
// removed by the lexer before the parser sees the tokens, so the
// declaration `int x = 5;` is still well-formed even though a
// `/* ... */` sits between the type and the name, and a `//` line
// comment between two statements does not break the second one.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int /* trivia */ x /* more */ = 5; // tail comment
    // standalone comment line
    int y = x + 1;
    // CHECK: lex=6
    printf("lex=%d\n", y);
}
