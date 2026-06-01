//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
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


// CHECK: E31223
