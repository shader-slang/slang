//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=9c3b0473d92dc983eefa739ead53bf2055c86632613dc8c21720d0a5fe754a1b
//META: purpose=Fires diagnostic E52002 (pass-through-compiler-not-found) - pass-through compiler not found
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=52002
//META: catalog_name=pass-through-compiler-not-found
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Requesting -pass-through with a downstream tool (fxc) that is
// not installed produces the pass-through-not-found error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target dxil -entry main -profile cs_6_0 -pass-through fxc

[numthreads(1,1,1)]
void main() {}

//CHECK: E52002
