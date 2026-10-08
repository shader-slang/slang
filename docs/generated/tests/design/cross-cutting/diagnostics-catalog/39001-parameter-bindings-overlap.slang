//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=d08291b92ae02458b9d02fd92cbc765b9424c505747c1e9befa457169c01afe4
//META: purpose=Fires diagnostic E39001 (parameter-bindings-overlap) - explicit binding overlap
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39001
//META: catalog_name=parameter-bindings-overlap
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two parameters with identical `register(t0)` bindings trigger
// E39001.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

Texture2D a : register(t0);
Texture2D b : register(t0);

//CHECK: E39001