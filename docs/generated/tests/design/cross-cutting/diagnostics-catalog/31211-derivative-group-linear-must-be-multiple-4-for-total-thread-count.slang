//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f374ca1ebf6ac1c4b4afb40502e624015ef89a29389d18ed4b2eddb8a15be2ad
//META: purpose=Fires diagnostic E31211 (derivative-group-linear-must-be-multiple-4-for-total-thread-count) -- derivative group linear thread count error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31211
//META: catalog_name=derivative-group-linear-must-be-multiple-4-for-total-thread-count
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeGroupLinear][shader("compute")][numthreads(3,1,1)]
void main() {}


//CHECK: E31211
