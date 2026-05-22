//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T12:00:00+00:00
//META: source_commit=ef4af96c0996402dfe65ab0fdd347e4ae7e1a742
//META: doc_ref=docs/llm-generated/pipeline/01-lex-preprocess.md#macro-expansion
//META: doc_section_digest=5bd3f7fd4d4aec36189e915e3bcaded5ad098fe089b37ce5114d8bfba37a75a3
//META: purpose=Negative probe of function-style macros: invoking a 2-parameter macro with only 1 argument is rejected with the documented arg-count diagnostic.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// The doc says "function-style macros bind parameter names to
// argument tokens". When the argument list does not match the
// parameter list, the preprocessor rejects the invocation. We
// probe the under-supply boundary.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):

#define BIN(a, b) ((a) + (b))

static const int q = BIN(1);
//CHECK:                ^ wrong macro argument count
//CHECK:                ^ wrong number of arguments to macro (expected 2, got 1)
// CHECK: E15501

void main() {}
