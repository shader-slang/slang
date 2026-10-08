//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=751f35c40cfee070d14f2caabd3a7068445501147b6307bca6ebe52fdae2d138
//META: purpose=Fires diagnostic E41015 (using-uninitialized-out) - use of uninitialized out parameter
//META: intent=negative
//META: pipeline_stage=ir-pass
//META: catalog_code=41015
//META: catalog_name=using-uninitialized-out
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Reading from an `out` parameter before assigning to it produces
// the uninitialized-out-parameter warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

RWStructuredBuffer<int> o;

void f(out int x) { o[0] = x; x = 0; }

[numthreads(1,1,1)]
void main() {
    int a;
    f(a);
}

//CHECK: E41015
