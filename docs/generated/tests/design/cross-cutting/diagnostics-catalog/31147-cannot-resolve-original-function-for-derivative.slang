//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=82fec82e120e5edb2ace5cf7cbeb248d0147167e094b0c7155c341ae8fd6ebbc
//META: purpose=Fires diagnostic E31147 (cannot-resolve-original-function-for-derivative) -- cannot resolve original function for derivative
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31147
//META: catalog_name=cannot-resolve-original-function-for-derivative
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

[BackwardDerivativeOf(unknownFunc)]
void derivFunc(inout DifferentialPair<float> p, float dOut) {}


//CHECK: E31147
