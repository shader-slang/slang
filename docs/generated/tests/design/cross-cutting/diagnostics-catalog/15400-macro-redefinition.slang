//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=70844dcc7161f0f7dfeb32e1ea4a637bfa562eed6c74d5fce1d9db2dc6778fcd
//META: purpose=Fires diagnostic E15400 (macro-redefinition) - macro redefined
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15400
//META: catalog_name=macro-redefinition
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Redefining an object-like macro with a different body is legal but warned
// about, so a shadowed definition is not silent. The CHECK pins the
// macro-redefinition warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define DUP_MACRO 1
#define DUP_MACRO 2
void main() {}

//CHECK: E15400
