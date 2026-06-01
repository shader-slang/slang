//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: purpose=Fires diagnostic W31231 (deprecated-modifier-usage) - use of deprecated modifier
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=31231
//META: catalog_name=deprecated-modifier-usage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 'volatile' is deprecated in Slang 2025; using it triggers W31231.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2025

static volatile int b;

// CHECK: W31231
