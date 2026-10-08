//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#trivia-whitespace-and-comments
//META: doc_section_digest=e8da9aa08a65016dd5b8d75b6932a8c7ee98601f324137231a1bcc21e05b49ac
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
