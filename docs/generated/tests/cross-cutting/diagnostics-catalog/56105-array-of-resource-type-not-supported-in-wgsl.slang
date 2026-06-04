//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E56105 (array-of-resource-type-not-supported-in-wgsl) - array of resource types not supported in WGSL
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=56105
//META: catalog_name=array-of-resource-type-not-supported-in-wgsl
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// WGSL does not support arrays of resource (buffer) types as a
// single global. The emit pass rejects the declaration.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target wgsl -entry main -stage compute

StructuredBuffer<int> buffers[4];
RWStructuredBuffer<int> out_;

[numthreads(1,1,1)]
void main() {
    out_[0] = buffers[0][0];
}

// CHECK: E56105
