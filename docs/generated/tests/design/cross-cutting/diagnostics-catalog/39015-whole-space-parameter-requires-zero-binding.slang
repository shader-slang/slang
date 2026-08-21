//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=80fc211b5014ebf21cef23474f5d86506eab664273088eb817a27c89d0f5304b
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

//CHECK: E39015
