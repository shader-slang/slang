//META: generated=true
//META: model=claude-sonnet-5-5
//META: generated_at=2026-11-09T10:40:15+00:00
//META: source_commit=ae4e57957a0adad3061b069b0e0eec8217779338
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=3a235903648fca659462d6304faeb328f1fb574dd3679e127f14ad5b78f932b7
//META: purpose=A variadic macro requires only its non-variadic parameters and accepts any number of trailing arguments.
//META: intent=boundary
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A variadic macro with one fixed parameter is valid with zero, one, or
// several trailing arguments; the arity check only counts the fixed ones.

//TEST:INTERPRET(filecheck=CHECK):

#define SUM3(a, ...) (a + __VA_ARGS__ + 0)
#define FIRST(a, ...) (a)

void main()
{
    // CHECK: first=1,1,1
    printf("first=%d,%d,%d\n", FIRST(1), FIRST(1, 2), FIRST(1, 2, 3));
}
