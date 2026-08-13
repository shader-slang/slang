//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/syntax-reference/tokens.md#special-case-lexing-rules
//META: doc_section_digest=790d7f70b0af6fa58e0bbfe83b09c796ce08d85a7e54ec041d9c3c7dfdcfd61b
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
