//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W39001 (parameter-bindings-overlap) - explicit binding overlap
//META: intent=negative
//META: pipeline_stage=check
//META: catalog_code=39001
//META: catalog_name=parameter-bindings-overlap
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Two parameters with identical `register(t0)` bindings trigger
// W39001.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-target hlsl

Texture2D a : register(t0);
Texture2D b : register(t0);

// CHECK: W39001
