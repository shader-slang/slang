//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f2f33c070c64520bf27ee31c54a7ee20251a633259b6cf2c2a287fb66e685672
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


//CHECK: E31224
