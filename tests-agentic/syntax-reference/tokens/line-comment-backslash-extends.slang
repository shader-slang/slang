//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T20:30:00+00:00
//META: source_commit=1099f9d83f3d63135650f759de64b32a4eee48aa
//META: doc_ref=docs/llm-generated/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=e44df04d9dcfdbcd540c6b84f9f1c3b8b556eee2b84931196b66361c17faa2df
//META: purpose=Boundary - backslash-line-continuation runs before lexing, so a `\<newline>` at the end of a `//` line splices the following physical line into the comment and the line is consumed by it.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Boundary probe: the line-continuation rule is applied before the
// lexer recognizes `//`-comment boundaries. A `\` at the end of a
// comment line therefore swallows the *next* physical line. The
// declaration `int swallowed = 0;` directly below the comment is
// consumed; only the line after it (`real`) becomes a real
// declaration that `main` can print.

//TEST:INTERPRET(filecheck=CHECK):

// this comment continues with a backslash \
int swallowed = 0;
static const int real = 21;

void main()
{
    // CHECK: real=21
    printf("real=%d\n", real);
}
