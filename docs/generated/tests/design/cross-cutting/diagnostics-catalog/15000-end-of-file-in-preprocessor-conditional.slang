//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e967b66051644b45362d65845ab54513514d4baa866ac2605db0777ea9f6bd9b
//META: purpose=Fires diagnostic E15000 (end-of-file-in-preprocessor-conditional) - end of file encountered during preprocessor conditional
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15000
//META: catalog_name=end-of-file-in-preprocessor-conditional
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A conditional block that reaches end of file without `#endif` is unbalanced.
// The CHECK pins the end-of-file-in-conditional error, proving the preprocessor
// tracks conditional nesting to EOF rather than closing it implicitly.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#if 1
void main() {}

//CHECK: E15000
