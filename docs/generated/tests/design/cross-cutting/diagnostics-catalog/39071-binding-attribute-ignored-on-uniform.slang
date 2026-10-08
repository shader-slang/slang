//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=53f6ba7d88c2a2362d735ce30a617ef5cbead5b6466120e91400836af47590d1
//META: purpose=Fires diagnostic E39071 (binding-attribute-ignored-on-uniform) - binding attribute ignored
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39071
//META: catalog_name=binding-attribute-ignored-on-uniform
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A binding attribute on a global uniform (not inside a constant
// buffer) is ignored and triggers E39071.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target spirv -stage compute -entry main

[[vk::binding(1,0)]]
uniform float v;

[numthreads(1,1,1)]
void main() {}

//CHECK: E39071