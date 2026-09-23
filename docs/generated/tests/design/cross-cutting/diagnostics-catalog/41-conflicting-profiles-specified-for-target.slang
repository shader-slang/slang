//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=58b6bf61c9b5f2b459b6e02438ff20d5c1ea843ea8627e51138a043e5e127937
//META: purpose=Fires diagnostic E00041 (conflicting-profiles-specified-for-target) - conflicting profiles have been specified for target '~target'
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=41
//META: catalog_name=conflicting-profiles-specified-for-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two different `-profile` values for one target cannot both apply. The CHECK
// pins the conflicting-profiles error to keep that rejection observable.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -profile sm_6_0 -profile sm_5_0 -entry main -stage compute
[numthreads(1,1,1)]
void main() {}

//CHECK: E00041
