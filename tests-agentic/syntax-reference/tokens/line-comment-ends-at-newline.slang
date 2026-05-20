//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/llm-generated/syntax-reference/tokens.md#trivia-whitespace-and-comments
//META: doc_section_digest=d9d684efc155e9dcf78590f8a9df4e524c5d0a4373374fb6cbb45aaf8f48b844
//META: purpose=A `LineComment` (`// ...`) ends at the end of its physical line; code on the following line is not part of the comment.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that `//` comments terminate at the newline. The
// declaration `int x = 9;` on the line *after* the `//` comment must
// still be parsed and produce the expected value.

//TEST:INTERPRET(filecheck=CHECK):

// this comment ends here; the next line is real code
static const int x = 9;

void main()
{
    // CHECK: x=9
    printf("x=%d\n", x);
}
