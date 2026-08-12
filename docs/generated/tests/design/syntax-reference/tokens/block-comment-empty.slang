//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#trivia-whitespace-and-comments
//META: doc_section_digest=e8da9aa08a65016dd5b8d75b6932a8c7ee98601f324137231a1bcc21e05b49ac
//META: purpose=Boundary - an empty block comment (`/**/`) is a valid BlockComment token and is removed from the token stream.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: zero-content block comment must be accepted by
// the lexer. The expression `1 + /**/ 2` must parse as `1 + 2`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int v = 1 + /**/ 2;
    // CHECK: v=3
    printf("v=%d\n", v);
}
