//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=c2b089bd56c81495024bd0c4ab60fe6f0fbb5efb1de0664edab184b7d9a5b03a
//META: purpose=Fires diagnostic E00018 (separate-debug-info-unsupported-for-target) - '-separate-debug-info' is not supported for target '~target'
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=18
//META: catalog_name=separate-debug-info-unsupported-for-target
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Separate debug information is only produced for targets that can carry it.
// Requesting it for the HLSL text target produces a warning rather than an
// error, so the CHECK pins the warning code and the compile still proceeds.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry main -stage compute -separate-debug-info
[numthreads(1,1,1)]
void main() {}

//CHECK: E00018
