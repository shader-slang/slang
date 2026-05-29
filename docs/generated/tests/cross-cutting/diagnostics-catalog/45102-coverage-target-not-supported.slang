//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E45102 (coverage-target-not-supported) - `-trace-coverage` is not supported on this target; coverage instrumentation skipped
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=45102
//META: catalog_name=coverage-target-not-supported
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// -trace-coverage requires atomic-int operations on the counter
// buffer; the WGSL backend cannot satisfy this — coverage is
// skipped with a warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute -trace-coverage

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main(uint3 tid : SV_DispatchThreadID) {
    if (tid.x > 0) o[0] = 1;
    else o[0] = 2;
}

// CHECK: E45102
