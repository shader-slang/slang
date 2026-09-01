//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=5b868fbe8987cc7cabf940a0a29dde62c593e15483ad9c4ec5c73c3de150a737
//META: purpose=Fires diagnostic E15201 (syntax-error-in-preprocessor-expression) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15201
//META: catalog_name=syntax-error-in-preprocessor-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15201.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#if @
#endif
void main() {}

//CHECK: E15201
