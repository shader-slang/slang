//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=7d7ed33aa953c7c0c272d55f11582399c0d082ae0868d6bbce3751b746f3018a
//META: purpose=Fires diagnostic E31102 (non-positive-num-threads) - invalid 'numthreads' value
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31102
//META: catalog_name=non-positive-num-threads
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A thread-group extent must be positive, since a zero extent would launch no
// threads. The CHECK pins the non-positive-numthreads error for an X extent of
// 0.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
[numthreads(0,1,1)]
void main() {}

//CHECK: E31102
