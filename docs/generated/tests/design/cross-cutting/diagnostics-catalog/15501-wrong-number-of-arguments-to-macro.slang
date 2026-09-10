//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=668864fb272a8525ac1c1740debefea9d1f99a89fed8f3c6676e3dba99ff06d4
//META: purpose=Fires diagnostic E15501 (wrong-number-of-arguments-to-macro) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15501
//META: catalog_name=wrong-number-of-arguments-to-macro
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15501.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#define M(a,b) a
int x = M(1);
void main() { x; }

//CHECK: E15501
