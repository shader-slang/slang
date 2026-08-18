//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=36a8dad99ac4edf423dbc20e1230c8b8923b08e25dfb7fa7a4bbbde8c24ca2b2
//META: purpose=Fires diagnostic E31215 (cannot-use-unsized-type-in-constant-buffer) -- unsized type in constant buffer
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31215
//META: catalog_name=cannot-use-unsized-type-in-constant-buffer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

struct S {
    int data[];
}
ConstantBuffer<S> cb;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E31215
