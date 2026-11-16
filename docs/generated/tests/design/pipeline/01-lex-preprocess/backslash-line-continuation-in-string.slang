//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules
//META: doc_section_digest=7c196db588b1b46ddab5c3772a5da1bb60ade3121eac6723644f0c4808d045b7
//META: purpose=Boundary probe of backslash line continuation: a `\<newline>` inside a string literal is folded out, so the two physical lines become one logical literal.
//META: intent=boundary
//META: pipeline_stage=lex
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The string literal `"foo\<newline>bar"` spans two physical lines.
// Backslash-newline must be consumed by the lexer so the string
// content is `"foobar"`. We emit to CPP and FileCheck the emitted
// literal (slangi printf does not support `%s`).

//TEST:SIMPLE(filecheck=CHECK):-target cpp -entry main -stage compute

[shader("compute")]
[numthreads(1, 1, 1)]
void main()
{
    printf("%s\n", "foo\
bar");
    //CHECK: foobar
}
