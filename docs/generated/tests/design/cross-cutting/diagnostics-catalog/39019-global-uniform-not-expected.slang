//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W39019 (global-uniform-not-expected) - implicit global shader parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39019
//META: catalog_name=global-uniform-not-expected
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A global ordinary variable without `uniform` or `static`
// becomes an implicit shader parameter, triggering W39019.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_6_0

float someGlobal;

[numthreads(1,1,1)]
void main() {}

// CHECK: W39019
