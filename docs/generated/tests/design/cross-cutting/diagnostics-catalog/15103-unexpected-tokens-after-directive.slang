//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=8901bae0520834a471d72058bcccf5234f112d12d6c0fb8643ba3cf07196421f
//META: purpose=Fires diagnostic E15103 (unexpected-tokens-after-directive) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15103
//META: catalog_name=unexpected-tokens-after-directive
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15103.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#endif x
void main() {}

//CHECK: E15103
