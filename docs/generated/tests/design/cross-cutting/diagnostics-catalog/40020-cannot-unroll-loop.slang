//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=6cb2c12ecb1a23efd056d3000768cb8eb41b50489d283b94104b666622d04a89
//META: purpose=Fires diagnostic E40020 (cannot-unroll-loop) - loop unrolling failed
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=40020
//META: catalog_name=cannot-unroll-loop
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A [ForceUnroll] loop with a runtime bound cannot terminate during
// the limited iteration budget — the unrolling pass aborts.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> o;

[numthreads(1,1,1)]
void main(uniform int n) {
    int sum = 0;
    [ForceUnroll]
    for (int i = 0; i < n; ++i) { sum += i; }
    o[0] = sum;
}

//CHECK: E40020
