//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=fdd590cd68b5461dae40c566b8e0d5a22f0ec1614037758d1bf966b66fc61695
//META: purpose=Fires diagnostic E31006 (attribute-function-not-found) -- function not found for attribute
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31006
//META: catalog_name=attribute-function-not-found
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[domain("tri")]
[outputcontrolpoints(3)]
[outputtopology("triangle_cw")]
[partitioning("integer")]
[patchconstantfunc("nothere")]
void main() {}


// CHECK: E31006
