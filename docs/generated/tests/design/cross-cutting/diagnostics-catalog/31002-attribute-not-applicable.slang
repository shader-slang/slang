//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0a7ec6e53434a7791f783e155939294ecc3201e0ccfd164e9b277146260c2817
//META: purpose=Fires diagnostic E31002 (attribute-not-applicable) - invalid attribute placement
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=31002
//META: catalog_name=attribute-not-applicable
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Each attribute declares the syntactic targets it may decorate. `[numthreads]`
// applies to entry points, so placing it on a global variable is rejected and
// the CHECK pins the not-applicable error.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute
[numthreads(1,1,1)] int gx;
[numthreads(1,1,1)]
void main() {}

//CHECK: E31002
