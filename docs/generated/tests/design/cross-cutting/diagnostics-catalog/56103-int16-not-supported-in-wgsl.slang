//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f8d268338658ab79c6b91a929a6eb483dee80e4e0947242116b2e8493a2776b0
//META: purpose=Fires diagnostic E56103 (int16-not-supported-in-wgsl) - 16-bit integers not supported in WGSL
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=56103
//META: catalog_name=int16-not-supported-in-wgsl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL does not have a 16-bit integer type. Using `int16_t` for
// any value visible in the emitted code is rejected.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

RWStructuredBuffer<int16_t> buf;

[numthreads(1,1,1)]
void main() {
    buf[0] = int16_t(1);
}

//CHECK: E56103
