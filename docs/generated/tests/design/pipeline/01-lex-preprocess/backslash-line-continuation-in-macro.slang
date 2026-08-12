//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules
//META: doc_section_digest=72b840c61355a14be7fbb014af8bcd6de560d5fc475d27cc6d1091a08e6f83e6
//META: purpose=The lexer handles backslash line continuations; a `\` immediately before a newline is consumed so the macro definition spans multiple physical lines as if it were one logical line.
//META: intent=functional
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that a `#define` body spanning multiple lines with
// trailing backslashes is lexed as one logical line. The macro
// `SUM3(a, b, c)` expands to `((a) + (b) + (c))`; calling
// `SUM3(1, 2, 3)` must yield 6. If the line continuation were not
// folded out, the macro body would terminate at the first newline
// and the expansion would be malformed.

//TEST:INTERPRET(filecheck=CHECK):

#define SUM3(a, b, c)  \
    ((a)               \
    + (b)              \
    + (c))

void main()
{
    // CHECK: sum=6
    printf("sum=%d\n", SUM3(1, 2, 3));
}
