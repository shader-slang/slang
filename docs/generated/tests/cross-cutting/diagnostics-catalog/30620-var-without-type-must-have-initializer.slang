//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0a4736f8516c374747010569ec0cdcb0b3466b546545624d3f4bfeddc0c082d1
//META: purpose=Fires diagnostic E30620 (var-without-type-must-have-initializer) -- a variable declaration without an initial-value expression must be given an explicit type
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=30620
//META: catalog_name=var-without-type-must-have-initializer
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

var x;
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E30620
