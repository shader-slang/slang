//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=90e08e718ef3c42629a46041937d0ea97ce192cb87a0b3e414306237480cd0b9
//META: purpose=Fires diagnostic E15207 (unknown-language-version) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=preprocess
//META: catalog_code=15207
//META: catalog_name=unknown-language-version
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E15207.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
#version 99999 hlsl
void main() {}

//CHECK: E15207
