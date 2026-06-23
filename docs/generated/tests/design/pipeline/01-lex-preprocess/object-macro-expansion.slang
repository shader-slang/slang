//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=5bd3f7fd4d4aec36189e915e3bcaded5ad098fe089b37ce5114d8bfba37a75a3
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
