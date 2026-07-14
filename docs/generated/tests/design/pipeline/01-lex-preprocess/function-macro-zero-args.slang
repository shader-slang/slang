//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cf4f1b505a333d6ccc1a3cb482330c4dde4668bd530a4d0fe4905412ffd9448a
//META: purpose=Boundary probe of function-style macros: a zero-parameter macro `FOO()` is valid and expands without taking arguments.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Function-style macro with **zero** parameters. The parameter
// environment that the doc describes still has to be allocated (it
// is empty), and the invocation `ZERO_ARG()` must replay the body.

//TEST:INTERPRET(filecheck=CHECK):

#define ZERO_ARG() 42

void main()
{
    // CHECK: zero=42
    printf("zero=%d\n", ZERO_ARG());
}
