//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0cfcc5b32075f14184a53fd8e0c5b3e189706b5def8f0eafc7db9b5182d002f5
//META: purpose=Fires diagnostic E31223 (extern-and-export-var-decl-must-be-const) -- extern/export requires static const
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31223
//META: catalog_name=extern-and-export-var-decl-must-be-const
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static extern int x = 5;
[shader("compute")][numthreads(1,1,1)]
void main() {}


//CHECK: E31223
