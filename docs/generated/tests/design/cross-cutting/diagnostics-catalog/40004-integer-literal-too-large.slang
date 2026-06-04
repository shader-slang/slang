//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:30:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=0000000000000000000000000000000000000000000000000000000000000000
//META: purpose=Fires diagnostic W40004 (integer-literal-too-large) - integer literal too large for signed
//META: intent=negative
//META: pipeline_stage=lex
//META: catalog_code=40004
//META: catalog_name=integer-literal-too-large
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// An integer literal larger than the largest signed int triggers W40004.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):

static const int g = 9999999999999;

// CHECK: W40004
