//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W39071 (binding-attribute-ignored-on-uniform) - binding attribute ignored
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39071
//META: catalog_name=binding-attribute-ignored-on-uniform
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A binding attribute on a global uniform (not inside a constant
// buffer) is ignored and triggers W39071.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

[[vk::binding(1,0)]]
uniform float v;

[numthreads(1,1,1)]
void main() {}

// CHECK: W39071
