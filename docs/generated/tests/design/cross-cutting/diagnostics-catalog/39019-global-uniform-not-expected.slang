//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=81ded0f694080d79c6e982f56df4be905d86211847931b9135e1f761e74e2dd5
//META: purpose=Fires diagnostic E39019 (global-uniform-not-expected) - implicit global shader parameter
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39019
//META: catalog_name=global-uniform-not-expected
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A global ordinary variable without `uniform` or `static`
// becomes an implicit shader parameter, triggering E39019.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main -profile sm_6_0

float someGlobal;

[numthreads(1,1,1)]
void main() {}

//CHECK: E39019