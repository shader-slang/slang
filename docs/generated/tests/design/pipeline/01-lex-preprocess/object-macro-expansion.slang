//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=3a235903648fca659462d6304faeb328f1fb574dd3679e127f14ad5b78f932b7
//META: purpose=Object-like `#define` stores a lexed token sequence that is replayed verbatim at every use, so the expanded value is what the parser sees.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an object-like macro is expanded into its definition
// at the point of use. If the macro were not expanded, the test would
// fail to parse (the identifier `ANSWER` is not declared anywhere).

//TEST:INTERPRET(filecheck=CHECK):

#define ANSWER 42

void main()
{
    // CHECK: answer=42
    printf("answer=%d\n", ANSWER);
}
