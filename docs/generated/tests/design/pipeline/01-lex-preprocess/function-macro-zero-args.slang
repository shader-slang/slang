//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=74e43f68dacbee88ce5a4d003aeba806114c7db9ff5ee4ebefea236b92eca467
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
