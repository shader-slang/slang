//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E56102 (division-by-matrix-not-supported) - division by matrix not supported
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=56102
//META: catalog_name=division-by-matrix-not-supported
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Matrix-by-matrix division is unsupported on WGSL (and Metal).
// The emit pass rejects the operation.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

RWStructuredBuffer<float> buf;

[numthreads(1,1,1)]
void main() {
    float4x4 m = float4x4(1.0);
    float4x4 r = m / m;
    buf[0] = r[0][0];
}

// CHECK: E56102
