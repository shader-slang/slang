//META: generated=true
//META: model=claude-opus-4-7
//META: generated_at=2026-05-21T08:00:00Z
//META: source_commit=2964da04de136705348240f2bd9affa5f3818226
//META: doc_ref=source/slang/slang-diagnostics.lua
//META: doc_section_digest=90e08e718ef3c42629a46041937d0ea97ce192cb87a0b3e414306237480cd0b9
//META: purpose=Fires diagnostic E20102 (unexpected-body-after-semicolon) from a minimum-reproduction input.
//META: intent=negative
//META: pipeline_stage=parse
//META: catalog_code=20102
//META: catalog_name=unexpected-body-after-semicolon
//META: warning=Auto-generated. May drift from source. Do not edit by hand.

// Minimum reproduction for diagnostic E20102.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,non-exhaustive):
void foo(); { }
void main() {}

//CHECK: E20102
