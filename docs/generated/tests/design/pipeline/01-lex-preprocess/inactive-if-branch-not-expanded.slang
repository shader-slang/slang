//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=docs/generated/design/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=cfe8fdce64ff12d1e1dbd2e4845beaf13fd1ba25fc9f4115e0139eab627320e7
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
