//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments
//META: doc_section_digest=d9d684efc155e9dcf78590f8a9df4e524c5d0a4373374fb6cbb45aaf8f48b844
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
