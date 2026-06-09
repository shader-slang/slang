//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic E39015 (whole-space-parameter-requires-zero-binding) - whole descriptor set requires binding 0
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39015
//META: catalog_name=whole-space-parameter-requires-zero-binding
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A ParameterBlock consumes a whole descriptor set; non-zero
// `[[vk::binding(N, ...)]]` triggers E39015.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

struct GS { int v; }

[[vk::binding(1,0)]]
ParameterBlock<GS> g;

[numthreads(1,1,1)]
void main() {}

// CHECK: E39015
