//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=5bd3f7fd4d4aec36189e915e3bcaded5ad098fe089b37ce5114d8bfba37a75a3
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
