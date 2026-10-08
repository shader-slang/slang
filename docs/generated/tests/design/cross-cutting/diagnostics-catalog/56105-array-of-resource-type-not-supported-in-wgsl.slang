//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=b22f9e0b695939c6ab63eb94ea8081cf1d98f53e8206d9c8d9042bb3d24dd3b7
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

//CHECK: E56105
