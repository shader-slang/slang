//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=cab8a95425f208e45eb1ac8c90c4e6ccc41ac77181e4813afd6d9de7a0e1f638
//META: purpose=Fires diagnostic E15205 (undefined-identifier-in-preprocessor-expression) - undefined identifier in preprocessor
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15205
//META: catalog_name=undefined-identifier-in-preprocessor-expression
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An identifier that is not a defined macro evaluates to 0 in a conditional,
// but the preprocessor still warns so typos in feature checks are visible. The
// CHECK pins that warning code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#if UNDEFINED_IDENTIFIER_XYZ
#endif
void main() {}

//CHECK: E15205
