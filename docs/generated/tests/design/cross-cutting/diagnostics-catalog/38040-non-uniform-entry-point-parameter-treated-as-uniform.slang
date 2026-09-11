//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=f1b1378e6bd4e5443df935263c376ae52381df3e9540dc5268396d3321ba2d71
//META: purpose=Fires diagnostic E38040 (non-uniform-entry-point-parameter-treated-as-uniform) - entry point parameter treated as uniform
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=38040
//META: catalog_name=non-uniform-entry-point-parameter-treated-as-uniform
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// A compute entry point parameter with no system-value semantic
// and no `uniform` modifier is auto-converted to uniform with a
// warning.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl -stage compute -entry main

[numthreads(1,1,1)]
void main(int p)
{
}

//CHECK: E38040