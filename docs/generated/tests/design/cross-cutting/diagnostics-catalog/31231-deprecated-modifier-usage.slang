//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=9fbcde4f9ac224c04e5f2a9b5897f6aa1ba29646065c3a6493db47be21a2d692
//META: purpose=Fires diagnostic E31231 (deprecated-modifier-usage) - use of deprecated modifier
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=31231
//META: catalog_name=deprecated-modifier-usage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 'volatile' is deprecated in Slang 2025; using it triggers E31231.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2025

static volatile int b;

//CHECK: E31231