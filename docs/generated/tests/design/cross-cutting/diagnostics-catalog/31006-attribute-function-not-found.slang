//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=e8965581afe8d85cc901f2b6b33fd31d7f1240ed5423425d9be87c62f27c27c2
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


//CHECK: E31006
