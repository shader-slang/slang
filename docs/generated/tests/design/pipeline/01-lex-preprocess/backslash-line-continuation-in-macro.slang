//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules
//META: doc_section_digest=ab0cf5e019a8433b239b8a025cd2c3014f7a03ebec00c3c2c6d172456d9b0e3c
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
