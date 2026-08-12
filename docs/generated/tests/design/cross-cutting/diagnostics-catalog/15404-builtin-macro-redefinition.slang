//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=75b224783de1fb2ee653ff0246ed117c37853df359c78d8810b049179c61d112
//META: purpose=Fires diagnostic E15404 (builtin-macro-redefinition) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15404
//META: catalog_name=builtin-macro-redefinition
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15404.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define __FILE__ 5
void main() { __FILE__; }

//CHECK: E15404
