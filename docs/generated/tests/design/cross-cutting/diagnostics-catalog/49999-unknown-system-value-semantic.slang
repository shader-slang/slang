//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=3a30f52de9752007a66d48558ed00874b9fcb580150c0ea9116508669444d00c
//META: purpose=Fires diagnostic E49999 (unknown-system-value-semantic) - unknown system-value semantic
//META: intent=negative
//META: pipeline_stage=emit
//META: catalog_code=49999
//META: catalog_name=unknown-system-value-semantic
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A made-up SV_ semantic name is not in the system-value table
// and is rejected at code-generation time.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -entry main -stage compute

[numthreads(1,1,1)]
void main(float x : SV_BOGUSNAME) {}

//CHECK: E49999
