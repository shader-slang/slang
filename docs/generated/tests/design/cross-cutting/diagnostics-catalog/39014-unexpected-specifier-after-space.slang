//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=089a2ea78386f9bec1f3befcdeaa31e083427b08197641aacb722efc2f0b23a8
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

//CHECK: E39014
