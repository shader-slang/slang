//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=65450b8dda1f9d9a117c16039a3f26495eb5f93db859e2bae7274f12de3d37cc
//META: purpose=Fires diagnostic E15500 (expected-token-in-macro-arguments) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15500
//META: catalog_name=expected-token-in-macro-arguments
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15500.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M(a,b) a
int x = M(1
;
void main() { x; }

//CHECK: E15500
