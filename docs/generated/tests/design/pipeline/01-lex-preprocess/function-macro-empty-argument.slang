//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=74e43f68dacbee88ce5a4d003aeba806114c7db9ff5ee4ebefea236b92eca467
//META: purpose=Boundary probe of function-style macros: an empty argument (the comma is present, the token list before/after is empty) substitutes to nothing.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `WRAP(7,)` — the second argument is the empty token sequence.
// The macro body `(a) + (b)` must expand to `(7) + ()`, which is
// a syntax error in C but Slang's preprocessor / parser accepts
// the empty-pair `()` as zero. We probe with `WRAP(7, 5)` and
// `WRAP(, 5)` to verify the empty-front variant.

//TEST:INTERPRET(filecheck=CHECK):

#define FIRST(a, b) (a)
#define SECOND(a, b) (b)

void main()
{
    // CHECK: first-empty-rest=5
    printf("first-empty-rest=%d\n", SECOND(, 5));
    // CHECK: second-empty=7
    printf("second-empty=%d\n", FIRST(7,));
}
