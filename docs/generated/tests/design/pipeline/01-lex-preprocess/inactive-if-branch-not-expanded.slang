//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-20T15:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=5bd3f7fd4d4aec36189e915e3bcaded5ad098fe089b37ce5114d8bfba37a75a3
//META: purpose=Tokens inside an inactive `#if` branch flow through the lexer for accounting but their contents are not expanded or parsed.
//META: intent=functional
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Verifies that an inactive `#if 0` branch is discarded by the
// preprocessor. The branch contains a reference to an undeclared
// identifier (`undefined_symbol`) and a malformed statement; if the
// branch were not skipped, the file would fail to compile. The
// active `#else` provides the real definition used at runtime.

//TEST:INTERPRET(filecheck=CHECK):

#if 0
int value() { return undefined_symbol + ::: garbage; }
#else
int value() { return 5; }
#endif

void main()
{
    // CHECK: v=5
    printf("v=%d\n", value());
}
