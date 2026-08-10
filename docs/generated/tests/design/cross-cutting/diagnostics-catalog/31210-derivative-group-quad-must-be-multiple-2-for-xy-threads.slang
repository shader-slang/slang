//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=71bc04ee62e1c22187eb0ac11c5c43fd64a632f94ed3a945c0b9de7d2b202d54
//META: purpose=Fires diagnostic E31210 (derivative-group-quad-must-be-multiple-2-for-xy-threads) -- derivative group quad thread count error
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31210
//META: catalog_name=derivative-group-quad-must-be-multiple-2-for-xy-threads
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[DerivativeGroupQuad][shader("compute")][numthreads(3,3,1)]
void main() {}


//CHECK: E31210
