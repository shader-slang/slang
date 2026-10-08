//META: generated=true
//META: model=claude-opus-5[1m]
//META: generated_at=2026-08-04T00:00:00+00:00
//META: source_commit=7e725f15572c6589ee6d738a8856fb3348f11617
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=78b1b44c27c4341b3095493831f694d027fa409126547f61baebc6616711d2b5
//META: purpose=Fires diagnostic E31230 (removed-modifier-usage) - use of removed modifier
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=31230
//META: catalog_name=removed-modifier-usage
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// 'volatile' is removed in Slang 2026; using it triggers E31230.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):-std 2026

static volatile int b;

//CHECK: E31230
