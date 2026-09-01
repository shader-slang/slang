//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0c0e8bbd8e4683fb76f4a737bca99dfec76f0ec9720957e06b9e6d36f821c3db
//META: purpose=Fires diagnostic E45102 (coverage-target-not-supported) - `-trace-coverage` is not supported on this target; coverage instrumentation skipped
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=45102
//META: catalog_name=coverage-target-not-supported
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// -trace-coverage requires atomic-int operations on the counter
// buffer; the WGSL backend cannot satisfy this — coverage is
// skipped with a warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target wgsl -entry main -stage compute -trace-coverage

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) {
    if (tid.x > 0) o[0] = 1;
    else o[0] = 2;
}

//CHECK: E45102
