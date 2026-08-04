//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00+00:00
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=98e5a84bb29daa10ae05a5f74cbf92049e34e2a0b57067e7b441604a482e89ff
//META: purpose=Fires diagnostic E31210 (derivative-group-quad-must-be-multiple-2-for-xy-threads) -- derivative group quad thread count error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31210
//META: catalog_name=derivative-group-quad-must-be-multiple-2-for-xy-threads
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeGroupQuad][shader("compute")][numthreads(3,3,1)]
void main() {}


// CHECK: E31210
