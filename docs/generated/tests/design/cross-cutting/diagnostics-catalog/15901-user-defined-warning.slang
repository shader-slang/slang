//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=97bc6c96922f0ded35d01c1c6f6bad0bd483a7d1ec42ee18a5b8d30eb15de634
//META: purpose=Fires diagnostic E15901 (user-defined-warning) - preprocessor warning
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15901
//META: catalog_name=user-defined-warning
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// `#warning` reports its message without failing the compile. The CHECK pins
// the user-defined-warning code, which distinguishes it from `#error`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#warning deliberate user-defined preprocessor warning
void main() {}

//CHECK: E15901
