//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=72776cf488f2de22fb8f436ae9360f07f292a93d70f2e09de5c0f5deba27b671
//META: purpose=Fires diagnostic E15101 (unknown-preprocessor-directive) - unknown directive
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15101
//META: catalog_name=unknown-preprocessor-directive
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Directive names are matched against a fixed set. A `#`-prefixed name outside
// that set is diagnosed as an unknown directive rather than skipped, which the
// CHECK observes by code.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#bogusdirective
void main() {}

//CHECK: E15101
