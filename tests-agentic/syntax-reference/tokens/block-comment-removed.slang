//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments
//META: doc_section_digest=d9d684efc155e9dcf78590f8a9df4e524c5d0a4373374fb6cbb45aaf8f48b844
//META: purpose=A `BlockComment` (`/* ... */`) is treated as trivia; code on either side of the comment is parsed normally and the comment text has no semantic effect.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `/* ... */` is removed from the token stream. The
// inline block comment between the two halves of `x + /* ... */ y`
// must not break the expression — the result is the sum of `x` and
// `y`.

//TEST:INTERPRET(filecheck=CHECK):

void main()
{
    int x = 3;
    int y = 4;
    int z = x + /* commented away */ y;
    // CHECK: z=7
    printf("z=%d\n", z);
}
