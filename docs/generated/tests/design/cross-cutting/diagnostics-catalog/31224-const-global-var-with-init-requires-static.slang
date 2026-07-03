//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=3841db1fe6ea6cc56d14f70da0b620384a824b5e3ff9a9fa3ce2d6a1f26975e2
//META: purpose=Fires diagnostic E31224 (const-global-var-with-init-requires-static) -- global const requires static
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31224
//META: catalog_name=const-global-var-with-init-requires-static
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

const int x = 5;
[shader("compute")][numthreads(1,1,1)]
void main() {}


// CHECK: E31224
