//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0457c77489c9cb94e69218e7de0cdd8032eac44a2b4b098593f3d016ec31a599
//META: purpose=Fires diagnostic E31217 (unrecognized-glsl-layout-qualifier-or-requires-assignment) -- unrecognized GLSL layout qualifier
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31217
//META: catalog_name=unrecognized-glsl-layout-qualifier-or-requires-assignment
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

layout(set) int x;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E31217
