//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#lexer-flags-and-special-case-rules
//META: doc_section_digest=ab0cf5e019a8433b239b8a025cd2c3014f7a03ebec00c3c2c6d172456d9b0e3c
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
