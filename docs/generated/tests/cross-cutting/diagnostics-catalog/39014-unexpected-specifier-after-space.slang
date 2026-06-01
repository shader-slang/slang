//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic E39014 (unexpected-specifier-after-space) - unexpected specifier after register space
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39014
//META: catalog_name=unexpected-specifier-after-space
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Putting a space specifier after a `register(space0)` (whole
// descriptor set) parameter triggers E39014.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_5_0

struct GS { int a; }
ParameterBlock<GS> g : register(space0, foo);

[numthreads(1,1,1)]
void main() {}

// CHECK: E39014
