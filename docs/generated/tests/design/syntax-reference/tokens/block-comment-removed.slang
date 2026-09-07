//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#trivia-whitespace-and-comments
//META: doc_section_digest=e8da9aa08a65016dd5b8d75b6932a8c7ee98601f324137231a1bcc21e05b49ac
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
