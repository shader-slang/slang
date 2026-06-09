//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=510294c72102216c9ce081d3f044769f9c4bfd6d3087f1d79082cf1e5fb0e398
//META: purpose=Fires diagnostic E30521 (interface-default-initializer) -- initializing an interface variable with defaults is deprecated and may cause unexpected behavior
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30521
//META: catalog_name=interface-default-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

interface IFoo {}
[shader("compute")][numthreads(1,1,1)]
void main() {
    IFoo x = {};
}


// CHECK: E30521
